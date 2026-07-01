/**
 * QEMUController.cpp
 * 
 * Implementation of QEMU-based VM Controller
 * Replaces VirtualBox with direct QEMU control
 */

#include "vmcontroller/QEMUController.hpp"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHostAddress>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
QEMUController& QEMUController::getInstance() {
    static QEMUController instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
QEMUController::QEMUController(QObject* parent)
    : QObject(parent)
    , m_kvmEnabled(true)
    , m_defaultDisplayType(DisplayType::SDL)
    , m_nextVNCPort(5900)
    , m_qmpServer(nullptr)
    , m_initialized(false)
{
#ifdef _WIN32
    m_qemuPath = "C:\\Program Files\\qemu";
    m_qemuBinary = "qemu-system-x86_64.exe";
#elif __linux__
    m_qemuPath = "/usr/bin";
    m_qemuBinary = "qemu-system-x86_64";
#else
    m_qemuPath = "/usr/local/bin";
    m_qemuBinary = "qemu-system-x86_64";
#endif

    m_vmStoragePath = QDir::homePath() + "/VirtualPhonePro/VMs";
    QDir().mkpath(m_vmStoragePath);
}

QEMUController::~QEMUController() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool QEMUController::initialize() {
    if (m_initialized) {
        return true;
    }

    qInfo() << "[QEMUController] Initializing...";

    // Check if QEMU is installed
    if (!isQEMUInstalled()) {
        qWarning() << "[QEMUController] QEMU not found. Please install QEMU.";
        return false;
    }

    // Create QMP server for QEMU to connect back
    m_qmpServer = new QTcpServer(this);
    if (!m_qmpServer->listen(QHostAddress::LocalHost, 0)) {
        qWarning() << "[QEMUController] Failed to create QMP server";
    } else {
        qInfo() << "[QEMUController] QMP server listening on port" << m_qmpServer->serverPort();
    }

    m_initialized = true;
    qInfo() << "[QEMUController] Initialization complete";
    return true;
}

void QEMUController::shutdown() {
    qInfo() << "[QEMUController] Shutting down...";

    // Stop all VMs
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it) {
        stopVM(it.key());
    }

    // Close QMP server
    if (m_qmpServer) {
        m_qmpServer->close();
        delete m_qmpServer;
        m_qmpServer = nullptr;
    }

    m_initialized = false;
    qInfo() << "[QEMUController] Shutdown complete";
}

bool QEMUController::isInitialized() const {
    return m_initialized;
}

bool QEMUController::isQEMUInstalled() {
    QStringList possiblePaths = {
#ifdef _WIN32
        "qemu-system-x86_64.exe",
        "C:\\Program Files\\qemu\\qemu-system-x86_64.exe",
        "C:\\Program Files (x86)\\qemu\\qemu-system-x86_64.exe",
#elif __linux__
        "qemu-system-x86_64",
        "/usr/bin/qemu-system-x86_64",
        "/usr/local/bin/qemu-system-x86_64",
#else
        "qemu-system-x86_64",
        "/usr/bin/qemu-system-x86_64"
#endif
    };

    for (const QString& path : possiblePaths) {
        QProcess proc;
        proc.start(path, { "--version" });
        if (proc.waitForFinished(3000) && proc.exitCode() == 0) {
            return true;
        }
    }
    return false;
}

QString QEMUController::getQEMUVersion() {
    QProcess proc;
    proc.start(m_qemuBinary, { "--version" });
    if (proc.waitForFinished(3000) && proc.exitCode() == 0) {
        return proc.readAllStandardOutput().trimmed();
    }
    return QString();
}

// ============================================================================
// VM Instance Management
// ============================================================================
QString QEMUController::launchVM(const QString& profileId, const QEMUConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_initialized) {
        qWarning() << "[QEMUController] Not initialized";
        return QString();
    }

    QString instanceId = generateInstanceId();

    qInfo() << "[QEMUController] Launching VM:" << instanceId << "for profile:" << profileId;

    // Create instance info
    QEMUInstance instance;
    instance.instanceId = instanceId;
    instance.profileId = profileId;
    instance.vmName = QString("VPhone_%1").arg(profileId);
    instance.state = QEMUState::STARTING;
    instance.config = config;
    instance.startTime = QDateTime::currentDateTime();
    instance.vncPort = allocateVNCPort();

    // Generate spoofing parameters
    instance.config.serialNumber = generateSerialNumber();
    instance.config.imei = "";
    generateIMEI(&instance.config.imei);
    instance.config.androidId = "";
    generateAndroidId(&instance.config.androidId);
    if (instance.config.macAddress.isEmpty()) {
        instance.config.macAddress = generateMAC();
    }

    // Store instance
    m_instances[instanceId] = instance;

    // Build QEMU command
    QStringList args = buildQEMUCommand(instanceId, config).split(' ');

    // Start QEMU process
    QProcess* process = new QProcess(this);
    m_processes[instanceId] = process;

    connect(process, &QProcess::started, [this, instanceId]() {
        onProcessStarted(instanceId);
    });
    connect(process, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            [this, instanceId](QProcess::ProcessError error) {
        onProcessError(instanceId, error);
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, instanceId](int exitCode, QProcess::ExitStatus status) {
        onProcessFinished(instanceId, exitCode, status);
    });

    // Start process
    process->setProgram(m_qemuBinary);
    process->setArguments(args);
    process->start();

    emit vmStarted(instanceId);
    emit vmStateChanged(instanceId, QEMUState::STARTING);

    return instanceId;
}

bool QEMUController::stopVM(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    qInfo() << "[QEMUController] Stopping VM:" << instanceId;

    QEMUInstance& instance = m_instances[instanceId];
    instance.state = QEMUState::STOPPING;

    // Try graceful shutdown via QMP
    sendQMPCommand(instanceId, "{\"execute\":\"system_powerdown\"}");

    // Wait a bit
    QThread::msleep(2000);

    // Force stop if still running
    return stopVMImmediately(instanceId);
}

bool QEMUController::stopVMImmediately(const QString& instanceId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_instances.contains(instanceId)) {
        return false;
    }

    qInfo() << "[QEMUController] Force stopping VM:" << instanceId;

    if (m_processes.contains(instanceId)) {
        QProcess* proc = m_processes.take(instanceId);
        if (proc->state() != QProcess::NotRunning) {
            proc->kill();
            proc->waitForFinished(3000);
        }
        proc->deleteLater();
    }

    // Close QMP socket
    if (m_qmpSockets.contains(instanceId)) {
        m_qmpSockets[instanceId]->close();
        m_qmpSockets[instanceId]->deleteLater();
        m_qmpSockets.remove(instanceId);
    }

    QEMUInstance& instance = m_instances[instanceId];
    instance.state = QEMUState::STOPPED;

    emit vmStopped(instanceId);
    emit vmStateChanged(instanceId, QEMUState::STOPPED);

    return true;
}

bool QEMUController::pauseVM(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    bool success = sendQMPCommand(instanceId, "{\"execute\":\"stop\"}");
    if (success) {
        m_instances[instanceId].state = QEMUState::PAUSED;
        emit vmStateChanged(instanceId, QEMUState::PAUSED);
    }
    return success;
}

bool QEMUController::resumeVM(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    bool success = sendQMPCommand(instanceId, "{\"execute\":\"cont\"}");
    if (success) {
        m_instances[instanceId].state = QEMUState::RUNNING;
        emit vmStateChanged(instanceId, QEMUState::RUNNING);
    }
    return success;
}

// ============================================================================
// VM Queries
// ============================================================================
bool QEMUController::isVMRunning(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    return m_instances[instanceId].state == QEMUState::RUNNING;
}

QEMUState QEMUController::getVMState(const QString& instanceId) {
    if (m_instances.contains(instanceId)) {
        return m_instances[instanceId].state;
    }
    return QEMUState::STOPPED;
}

QEMUInstance QEMUController::getInstanceInfo(const QString& instanceId) {
    return m_instances.value(instanceId);
}

QList<QEMUInstance> QEMUController::getAllInstances() {
    return m_instances.values();
}

int QEMUController::getRunningInstanceCount() {
    int count = 0;
    for (const auto& instance : m_instances) {
        if (instance.state == QEMUState::RUNNING) {
            count++;
        }
    }
    return count;
}

// ============================================================================
// QMP Control
// ============================================================================
bool QEMUController::sendQMPCommand(const QString& instanceId, const QString& command, QJsonObject* response) {
    if (!m_qmpSockets.contains(instanceId)) {
        return false;
    }

    QTcpSocket* socket = m_qmpSockets[instanceId];
    if (!socket->isOpen()) {
        return false;
    }

    QByteArray data = command.toUtf8() + "\n";
    socket->write(data);
    socket->flush();

    if (socket->waitForReadyRead(3000)) {
        QByteArray reply = socket->readAll();
        if (response) {
            QJsonDocument doc = QJsonDocument::fromJson(reply);
            *response = doc.object();
        }
        return true;
    }

    return false;
}

bool QEMUController::stopQEMU(const QString& instanceId) {
    return sendQMPCommand(instanceId, "{\"execute\":\"quit\"}");
}

bool QEMUController::contQEMU(const QString& instanceId) {
    return sendQMPCommand(instanceId, "{\"execute\":\"cont\"}");
}

bool QEMUController::systemReset(const QString& instanceId) {
    return sendQMPCommand(instanceId, "{\"execute\":\"system_reset\"}");
}

bool QEMUController::systemPowerdown(const QString& instanceId) {
    return sendQMPCommand(instanceId, "{\"execute\":\"system_powerdown\"}");
}

bool QEMUController::screendump(const QString& instanceId, const QString& filename) {
    QJsonObject response;
    QString cmd = QString("{\"execute\":\"screendump\",\"arguments\":{\"filename\":\"%1\"}}").arg(filename);
    return sendQMPCommand(instanceId, cmd, &response);
}

// ============================================================================
// Display Control
// ============================================================================
DisplayType QEMUController::getDisplayType() const {
    return m_defaultDisplayType;
}

void QEMUController::setDisplayType(DisplayType type) {
    m_defaultDisplayType = type;
}

int QEMUController::getVNCPort(const QString& instanceId) {
    if (m_instances.contains(instanceId)) {
        return m_instances[instanceId].vncPort;
    }
    return -1;
}

bool QEMUController::connectVNC(const QString& instanceId, const QString& host, int port) {
    Q_UNUSED(instanceId);
    Q_UNUSED(host);
    Q_UNUSED(port);
    // VNC connection handled by VNC client widget
    return true;
}

// ============================================================================
// Input Control (Direct QEMU input, no ADB needed)
// ============================================================================
bool QEMUController::sendKey(const QString& instanceId, int keycode) {
    // QEMU input event via QMP
    QJsonObject args;
    args["type"] = "key";
    args["data"] = QJsonObject({
        {"key", QJsonObject({{"type", "number"}, {"data", keycode}})}
    });

    QJsonObject cmd;
    cmd["execute"] = "send-key";
    cmd["arguments"] = args;

    QJsonObject response;
    return sendQMPCommand(instanceId, QJsonDocument(cmd).toJson(), &response);
}

bool QEMUController::sendText(const QString& instanceId, const QString& text) {
    // For text, we need to type each character
    // This requires a guest agent or keyboard event simulation
    Q_UNUSED(instanceId);
    Q_UNUSED(text);
    return false;
}

bool QEMUController::sendMouseEvent(const QString& instanceId, int x, int y, bool pressed) {
    QString cmd = QString(
        "{\"execute\":\"input_send_event\",\"arguments\":{\"type\":\"%1\",\"data\":{\"button\":\"left\",\"pressed\":%2,\"x\":%3,\"y\":%4}}}"
    ).arg(pressed ? "button" : "button").arg(pressed ? "true" : "false").arg(x).arg(y);

    return sendQMPCommand(instanceId, cmd);
}

bool QEMUController::pressPowerButton(const QString& instanceId) {
    // KEY_POWER = 116
    return sendKey(instanceId, 116);
}

bool QEMUController::pressHomeButton(const QString& instanceId) {
    // KEY_HOME = 102
    return sendKey(instanceId, 102);
}

bool QEMUController::pressBackButton(const QString& instanceId) {
    // KEY_BACK = 158
    return sendKey(instanceId, 158);
}

bool QEMUController::pressVolumeUp(const QString& instanceId) {
    // KEY_VOLUMEUP = 115
    return sendKey(instanceId, 115);
}

bool QEMUController::pressVolumeDown(const QString& instanceId) {
    // KEY_VOLUMEDOWN = 114
    return sendKey(instanceId, 114);
}

// ============================================================================
// Device Spoofing (via Kernel Command Line, no ADB)
// ============================================================================
bool QEMUController::setDeviceInfo(const QString& instanceId, 
                                   const QString& manufacturer,
                                   const QString& model,
                                   const QString& brand) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    QEMUInstance& instance = m_instances[instanceId];
    instance.config.manufacturer = manufacturer;
    instance.config.deviceModel = model;
    instance.config.brand = brand;

    // Build kernel command line with new device info
    QString cmdline = QString("androidboot.hardware=%1 ro.product.model=%2 ro.product.brand=%3 ro.product.manufacturer=%4")
        .arg(manufacturer.toLower())
        .arg(model)
        .arg(brand)
        .arg(manufacturer);

    return setKernelCmdLine(instanceId, cmdline);
}

bool QEMUController::setSerialNumber(const QString& instanceId, const QString& serial) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    m_instances[instanceId].config.serialNumber = serial;

    QString cmdline = QString("androidboot.serialno=%1 ro.serialno=%1").arg(serial);
    return setKernelCmdLine(instanceId, cmdline);
}

bool QEMUController::setIMEI(const QString& instanceId, const QString& imei) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    m_instances[instanceId].config.imei = imei;

    // IMEI is typically set via modem firmware
    // For Android-x86, we set it in the kernel cmdline
    QString cmdline = QString("androidboot.imei=%1").arg(imei);
    return setKernelCmdLine(instanceId, cmdline);
}

bool QEMUController::setAndroidId(const QString& instanceId, const QString& androidId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    m_instances[instanceId].config.androidId = androidId;

    QString cmdline = QString("androidboot.android_id=%1").arg(androidId);
    return setKernelCmdLine(instanceId, cmdline);
}

bool QEMUController::setKernelCmdLine(const QString& instanceId, const QString& cmdline) {
    Q_UNUSED(instanceId);
    Q_UNUSED(cmdline);
    // Note: Kernel command line is set at boot time
    // To change it dynamically, you'd need to reboot with new parameters
    return false;
}

bool QEMUController::setGPSCoordinates(const QString& instanceId, double lat, double lon) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    // GPS spoofing via qemu-guest-agent or kernel parameters
    QString cmdline = QString("gps.latitude=%1 gps.longitude=%2").arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6);
    return setKernelCmdLine(instanceId, cmdline);
}

bool QEMUController::setTimezone(const QString& instanceId, const QString& timezone) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    QString cmdline = QString("androidboot.timezone=%1").arg(timezone);
    return setKernelCmdLine(instanceId, cmdline);
}

bool QEMUController::setMACAddress(const QString& instanceId, const QString& mac) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    m_instances[instanceId].config.macAddress = mac;
    return true;
}

bool QEMUController::setNetworkMode(const QString& instanceId, const QString& mode) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }

    m_instances[instanceId].config.networkMode = mode;
    return true;
}

// ============================================================================
// Android Boot Parameters
// ============================================================================
QString QEMUController::buildAndroidBootParams(const QEMUConfig& config, const FingerprintConfig& profile) {
    QStringList params;

    // Device identification
    params << QString("androidboot.hardware=%1").arg(config.deviceModel.toLower());
    params << QString("androidboot.serialno=%1").arg(config.serialNumber);
    params << QString("ro.serialno=%1").arg(config.serialNumber);
    params << QString("ro.product.model=%1").arg(profile.model);
    params << QString("ro.product.brand=%1").arg(profile.brand);
    params << QString("ro.product.manufacturer=%1").arg(profile.manufacturer);
    params << QString("ro.product.name=%1").arg(config.deviceModel);

    // Android ID
    params << QString("androidboot.android_id=%1").arg(config.androidId);

    // Device info
    params << QString("ro.build.product=%1").arg(profile.model);
    params << QString("ro.build.id=%1").arg(profile.buildId);
    params << QString("ro.build.version.release=%1").arg(profile.androidVersion);
    params << QString("ro.build.version.sdk=%1").arg(profile.androidVersion);
    params << QString("ro.build.version.security_patch=%1").arg(profile.securityPatch);

    // Bootloader
    params << QString("ro.bootloader=%1").arg(profile.bootloader);

    // Hardware
    params << QString("ro.hardware=%1").arg(profile.cpuABI);
    params << QString("ro.arch=%1").arg(profile.cpuABI);

    // Location (if spoofed)
    if (config.gpsLatitude != 0.0 || config.gpsLongitude != 0.0) {
        params << QString("gps.latitude=%1").arg(config.gpsLatitude, 0, 'f', 6);
        params << QString("gps.longitude=%1").arg(config.gpsLongitude, 0, 'f', 6);
    }

    // Timezone
    if (!profile.timezone.isEmpty()) {
        params << QString("androidboot.timezone=%1").arg(profile.timezone);
    }

    // Board and device
    params << QString("ro.board.platform=%1").arg(profile.cpuModel);
    params << QString("ro.board.cpu=%1").arg(profile.cpuModel);

    return params.join(" ");
}

// ============================================================================
// Configuration
// ============================================================================
void QEMUController::setQEMUPath(const QString& path) {
    m_qemuPath = path;
}

QString QEMUController::getQEMUPath() const {
    return m_qemuPath;
}

void QEMUController::setQEMUBinary(const QString& binary) {
    m_qemuBinary = binary;
}

QString QEMUController::getQEMUBinary() const {
    return m_qemuBinary;
}

void QEMUController::setKVMEnabled(bool enabled) {
    m_kvmEnabled = enabled;
}

bool QEMUController::isKVMEnabled() const {
    return m_kvmEnabled;
}

void QEMUController::setVMStoragePath(const QString& path) {
    m_vmStoragePath = path;
    QDir().mkpath(m_vmStoragePath);
}

QString QEMUController::getVMStoragePath() const {
    return m_vmStoragePath;
}

void QEMUController::setKernelPath(const QString& path) {
    m_kernelPath = path;
}

QString QEMUController::getKernelPath() const {
    return m_kernelPath;
}

// ============================================================================
// VM Template/Disk Management
// ============================================================================
bool QEMUController::createVMDisk(const QString& diskPath, int sizeGB) {
    qInfo() << "[QEMUController] Creating disk:" << diskPath << "Size:" << sizeGB << "GB";

    QStringList args = {
        "qemu-img", "create", "-f", "qcow2",
        diskPath,
        QString("%1G").arg(sizeGB)
    };

    QProcess proc;
    proc.start("qemu-img", args.mid(1));
    proc.waitForFinished(30000);

    return proc.exitCode() == 0;
}

bool QEMUController::deleteVMDisk(const QString& diskPath) {
    return QFile::remove(diskPath);
}

bool QEMUController::cloneVMDisk(const QString& source, const QString& dest) {
    qInfo() << "[QEMUController] Cloning disk:" << source << "->" << dest;

    QStringList args = {
        "qemu-img", "create", "-f", "qcow2", "-b", source, "-F", "qcow2", dest
    };

    QProcess proc;
    proc.start("qemu-img", args.mid(1));
    proc.waitForFinished(60000);

    return proc.exitCode() == 0;
}

bool QEMUController::isDiskExists(const QString& diskPath) {
    return QFile::exists(diskPath);
}

bool QEMUController::downloadAndroidImage(const QString& version) {
    Q_UNUSED(version);
    // In production, implement download from Android-x86 mirrors
    return false;
}

bool QEMUController::setupAndroidImage(const QString& imagePath) {
    Q_UNUSED(imagePath);
    return false;
}

QString QEMUController::getDefaultAndroidImagePath() const {
    return m_vmStoragePath + "/android.img";
}

// ============================================================================
// Private Helpers
// ============================================================================
QString QEMUController::generateInstanceId() {
    static int counter = 0;
    return QString("qemu_%1_%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(++counter);
}

QString QEMUController::generateSerialNumber() {
    // Generate Android-like serial number
    QString chars = "0123456789ABCDEF";
    QString serial;
    for (int i = 0; i < 16; ++i) {
        serial.append(chars[QRandomGenerator::global()->bounded(16)]);
    }
    return serial;
}

QString QEMUController::generateMAC() {
    // Generate MAC with plausible OUI
    QStringList oui = {"02:", "06:", "0a:", "12:", "1e:", "3a:"};
    QString mac = oui[QRandomGenerator::global()->bounded(oui.size())];
    
    for (int i = 0; i < 3; ++i) {
        mac += QString("%1:").arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0')).toUpper();
    }
    return mac.chopped(1);
}

bool QEMUController::generateIMEI(QString* outIMEI) {
    // Generate Luhn-valid IMEI
    QString imei = "35";  // Type Allocation Code prefix
    for (int i = 0; i < 13; ++i) {
        imei.append(QString::number(QRandomGenerator::global()->bounded(10)));
    }
    
    // Calculate Luhn checksum
    int sum = 0;
    for (int i = 0; i < 14; ++i) {
        int digit = imei[i].digitValue();
        if (i % 2 == 0) {
            digit *= 2;
            if (digit > 9) digit -= 9;
        }
        sum += digit;
    }
    int checkDigit = (10 - (sum % 10)) % 10;
    imei.append(QString::number(checkDigit));
    
    *outIMEI = imei;
    return true;
}

bool QEMUController::generateAndroidId(QString* outId) {
    // Generate 16 hex character Android ID
    QString chars = "0123456789abcdef";
    QString id;
    for (int i = 0; i < 16; ++i) {
        id.append(chars[QRandomGenerator::global()->bounded(16)]);
    }
    *outId = id;
    return true;
}

QString QEMUController::buildQEMUCommand(const QString& instanceId, const QEMUConfig& config) {
    QStringList args;

    // VM name (for display)
    args << "-name" << QString("VPhone_%1,process=%1").arg(instanceId);

    // Machine type (pc-i440fx for x86, virt for ARM)
    args << "-machine" << "pc-i440fx-7.0";

    // CPU configuration
    args << "-cpu" << "host";  // Use host CPU for best performance
    args << "-smp" << QString("%1,cores=%1,threads=1,sockets=1").arg(config.cpuCores);

    // Memory
    args << "-m" << QString("%1").arg(config.memoryMB);

    // Display
    if (config.displayType == DisplayType::SDL) {
        args << "-display" << "sdl";
        args << "-vga" << "virtio";
    } else if (config.displayType == DisplayType::GTK) {
        args << "-display" << "gtk";
        args << "-vga" << "virtio";
    } else if (config.displayType == DisplayType::VNC) {
        int vncPort = m_instances.value(instanceId).vncPort;
        args << "-vnc" << QString(":%1").arg(vncPort - 5900);
        args << "-vga" << "virtio";
    }

    // Storage
    if (!config.diskPath.isEmpty() && QFile::exists(config.diskPath)) {
        args << "-drive" << QString("file=%1,if=virtio,aio=native,cache=directsync").arg(config.diskPath);
    }

    // Network
    args << "-net" << QString("nic,model=virtio,macaddr=%1").arg(config.macAddress);
    args << "-net" << "user,hostfwd=tcp::5555-:5555";

    // KVM acceleration
    if (m_kvmEnabled) {
        args << "-enable-kvm";
        args << "-machine" << "pc-i440fx-7.0,accel=kvm";
    }

    // USB for input
    args << "-usb";
    args << "-device" << "usb-tablet";

    // Sound
    args << "-device" << "intel-hda";
    args << "-device" << "hda-duplex";

    // QMP socket for control
    int qmpPort = m_nextVNCPort + 100;
    args << "-qmp" << QString("tcp:127.0.0.1:%1,server,nowait").arg(qmpPort);

    // Monitor socket
    args << "-monitor" << QString("tcp:127.0.0.1:%1,server,nowait").arg(qmpPort + 1);

    // Serial console (for Android kernel messages)
    args << "-serial" << "stdio";

    // Kernel and initrd (for Android)
    if (!config.kernelPath.isEmpty()) {
        args << "-kernel" << config.kernelPath;
    }
    if (!config.initrdPath.isEmpty()) {
        args << "-initrd" << config.initrdPath;
    }
    if (!config.append.isEmpty()) {
        args << "-append" << config.append;
    }

    // Enable clipboard sharing
    args << "-device" << "virtio-balloon-pci";

    // SMP alignment
    args << "-no-fortune cookie";

    return args.join(" ");
}

QString QEMUController::buildQEMUCPUArgs(const QEMUConfig& config) {
    QStringList args;
    args << "-cpu" << "host";
    args << "-smp" << QString("%1").arg(config.cpuCores);
    return args.join(" ");
}

QString QEMUController::buildQEMUDisplayArgs(const QEMUConfig& config) {
    QStringList args;
    args << "-vga" << "virtio";
    
    if (config.displayType == DisplayType::SDL) {
        args << "-display" << "sdl";
    } else if (config.displayType == DisplayType::GTK) {
        args << "-display" << "gtk";
    }
    
    return args.join(" ");
}

QString QEMUController::buildQEMUNetworkArgs(const QEMUConfig& config) {
    QStringList args;
    args << "-net" << QString("nic,model=virtio,macaddr=%1").arg(config.macAddress);
    args << "-net" << "user";
    return args.join(" ");
}

QString QEMUController::buildQEMUDeviceArgs(const QEMUConfig& config) {
    Q_UNUSED(config);
    QStringList args;
    args << "-device" << "virtio-keyboard-pci";
    args << "-device" << "virtio-mouse-pci";
    args << "-device" << "virtio-tablet-pci";
    return args.join(" ");
}

QString QEMUController::buildQEMUAndroidArgs(const QEMUConfig& config, const FingerprintConfig& profile) {
    Q_UNUSED(config);
    Q_UNUSED(profile);
    // Android-specific boot parameters
    return QString();
}

int QEMUController::allocateVNCPort() {
    std::lock_guard<std::mutex> lock(m_portMutex);
    
    for (int port = m_nextVNCPort; port < 6000; ++port) {
        if (!m_allocatedPorts.contains(port)) {
            m_allocatedPorts.insert(port);
            m_nextVNCPort = (port + 1) % 100;
            if (m_nextVNCPort < 5900) m_nextVNCPort = 5900;
            return port;
        }
    }
    
    // Wrap around
    for (int port = 5900; port < m_nextVNCPort; ++port) {
        if (!m_allocatedPorts.contains(port)) {
            m_allocatedPorts.insert(port);
            m_nextVNCPort = port + 1;
            return port;
        }
    }
    
    return -1;
}

// ============================================================================
// Process Management
// ============================================================================
void QEMUController::onProcessStarted(const QString& instanceId) {
    qInfo() << "[QEMUController] QEMU process started:" << instanceId;

    if (m_instances.contains(instanceId)) {
        QEMUInstance& instance = m_instances[instanceId];
        instance.state = QEMUState::RUNNING;
        instance.pid = m_processes[instanceId]->processId();

        emit vmStateChanged(instanceId, QEMUState::RUNNING);

        // Try to connect to QMP
        connectQMP(instanceId);
    }
}

void QEMUController::onProcessError(const QString& instanceId, QProcess::ProcessError error) {
    qWarning() << "[QEMUController] Process error:" << instanceId << "Error:" << error;

    if (m_instances.contains(instanceId)) {
        m_instances[instanceId].state = QEMUState::ERROR;
        m_instances[instanceId].errorMessage = QString("Process error: %1").arg(error);
        emit vmError(instanceId, m_instances[instanceId].errorMessage);
        emit vmStateChanged(instanceId, QEMUState::ERROR);
    }
}

void QEMUController::onProcessFinished(const QString& instanceId, int exitCode, QProcess::ExitStatus status) {
    qInfo() << "[QEMUController] Process finished:" << instanceId 
            << "ExitCode:" << exitCode << "Status:" << status;

    // Clean up
    if (m_qmpSockets.contains(instanceId)) {
        m_qmpSockets[instanceId]->close();
        m_qmpSockets[instanceId]->deleteLater();
        m_qmpSockets.remove(instanceId);
    }

    if (m_instances.contains(instanceId)) {
        m_instances[instanceId].state = QEMUState::STOPPED;
        emit vmStopped(instanceId);
        emit vmStateChanged(instanceId, QEMUState::STOPPED);
    }
}

bool QEMUController::connectQMP(const QString& instanceId) {
    int port = m_nextVNCPort + 100;  // QMP port offset
    Q_UNUSED(port);
    
    // For now, QMP connection is optional
    // In production, you'd parse QEMU output to get the actual QMP port
    
    emit qmpConnected(instanceId);
    return true;
}

// ============================================================================
// Monitoring
// ============================================================================
void QEMUController::monitorVM(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return;
    }

    // Check if process is still running
    if (m_processes.contains(instanceId)) {
        QProcess* proc = m_processes[instanceId];
        if (proc->state() == QProcess::NotRunning) {
            m_instances[instanceId].state = QEMUState::STOPPED;
            emit vmStopped(instanceId);
        }
    }
}

void QEMUController::checkVMHealth(const QString& instanceId) {
    Q_UNUSED(instanceId);
    // Health check implementation
}

} // namespace VirtualPhonePro