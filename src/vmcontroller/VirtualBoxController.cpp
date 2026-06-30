/**
 * VirtualBoxController.cpp
 * 
 * Implementation of Headless VirtualBox VM Controller
 */

#include "vmcontroller/VirtualBoxController.hpp"
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QCoreApplication>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
VirtualBoxController& VirtualBoxController::getInstance() {
    static VirtualBoxController instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
VirtualBoxController::VirtualBoxController(QObject* parent)
    : QObject(parent)
    , m_nextPort(5555)
    , m_bootTimeout(60000)
    , m_initialized(false)
{
    // Default paths for Windows
#ifdef _WIN32
    m_vboxPath = "VBoxManage.exe";
    m_adbPath = "adb.exe";
#else
    m_vboxPath = "VBoxManage";
    m_adbPath = "adb";
#endif
    
    // Initialize timers
    m_stateCheckTimer = new QTimer(this);
    m_adbCheckTimer = new QTimer(this);
    
    connect(m_stateCheckTimer, &QTimer::timeout, this, &VirtualBoxController::checkVMStates);
    connect(m_adbCheckTimer, &QTimer::timeout, this, &VirtualBoxController::checkVMStates);
}

VirtualBoxController::~VirtualBoxController() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool VirtualBoxController::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Check if VirtualBox is installed
    if (!isVBoxInstalled()) {
        m_lastError = "VirtualBox is not installed or VBoxManage not found in PATH";
        qWarning() << "[VirtualBoxController]" << m_lastError;
        return false;
    }
    
    m_initialized = true;
    
    // Start monitoring timers
    m_stateCheckTimer->start(5000);  // Check every 5 seconds
    m_adbCheckTimer->start(2000);    // Check ADB every 2 seconds
    
    qInfo() << "[VirtualBoxController] Initialized successfully";
    return true;
}

void VirtualBoxController::shutdown() {
    qInfo() << "[VirtualBoxController] Shutting down...";
    
    // Stop timers
    m_stateCheckTimer->stop();
    m_adbCheckTimer->stop();
    
    // Stop all running VMs
    for (auto it = m_runningVMs.begin(); it != m_runningVMs.end(); ++it) {
        stopVM(it.value().vmName);
    }
    
    m_runningVMs.clear();
    m_initialized = false;
    
    qInfo() << "[VirtualBoxController] Shutdown complete";
}

bool VirtualBoxController::isInitialized() const {
    return m_initialized;
}

// ============================================================================
// VM Operations - Launch
// ============================================================================
bool VirtualBoxController::launchVM(const QString& vmName, const QString& profileId, int adbPort) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        qWarning() << "[VirtualBoxController] Not initialized";
        return false;
    }
    
    if (!vmExists(vmName)) {
        m_lastError = QString("VM '%1' does not exist").arg(vmName);
        qWarning() << "[VirtualBoxController]" << m_lastError;
        emit vmError(vmName, m_lastError);
        return false;
    }
    
    if (isVMRunning(vmName)) {
        qInfo() << "[VirtualBoxController] VM already running:" << vmName;
        return true;
    }
    
    // Update state
    VBoxVMInfo info;
    info.vmName = vmName;
    info.profileId = profileId;
    info.adbPort = adbPort;
    info.state = VMLaunchState::STARTING;
    info.startTime = QDateTime::currentDateTime();
    m_runningVMs[vmName] = info;
    
    // Configure ADB port forwarding in VirtualBox
    QStringList forwardArgs = {
        "guestproperty", "set", vmName,
        "/VirtualBox/GuestAdd/ExposedPorts", QString::number(adbPort)
    };
    executeVBoxCommand(forwardArgs);
    
    // Launch in headless mode
    return launchVMHeadless(vmName);
}

bool VirtualBoxController::launchVMHeadless(const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QStringList args = {
        "startvm", vmName,
        "--type", "headless"
    };
    
    qInfo() << "[VirtualBoxController] Launching VM in headless mode:" << vmName;
    
    QProcess* process = new QProcess(this);
    m_processes[vmName] = process;
    
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &VirtualBoxController::onVMProcessFinished);
    connect(process, &QProcess::errorOccurred,
            this, &VirtualBoxController::onVMProcessError);
    
    process->start(m_vboxPath, args);
    
    // Update state
    if (m_runningVMs.contains(vmName)) {
        m_runningVMs[vmName].state = VMLaunchState::WAITING_FOR_BOOT;
        emit vmStateChanged(vmName, VMLaunchState::WAITING_FOR_BOOT);
    }
    
    return true;
}

// ============================================================================
// VM Operations - Stop
// ============================================================================
bool VirtualBoxController::stopVM(const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isVMRunning(vmName)) {
        qInfo() << "[VirtualBoxController] VM not running:" << vmName;
        return true;
    }
    
    qInfo() << "[VirtualBoxController] Stopping VM:" << vmName;
    
    // Graceful shutdown via ACPI
    QStringList args = {
        "controlvm", vmName, "acpipowerbutton"
    };
    
    executeVBoxCommand(args);
    
    // Wait a bit for graceful shutdown
    QThread::msleep(2000);
    
    // If still running, force kill
    if (isVMRunning(vmName)) {
        return forceStopVM(vmName);
    }
    
    return true;
}

bool VirtualBoxController::forceStopVM(const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    qInfo() << "[VirtualBoxController] Force stopping VM:" << vmName;
    
    QStringList args = {
        "controlvm", vmName, "poweroff"
    };
    
    bool success = executeVBoxCommandSync(args);
    
    // Clean up process
    if (m_processes.contains(vmName)) {
        QProcess* proc = m_processes.take(vmName);
        if (proc->state() != QProcess::NotRunning) {
            proc->kill();
            proc->waitForFinished(5000);
        }
        delete proc;
    }
    
    // Update state
    if (m_runningVMs.contains(vmName)) {
        m_runningVMs[vmName].state = VMLaunchState::STOPPED;
        emit vmStopped(vmName);
        emit vmStateChanged(vmName, VMLaunchState::STOPPED);
    }
    
    return success;
}

bool VirtualBoxController::pauseVM(const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isVMRunning(vmName)) {
        return false;
    }
    
    QStringList args = {
        "controlvm", vmName, "pause"
    };
    
    bool success = executeVBoxCommandSync(args);
    
    if (success && m_runningVMs.contains(vmName)) {
        m_runningVMs[vmName].state = VMLaunchState::STOPPED; // Using STOPPED as paused
    }
    
    return success;
}

bool VirtualBoxController::resumeVM(const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    QStringList args = {
        "controlvm", vmName, "resume"
    };
    
    bool success = executeVBoxCommandSync(args);
    
    if (success && m_runningVMs.contains(vmName)) {
        m_runningVMs[vmName].state = VMLaunchState::RUNNING;
        emit vmStateChanged(vmName, VMLaunchState::RUNNING);
    }
    
    return success;
}

// ============================================================================
// VM Queries
// ============================================================================
bool VirtualBoxController::isVMRunning(const QString& vmName) {
    QString state = getVMStateString(vmName);
    return (state == "running" || state == "paused");
}

VMLaunchState VirtualBoxController::getVMState(const QString& vmName) {
    if (m_runningVMs.contains(vmName)) {
        return m_runningVMs[vmName].state;
    }
    return VMLaunchState::STOPPED;
}

QString VirtualBoxController::getVMUUID(const QString& vmName) {
    QStringList args = { "list", "vms" };
    QString output = executeVBoxCommand(args);
    
    // Parse output to find UUID
    // Format: "VMName" {uuid}
    QString pattern = QString("\"%1\"").arg(vmName);
    for (const QString& line : output.split('\n')) {
        if (line.contains(pattern)) {
            int start = line.indexOf('{');
            int end = line.indexOf('}');
            if (start != -1 && end != -1) {
                return line.mid(start + 1, end - start - 1);
            }
        }
    }
    return QString();
}

QString VirtualBoxController::getVMStateString(const QString& vmName) {
    QString uuid = getVMUUID(vmName);
    if (uuid.isEmpty()) {
        return "unknown";
    }
    
    QStringList args = { "showvminfo", uuid, "--machinereadable" };
    QString output = executeVBoxCommand(args);
    
    for (const QString& line : output.split('\n')) {
        if (line.startsWith("VMState=")) {
            QString state = line.section('=', 1).remove('"').trimmed();
            return state;
        }
    }
    return "unknown";
}

int VirtualBoxController::getRunningVMCount() {
    std::lock_guard<std::mutex> lock(m_mutex);
    int count = 0;
    for (const auto& info : m_runningVMs) {
        if (info.state == VMLaunchState::RUNNING || 
            info.state == VMLaunchState::WAITING_FOR_BOOT) {
            count++;
        }
    }
    return count;
}

VBoxVMInfo VirtualBoxController::getVMInfo(const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_runningVMs.value(vmName);
}

QList<VBoxVMInfo> VirtualBoxController::getAllRunningVMs() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_runningVMs.values();
}

// ============================================================================
// Port Management
// ============================================================================
int VirtualBoxController::allocateADBPort() {
    std::lock_guard<std::mutex> lock(m_portMutex);
    
    // Find next available port starting from 5555
    for (int port = m_nextPort; port < 65535; ++port) {
        if (!m_allocatedPorts.contains(port) && isADBPortAvailable(port)) {
            m_allocatedPorts.insert(port);
            m_nextPort = (port + 1) % 65535;
            if (m_nextPort < 5555) m_nextPort = 5555;
            qDebug() << "[VirtualBoxController] Allocated ADB port:" << port;
            return port;
        }
    }
    
    qWarning() << "[VirtualBoxController] No available ADB ports!";
    return -1;
}

void VirtualBoxController::releaseADBPort(int port) {
    std::lock_guard<std::mutex> lock(m_portMutex);
    m_allocatedPorts.remove(port);
    qDebug() << "[VirtualBoxController] Released ADB port:" << port;
}

bool VirtualBoxController::isADBPortAvailable(int port) {
    // Check if any process is listening on this port
    QStringList args;
#ifdef _WIN32
    args = { "netstat", "-an" };
#else
    args = { "netstat", "-an" };
#endif
    
    QProcess proc;
    proc.start("cmd.exe", args);
    proc.waitForFinished(5000);
    QString output = proc.readAllStandardOutput();
    
    return !output.contains(QString(":%1").arg(port));
}

// ============================================================================
// ADB Connection
// ============================================================================
bool VirtualBoxController::waitForADB(int port, int timeoutMs) {
    qInfo() << "[VirtualBoxController] Waiting for ADB on port" << port;
    
    QProcess adbProc;
    int elapsed = 0;
    int interval = 500;
    
    while (elapsed < timeoutMs) {
        // Try to connect
        QStringList args = { "connect", QString("127.0.0.1:%1").arg(port) };
        adbProc.start(m_adbPath, args);
        adbProc.waitForFinished(2000);
        QString output = adbProc.readAllStandardOutput();
        
        if (output.contains("connected") || output.contains("already connected")) {
            qInfo() << "[VirtualBoxController] ADB connected on port" << port;
            return true;
        }
        
        QThread::msleep(interval);
        elapsed += interval;
    }
    
    qWarning() << "[VirtualBoxController] ADB connection timeout on port" << port;
    return false;
}

bool VirtualBoxController::connectADB(const QString& vmName, int port) {
    QString connectStr = QString("127.0.0.1:%1").arg(port);
    
    QProcess adbProc;
    QStringList args = { "connect", connectStr };
    adbProc.start(m_adbPath, args);
    adbProc.waitForFinished(5000);
    
    QString output = adbProc.readAllStandardOutput();
    bool success = output.contains("connected") || output.contains("already connected");
    
    if (success) {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (m_runningVMs.contains(vmName)) {
            m_runningVMs[vmName].state = VMLaunchState::RUNNING;
            emit adbConnected(vmName, port);
            emit vmStateChanged(vmName, VMLaunchState::RUNNING);
        }
    }
    
    return success;
}

bool VirtualBoxController::isADBConnected(const QString& vmName) {
    if (!m_runningVMs.contains(vmName)) {
        return false;
    }
    
    int port = m_runningVMs[vmName].adbPort;
    QString connectStr = QString("127.0.0.1:%1").arg(port);
    
    QProcess adbProc;
    QStringList args = { "-s", connectStr, "get-state" };
    adbProc.start(m_adbPath, args);
    adbProc.waitForFinished(3000);
    
    QString output = adbProc.readAllStandardOutput().trimmed();
    return output == "device";
}

// ============================================================================
// Configuration
// ============================================================================
void VirtualBoxController::setVBoxPath(const QString& path) {
    m_vboxPath = path;
}

QString VirtualBoxController::getVBoxPath() const {
    return m_vboxPath;
}

void VirtualBoxController::setADBPath(const QString& path) {
    m_adbPath = path;
}

QString VirtualBoxController::getADBPath() const {
    return m_adbPath;
}

void VirtualBoxController::setBootTimeout(int milliseconds) {
    m_bootTimeout = milliseconds;
}

int VirtualBoxController::getBootTimeout() const {
    return m_bootTimeout;
}

// ============================================================================
// Private Helpers
// ============================================================================
QString VirtualBoxController::executeVBoxCommand(const QStringList& args) {
    QProcess proc;
    proc.start(m_vboxPath, args);
    proc.waitForFinished(10000);
    return proc.readAllStandardOutput();
}

bool VirtualBoxController::executeVBoxCommandSync(const QStringList& args, QString* output) {
    QProcess proc;
    proc.start(m_vboxPath, args);
    bool success = proc.waitForFinished(10000);
    
    if (output) {
        *output = proc.readAllStandardOutput();
    }
    
    return success && proc.exitCode() == 0;
}

bool VirtualBoxController::isVBoxInstalled() {
    QProcess proc;
    proc.start(m_vboxPath, { "--version" });
    bool success = proc.waitForFinished(5000);
    return success && proc.exitCode() == 0;
}

bool VirtualBoxController::vmExists(const QString& vmName) {
    QStringList args = { "list", "vms" };
    QString output = executeVBoxCommand(args);
    
    return output.contains(QString("\"%1\"").arg(vmName));
}

void VirtualBoxController::onVMProcessStarted() {
    qInfo() << "[VirtualBoxController] VM process started";
}

void VirtualBoxController::onVMProcessError(QProcess::ProcessError error) {
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;
    
    qWarning() << "[VirtualBoxController] Process error:" << error;
}

void VirtualBoxController::onVMProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    QProcess* proc = qobject_cast<QProcess*>(sender());
    if (!proc) return;
    
    // Find VM name for this process
    QString vmName;
    for (auto it = m_processes.begin(); it != m_processes.end(); ++it) {
        if (it.value() == proc) {
            vmName = it.key();
            break;
        }
    }
    
    if (!vmName.isEmpty()) {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (m_runningVMs.contains(vmName)) {
            if (exitStatus == QProcess::CrashExit) {
                m_runningVMs[vmName].state = VMLaunchState::ERROR;
                emit vmError(vmName, "Process crashed");
            } else {
                m_runningVMs[vmName].state = VMLaunchState::STOPPED;
                emit vmStopped(vmName);
            }
        }
        
        m_processes.remove(vmName);
    }
    
    proc->deleteLater();
}

void VirtualBoxController::checkVMStates() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto it = m_runningVMs.begin(); it != m_runningVMs.end(); ++it) {
        const QString& vmName = it.key();
        VBoxVMInfo& info = it.value();
        
        // Check if VM is still running in VirtualBox
        QString vboxState = getVMStateString(vmName);
        bool vboxRunning = (vboxState == "running");
        
        if (!vboxRunning && (info.state == VMLaunchState::RUNNING || 
                            info.state == VMLaunchState::WAITING_FOR_BOOT)) {
            // VM stopped unexpectedly
            info.state = VMLaunchState::STOPPED;
            emit vmStopped(vmName);
            qWarning() << "[VirtualBoxController] VM stopped unexpectedly:" << vmName;
        }
    }
}

void VirtualBoxController::monitorBootProgress(const QString& vmName) {
    // Wait for ADB to be available
    if (!m_runningVMs.contains(vmName)) {
        return;
    }
    
    int port = m_runningVMs[vmName].adbPort;
    int timeout = m_bootTimeout;
    int elapsed = 0;
    int interval = 1000;
    
    while (elapsed < timeout) {
        if (connectADB(vmName, port)) {
            // Try to check if Android is booted
            QProcess adb;
            adb.start(m_adbPath, { "-s", QString("127.0.0.1:%1").arg(port), "shell", "getprop", "sys.boot_completed" });
            adb.waitForFinished(3000);
            QString output = adb.readAllStandardOutput().trimmed();
            
            if (output == "1") {
                qInfo() << "[VirtualBoxController] VM boot completed:" << vmName;
                emit bootCompleted(vmName);
                return;
            }
        }
        
        QThread::msleep(interval);
        elapsed += interval;
    }
    
    qWarning() << "[VirtualBoxController] Boot timeout for VM:" << vmName;
    emit vmError(vmName, "Boot timeout");
}

} // namespace VirtualPhonePro