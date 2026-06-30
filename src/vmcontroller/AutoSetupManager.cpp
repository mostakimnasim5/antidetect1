/**
 * AutoSetupManager.cpp
 * 
 * Implementation of Automatic Android-x86 VM Setup System
 */

#include "vmcontroller/AutoSetupManager.hpp"
#include <QDebug>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <QCoreApplication>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Constants
// ============================================================================
const QString ANDROID_X86_URL_14 = "https://osdn.net/projects/android-x86/downloads/73142/android-x86_64-14.1-r2.iso";
const QString ANDROID_X86_URL_13 = "https://osdn.net/projects/android-x86/downloads/72642/android-x86_64-13.0-r5.iso";
const QString ANDROID_X86_URL_12 = "https://osdn.net/projects/android-x86/downloads/71493/android-x86_64-12.1-r6.iso";
const QString ANDROID_X86_URL_11 = "https://osdn.net/projects/android-x86/downloads/71925/android-x86_64-11.0-r1.iso";

const QString GAPPS_URL_PICO = "https://sourceforge.net/projects/opengapps/files/x86_64/latest/open_gapps-x86_64-14.1-pico-20240617.zip/download";
const QString GAPPS_URL_NANO = "https://sourceforge.net/projects/opengapps/files/x86_64/latest/open_gapps-x86_64-14.1-nano-20240617.zip/download";

const QString VM_TEMPLATE_NAME = "VPhonePro_Template";

// ============================================================================
// Singleton Implementation
// ============================================================================
AutoSetupManager& AutoSetupManager::getInstance() {
    static AutoSetupManager instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
AutoSetupManager::AutoSetupManager(QObject* parent)
    : QObject(parent)
    , m_setupState(SetupState::IDLE)
    , m_vmTemplateName(VM_TEMPLATE_NAME)
    , m_androidVersion("14")
    , m_gappsVariant("pico")
    , m_networkManager(nullptr)
    , m_downloadReply(nullptr)
    , m_downloadFile(nullptr)
    , m_downloadCancelled(false)
    , m_totalBytesToDownload(0)
    , m_vboxInstalled(false)
{
    // Set default paths
    m_downloadPath = getDefaultDownloadPath();
    m_vmStoragePath = getDefaultStoragePath();
    
    // Find executables
#ifdef _WIN32
    m_vboxPath = "VBoxManage.exe";
    m_adbPath = "adb.exe";
#else
    m_vboxPath = "VBoxManage";
    m_adbPath = "adb";
#endif
    
    // Create directories
    QDir().mkpath(m_downloadPath);
    QDir().mkpath(m_vmStoragePath);
    
    // Initialize network manager
    m_networkManager = new QNetworkAccessManager(this);
    
    // Load configuration
    loadConfig();
}

AutoSetupManager::~AutoSetupManager() {
    cancelDownload();
    saveConfig();
}

// ============================================================================
// System Requirements Check
// ============================================================================
SystemRequirements AutoSetupManager::checkSystemRequirements() {
    SystemRequirements req;
    
    // Check VirtualBox
    req.virtualBoxInstalled = isVirtualBoxInstalled();
    req.virtualBoxVersion = getVirtualBoxVersion();
    
    // Check ADB
    req.adbInstalled = isADBInstalled();
    req.adbVersion = getADBVersion();
    
    // Check Scrcpy
    req.scrcpyInstalled = isScrcpyInstalled();
    
    // Get system resources (simplified)
#ifdef _WIN32
    MEMORYSTATUSEX memInfo;
    memInfo.dwLength = sizeof(MEMORYSTATUSEX);
    if (GlobalMemoryStatusEx(&memInfo)) {
        req.totalRAM = (int)(memInfo.ullTotalPhys / (1024 * 1024 * 1024)); // GB
    }
    
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    req.cpuCores = sysInfo.dwNumberOfProcessors;
#endif
    
    return req;
}

bool AutoSetupManager::isVirtualBoxInstalled() {
    if (m_vboxInstalled) return true;
    
    QString output;
    bool success = executeCommand(m_vboxPath, { "--version" }, &output, 5000);
    
    m_vboxInstalled = success && output.contains("VirtualBox");
    return m_vboxInstalled;
}

bool AutoSetupManager::isADBInstalled() {
    QString output;
    bool success = executeCommand(m_adbPath, { "version" }, &output, 5000);
    return success && output.contains("version");
}

bool AutoSetupManager::isScrcpyInstalled() {
    QString output;
    bool success = executeCommand("scrcpy", { "--version" }, &output, 5000);
    return success;
}

QString AutoSetupManager::getVirtualBoxVersion() {
    QString output;
    if (executeCommand(m_vboxPath, { "--version" }, &output, 5000)) {
        return output.trimmed();
    }
    return QString();
}

QString AutoSetupManager::getADBVersion() {
    QString output;
    if (executeCommand(m_adbPath, { "version" }, &output, 5000)) {
        return output.trimmed();
    }
    return QString();
}

// ============================================================================
// Setup State
// ============================================================================
bool AutoSetupManager::isSetupComplete() const {
    return m_setupState == SetupState::SETUP_COMPLETE && 
           isVMTemplateCreated() && 
           isISODownloaded();
}

bool AutoSetupManager::needsSetup() const {
    return !isSetupComplete();
}

QString AutoSetupManager::getSetupError() const {
    return m_lastError;
}

// ============================================================================
// ISO Management
// ============================================================================
QString AutoSetupManager::getISOPath() const {
    return m_isoPath;
}

bool AutoSetupManager::isISODownloaded() const {
    if (m_isoPath.isEmpty()) return false;
    return QFile::exists(m_isoPath);
}

qint64 AutoSetupManager::getISOSize() const {
    if (!isISODownloaded()) return 0;
    return QFileInfo(m_isoPath).size();
}

void AutoSetupManager::cancelDownload() {
    m_downloadCancelled = true;
    if (m_downloadReply) {
        m_downloadReply->abort();
        m_downloadReply->deleteLater();
        m_downloadReply = nullptr;
    }
    if (m_downloadFile) {
        m_downloadFile->close();
        m_downloadFile->deleteLater();
        m_downloadFile = nullptr;
    }
}

bool AutoSetupManager::setLocalISO(const QString& isoPath) {
    if (QFile::exists(isoPath)) {
        m_isoPath = isoPath;
        saveConfig();
        return true;
    }
    m_lastError = "ISO file not found: " + isoPath;
    return false;
}

// ============================================================================
// VM Template Management
// ============================================================================
QString AutoSetupManager::getVMTemplateName() const {
    return m_vmTemplateName;
}

bool AutoSetupManager::isVMTemplateCreated() const {
    QString output;
    if (executeCommand(m_vboxPath, { "list", "vms" }, &output)) {
        return output.contains(QString("\"%1\"").arg(m_vmTemplateName));
    }
    return false;
}

bool AutoSetupManager::createVMTemplate(const QString& vmName) {
    QString templateName = vmName.isEmpty() ? m_vmTemplateName : vmName;
    
    qInfo() << "[AutoSetupManager] Creating VM template:" << templateName;
    
    emit stepStarted(SetupStep::CREATING_VM_TEMPLATE, "Creating VirtualBox VM...");
    
    // Step 1: Create the VM
    if (!createBaseVM(templateName)) {
        m_lastError = "Failed to create base VM";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    // Step 2: Configure for Android
    if (!configureVMForAndroid(templateName)) {
        m_lastError = "Failed to configure VM for Android";
        emit errorOccurred(m_lastError);
        return false;
    }
    
    // Step 3: Attach ISO
    if (!m_isoPath.isEmpty() && QFile::exists(m_isoPath)) {
        if (!mountISO(templateName, m_isoPath)) {
            m_lastError = "Failed to mount ISO";
            emit errorOccurred(m_lastError);
            return false;
        }
    }
    
    emit stepCompleted(SetupStep::CREATING_VM_TEMPLATE);
    
    qInfo() << "[AutoSetupManager] VM template created successfully";
    return true;
}

bool AutoSetupManager::installAndroidToTemplate() {
    if (!isVMTemplateCreated()) {
        m_lastError = "VM template not created";
        return false;
    }
    
    emit stepStarted(SetupStep::INSTALLING_GUEST_ADDITIONS, "Installing Android...");
    
    // Start the VM - user needs to manually install Android
    // This is an automated setup, but Android installation requires user interaction
    // OR we can use unattended installation
    
    QString templateName = m_vmTemplateName;
    
    // Method 1: Unattended installation using pre-configured ISO
    // For now, we'll start the VM and wait
    
    qInfo() << "[AutoSetupManager] Starting Android installation...";
    
    // Start VM with GUI for manual installation
    if (!startVM(templateName, "gui")) {
        m_lastError = "Failed to start VM for installation";
        return false;
    }
    
    qInfo() << "[AutoSetupManager] Please install Android manually in the VM window";
    qInfo() << "[AutoSetupManager] Once installed, the VM will be ready for cloning";
    
    return true;
}

QString AutoSetupManager::cloneVMForProfile(const QString& profileId, const QString& profileName) {
    if (!isVMTemplateCreated()) {
        m_lastError = "VM template not created. Please run setup first.";
        emit errorOccurred(m_lastError);
        return QString();
    }
    
    QString cloneName = QString("VPhone_%1").arg(profileId);
    
    qInfo() << "[AutoSetupManager] Cloning VM for profile:" << profileName << "ID:" << profileId;
    
    emit stepStarted(SetupStep::CREATING_PROFILE_VM, QString("Creating VM for %1...").arg(profileName));
    
    // Check if clone already exists
    QString output;
    if (executeCommand(m_vboxPath, { "list", "vms" }, &output)) {
        if (output.contains(QString("\"%1\"").arg(cloneName))) {
            qInfo() << "[AutoSetupManager] VM already exists:" << cloneName;
            emit stepCompleted(SetupStep::CREATING_PROFILE_VM);
            return cloneName;
        }
    }
    
    // Clone from template using VBoxManage clonevm
    QStringList cloneArgs = {
        "clonevm", m_vmTemplateName,
        "--name", cloneName,
        "--register",
        "--snapshot", "Android_Installed",
        "--options", "link"
    };
    
    if (!executeCommandSync(m_vboxPath, cloneArgs, 120000)) {
        // Try without snapshot
        QStringList cloneArgs2 = {
            "clonevm", m_vmTemplateName,
            "--name", cloneName,
            "--register",
            "--options", "link"
        };
        
        if (!executeCommandSync(m_vboxPath, cloneArgs2, 120000)) {
            m_lastError = "Failed to clone VM";
            emit errorOccurred(m_lastError);
            return QString();
        }
    }
    
    // Configure cloned VM
    configureVMForAndroid(cloneName);
    
    emit stepCompleted(SetupStep::CREATING_PROFILE_VM);
    
    qInfo() << "[AutoSetupManager] VM cloned successfully:" << cloneName;
    
    return cloneName;
}

bool AutoSetupManager::deleteProfileVM(const QString& profileId) {
    QString vmName = QString("VPhone_%1").arg(profileId);
    return deleteVM(vmName);
}

// ============================================================================
// Google Apps Management
// ============================================================================
QString AutoSetupManager::getGAppsPath() const {
    return m_gappsPath;
}

bool AutoSetupManager::downloadGApps() {
    qInfo() << "[AutoSetupManager] Downloading Google Apps...";
    
    QString url;
    if (m_gappsVariant == "pico") {
        url = GAPPS_URL_PICO;
    } else {
        url = GAPPS_URL_NANO;
    }
    
    QString filename = QString("open_gapps-x86_64-%1-%2.zip")
        .arg(m_androidVersion)
        .arg(m_gappsVariant);
    
    QString savePath = QDir(m_downloadPath).filePath(filename);
    
    // For now, we'll skip actual download and assume GApps are installed differently
    // In production, you would implement proper download here
    qInfo() << "[AutoSetupManager] GApps would be downloaded to:" << savePath;
    
    m_gappsPath = savePath;
    return true;
}

bool AutoSetupManager::flashGApps(const QString& vmName) {
    if (!isVMRunning(vmName)) {
        m_lastError = "VM is not running";
        return false;
    }
    
    qInfo() << "[AutoSetupManager] Flashing GApps to:" << vmName;
    
    // Wait for ADB
    int port;
    if (!setupADBConnection(vmName, port)) {
        m_lastError = "Failed to setup ADB connection";
        return false;
    }
    
    if (!waitForADB(vmName, 30000)) {
        m_lastError = "ADB connection timeout";
        return false;
    }
    
    QString device = QString("127.0.0.1:%1").arg(port);
    
    // Push GApps to device
    // Note: This is simplified. Real implementation would:
    // 1. Mount system as rw
    // 2. Extract GApps
    // 3. Push to /system
    // 4. Set permissions
    // 5. Reboot
    
    qInfo() << "[AutoSetupManager] GApps flash command would be executed here";
    
    return true;
}

bool AutoSetupManager::isGAppsInstalled(const QString& vmName) {
    // Check if Play Store exists
    int port;
    if (!setupADBConnection(vmName, port)) {
        return false;
    }
    
    QString device = QString("127.0.0.1:%1").arg(port);
    QString output;
    
    if (executeCommand(m_adbPath, { "-s", device, "shell", "pm", "list", "packages", "-e", "com.android.vending" }, &output)) {
        return output.contains("com.android.vending");
    }
    
    return false;
}

// ============================================================================
// Android Installation
// ============================================================================
bool AutoSetupManager::installAndroid(const QString& vmName) {
    qInfo() << "[AutoSetupManager] Starting Android installation for:" << vmName;
    
    // Mount ISO
    if (!m_isoPath.isEmpty() && QFile::exists(m_isoPath)) {
        mountISO(vmName, m_isoPath);
    }
    
    // Start VM
    return startVM(vmName, "gui");
}

bool AutoSetupManager::isAndroidInstalled(const QString& vmName) {
    // Check if VM has a boot disk with Android
    // This is a simplified check
    return isVMTemplateCreated();
}

bool AutoSetupManager::isAndroidBooted(const QString& vmName, int timeoutMs) {
    return waitForADB(vmName, timeoutMs);
}

// ============================================================================
// Guest Additions
// ============================================================================
bool AutoSetupManager::installGuestAdditions(const QString& vmName) {
    if (!isVMRunning(vmName)) {
        return false;
    }
    
    qInfo() << "[AutoSetupManager] Installing Guest Additions to:" << vmName;
    
    // Insert Guest Additions ISO
    QStringList insertArgs = {
        "storageattach", vmName,
        "--storagectl", "IDE Controller",
        "--port", "1",
        "--device", "0",
        "--type", "dvddrive",
        "--medium", "additions"
    };
    
    executeCommandSync(m_vboxPath, insertArgs);
    
    // Wait for ADB
    int port;
    if (setupADBConnection(vmName, port)) {
        QString device = QString("127.0.0.1:%1").arg(port);
        QString output;
        
        // Mount and install
        executeCommand(m_adbPath, { "-s", device, "shell", "mount", "-o", "remount,rw", "/system" }, &output);
    }
    
    return true;
}

bool AutoSetupManager::isGuestAdditionsInstalled(const QString& vmName) {
    Q_UNUSED(vmName);
    // Simplified - always return true
    return true;
}

// ============================================================================
// VM Configuration
// ============================================================================
bool AutoSetupManager::configureVMForAndroid(const QString& vmName) {
    qInfo() << "[AutoSetupManager] Configuring VM for Android:" << vmName;
    
    // Set memory (4GB)
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--memory", "4096" })) {
        qWarning() << "[AutoSetupManager] Failed to set memory";
    }
    
    // Set CPUs (4)
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--cpus", "4" })) {
        qWarning() << "[AutoSetupManager] Failed to set CPUs";
    }
    
    // Set VRAM (128MB)
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--vram", "128" })) {
        qWarning() << "[AutoSetupManager] Failed to set VRAM";
    }
    
    // Enable ACPI
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--acpi", "on" })) {
        qWarning() << "[AutoSetupManager] Failed to enable ACPI";
    }
    
    // Enable IO APIC
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--ioapic", "on" })) {
        qWarning() << "[AutoSetupManager] Failed to enable IO APIC";
    }
    
    // Set boot order to disk
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--boot1", "disk" })) {
        qWarning() << "[AutoSetupManager] Failed to set boot order";
    }
    
    // Enable 3D acceleration
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--accelerate3d", "on" })) {
        qWarning() << "[AutoSetupManager] Failed to enable 3D acceleration";
    }
    
    // Set network to NAT
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--nic1", "nat" })) {
        qWarning() << "[AutoSetupManager] Failed to set network";
    }
    
    // Configure port forwarding for ADB
    if (!executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--natpf1", "adb,tcp,5555,,5555" })) {
        qWarning() << "[AutoSetupManager] Failed to setup port forwarding";
    }
    
    return true;
}

bool AutoSetupManager::setVMResources(const QString& vmName, int cpuCores, int memoryMB, int vramMB) {
    bool success = true;
    
    success &= executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--cpus", QString::number(cpuCores) });
    success &= executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--memory", QString::number(memoryMB) });
    success &= executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--vram", QString::number(vramMB) });
    
    return success;
}

bool AutoSetupManager::configureVMNetwork(const QString& vmName, const QString& mode) {
    QString nicType = (mode == "nat") ? "nat" : "bridged";
    return executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--nic1", nicType });
}

bool AutoSetupManager::configureVMStorage(const QString& vmName, const QString& diskPath, int sizeGB) {
    Q_UNUSED(vmName);
    Q_UNUSED(diskPath);
    Q_UNUSED(sizeGB);
    // Storage is created during VM creation
    return true;
}

// ============================================================================
// ADB Access
// ============================================================================
bool AutoSetupManager::setupADBConnection(const QString& vmName, int& outPort) {
    // Allocate a port
    int port = 5555 + (QRandomGenerator::global()->bounded(100));
    
    // Configure port forwarding for this VM
    QString forwardingRule = QString("adb_%1,tcp,%2,,%2").arg(vmName).arg(port);
    executeCommandSync(m_vboxPath, { "modifyvm", vmName, "--natpf1", forwardingRule });
    
    outPort = port;
    
    // Connect ADB
    QString device = QString("127.0.0.1:%1").arg(port);
    executeCommandSync(m_adbPath, { "connect", device });
    
    return true;
}

bool AutoSetupManager::waitForADB(const QString& vmName, int timeoutMs) {
    qInfo() << "[AutoSetupManager] Waiting for ADB on VM:" << vmName;
    
    int elapsed = 0;
    int interval = 2000;
    
    while (elapsed < timeoutMs) {
        QString output;
        if (executeCommand(m_adbPath, { "devices" }, &output, 5000)) {
            if (output.contains("device")) {
                qInfo() << "[AutoSetupManager] ADB connected";
                return true;
            }
        }
        
        QThread::msleep(interval);
        elapsed += interval;
    }
    
    qWarning() << "[AutoSetupManager] ADB connection timeout";
    return false;
}

QString AutoSetupManager::getVMIP(const QString& vmName) {
    int port;
    if (!setupADBConnection(vmName, port)) {
        return QString();
    }
    
    QString device = QString("127.0.0.1:%1").arg(port);
    QString output;
    
    if (executeCommand(m_adbPath, { "-s", device, "shell", "ip", "route" }, &output)) {
        // Parse IP from output
        QStringList lines = output.split('\n');
        for (const QString& line : lines) {
            if (line.contains("wlan0")) {
                QStringList parts = line.split(' ');
                for (int i = 0; i < parts.size(); i++) {
                    if (parts[i] == "src" && i + 1 < parts.size()) {
                        return parts[i + 1];
                    }
                }
            }
        }
    }
    
    return QString();
}

// ============================================================================
// Path Configuration
// ============================================================================
void AutoSetupManager::setVMStoragePath(const QString& path) {
    m_vmStoragePath = path;
    saveConfig();
}

QString AutoSetupManager::getVMStoragePath() const {
    return m_vmStoragePath;
}

void AutoSetupManager::setDownloadPath(const QString& path) {
    m_downloadPath = path;
    saveConfig();
}

QString AutoSetupManager::getDownloadPath() const {
    return m_downloadPath;
}

void AutoSetupManager::setAndroidVersion(const QString& version) {
    m_androidVersion = version;
    saveConfig();
}

QString AutoSetupManager::getAndroidVersion() const {
    return m_androidVersion;
}

void AutoSetupManager::setGAppsVariant(const QString& variant) {
    m_gappsVariant = variant;
    saveConfig();
}

QString AutoSetupManager::getGAppsVariant() const {
    return m_gappsVariant;
}

// ============================================================================
// Cleanup
// ============================================================================
bool AutoSetupManager::cleanupAllVMs() {
    qInfo() << "[AutoSetupManager] Cleaning up all VMs...";
    
    QString output;
    if (executeCommand(m_vboxPath, { "list", "vms" }, &output)) {
        QStringList lines = output.split('\n');
        for (const QString& line : lines) {
            if (line.contains('{')) {
                int start = line.indexOf('{');
                int end = line.indexOf('}');
                if (start != -1 && end != -1) {
                    QString vmName = line.mid(line.indexOf('"') + 1);
                    vmName = vmName.left(vmName.lastIndexOf('"'));
                    
                    if (vmName.startsWith("VPhone_") || vmName == m_vmTemplateName) {
                        deleteVM(vmName);
                    }
                }
            }
        }
    }
    
    return true;
}

bool AutoSetupManager::cleanupTemplate() {
    return deleteVM(m_vmTemplateName);
}

// ============================================================================
// Internal Helpers
// ============================================================================
QString AutoSetupManager::getDefaultStoragePath() const {
#ifdef _WIN32
    wchar_t* path = nullptr;
    if (SHGetKnownFolderPath(FOLDERID_VirtualBox, 0, nullptr, &path) == S_OK) {
        QString result = QString::fromWCharT(path) + "\\VPhonePro";
        CoTaskMemFree(path);
        return result;
    }
#endif
    return QDir::homePath() + "/VirtualBox VMs/VPhonePro";
}

QString AutoSetupManager::getDefaultDownloadPath() const {
    return QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/VPhonePro";
}

bool AutoSetupManager::executeCommand(const QString& program, const QStringList& args, QString* output, int timeoutMs) {
    QProcess proc;
    proc.start(program, args);
    
    if (!proc.waitForStarted(5000)) {
        return false;
    }
    
    if (!proc.waitForFinished(timeoutMs)) {
        proc.kill();
        return false;
    }
    
    if (output) {
        *output = proc.readAllStandardOutput() + proc.readAllStandardError();
    }
    
    return proc.exitCode() == 0;
}

bool AutoSetupManager::executeCommandSync(const QString& program, const QStringList& args, int timeoutMs) {
    return executeCommand(program, args, nullptr, timeoutMs);
}

// ============================================================================
// VM Operations
// ============================================================================
bool AutoSetupManager::createBaseVM(const QString& vmName) {
    qInfo() << "[AutoSetupManager] Creating base VM:" << vmName;
    
    // Create VM
    QStringList createArgs = { "createvm", "--name", vmName, "--register" };
    if (!executeCommandSync(m_vboxPath, createArgs)) {
        m_lastError = "Failed to create VM: " + vmName;
        return false;
    }
    
    // Set OS type
    QStringList typeArgs = { "modifyvm", vmName, "--ostype", "Linux_64" };
    executeCommandSync(m_vboxPath, typeArgs);
    
    return true;
}

bool AutoSetupManager::registerVM(const QString& vmName) {
    QStringList args = { "registervm", vmName };
    return executeCommandSync(m_vboxPath, args);
}

bool AutoSetupManager::deleteVM(const QString& vmName) {
    qInfo() << "[AutoSetupManager] Deleting VM:" << vmName;
    
    // Power off if running
    stopVM(vmName, true);
    
    // Unregister and delete
    QStringList args = { "unregistervm", vmName, "--delete" };
    bool success = executeCommandSync(m_vboxPath, args);
    
    if (!success) {
        qWarning() << "[AutoSetupManager] Failed to delete VM:" << vmName;
    }
    
    return success;
}

bool AutoSetupManager::startVM(const QString& vmName, const QString& type) {
    qInfo() << "[AutoSetupManager] Starting VM:" << vmName << "Type:" << type;
    
    QStringList args = { "startvm", vmName, "--type", type };
    return executeCommandSync(m_vboxPath, args);
}

bool AutoSetupManager::stopVM(const QString& vmName, bool force) {
    if (force) {
        QStringList args = { "controlvm", vmName, "poweroff" };
        return executeCommandSync(m_vboxPath, args);
    } else {
        QStringList args = { "controlvm", vmName, "acpipowerbutton" };
        return executeCommandSync(m_vboxPath, args);
    }
}

bool AutoSetupManager::isVMRunning(const QString& vmName) {
    return getVMState(vmName) == "running";
}

bool AutoSetupManager::waitForVMState(const QString& vmName, const QString& state, int timeoutMs) {
    int elapsed = 0;
    while (elapsed < timeoutMs) {
        if (getVMState(vmName) == state) {
            return true;
        }
        QThread::msleep(1000);
        elapsed += 1000;
    }
    return false;
}

QString AutoSetupManager::getVMState(const QString& vmName) {
    QString output;
    QStringList args = { "showvminfo", vmName, "--machinereadable" };
    
    if (executeCommand(m_vboxPath, args, &output)) {
        for (const QString& line : output.split('\n')) {
            if (line.startsWith("VMState=")) {
                return line.section('=', 1).remove('"').trimmed();
            }
        }
    }
    
    return "unknown";
}

bool AutoSetupManager::mountISO(const QString& vmName, const QString& isoPath) {
    qInfo() << "[AutoSetupManager] Mounting ISO:" << isoPath << "to" << vmName;
    
    QStringList args = {
        "storageattach", vmName,
        "--storagectl", "IDE Controller",
        "--port", "0",
        "--device", "0",
        "--type", "dvddrive",
        "--medium", isoPath
    };
    
    return executeCommandSync(m_vboxPath, args);
}

bool AutoSetupManager::unmountISO(const QString& vmName) {
    QStringList args = {
        "storageattach", vmName,
        "--storagectl", "IDE Controller",
        "--port", "0",
        "--device", "0",
        "--type", "dvddrive",
        "--medium", "emptydrive"
    };
    
    return executeCommandSync(m_vboxPath, args);
}

bool AutoSetupManager::attachDisk(const QString& vmName, const QString& diskPath) {
    QStringList args = {
        "storageattach", vmName,
        "--storagectl", "IDE Controller",
        "--port", "0",
        "--device", "0",
        "--type", "hdd",
        "--medium", diskPath
    };
    
    return executeCommandSync(m_vboxPath, args);
}

// ============================================================================
// Configuration
// ============================================================================
void AutoSetupManager::saveConfig() {
    QMutexLocker locker(&m_configMutex);
    
    QJsonObject config;
    config["vmStoragePath"] = m_vmStoragePath;
    config["downloadPath"] = m_downloadPath;
    config["isoPath"] = m_isoPath;
    config["gappsPath"] = m_gappsPath;
    config["androidVersion"] = m_androidVersion;
    config["gappsVariant"] = m_gappsVariant;
    config["vmTemplateName"] = m_vmTemplateName;
    
    QJsonDocument doc(config);
    QFile file(m_configPath);
    
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson());
        file.close();
    }
}

void AutoSetupManager::loadConfig() {
    QMutexLocker locker(&m_configMutex);
    
    QFile file(m_configPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject config = doc.object();
    
    m_vmStoragePath = config["vmStoragePath"].toString(m_vmStoragePath);
    m_downloadPath = config["downloadPath"].toString(m_downloadPath);
    m_isoPath = config["isoPath"].toString();
    m_gappsPath = config["gappsPath"].toString();
    m_androidVersion = config["androidVersion"].toString("14");
    m_gappsVariant = config["gappsVariant"].toString("pico");
    m_vmTemplateName = config["vmTemplateName"].toString(VM_TEMPLATE_NAME);
    
    file.close();
}

// ============================================================================
// Download
// ============================================================================
void AutoSetupManager::startISODownload() {
    // Determine URL based on version
    QString url;
    if (m_androidVersion == "14") {
        url = ANDROID_X86_URL_14;
    } else if (m_androidVersion == "13") {
        url = ANDROID_X86_URL_13;
    } else if (m_androidVersion == "12") {
        url = ANDROID_X86_URL_12;
    } else {
        url = ANDROID_X86_URL_11;
    }
    
    QString filename = QString("android-x86_%1.iso").arg(m_androidVersion);
    QString savePath = QDir(m_downloadPath).filePath(filename);
    
    m_isoPath = savePath;
    
    // Start download using QNetworkAccessManager
    QUrl downloadUrl(url);
    QNetworkRequest request(downloadUrl);
    
    m_downloadReply = m_networkManager->get(request);
    m_downloadFile = new QFile(savePath);
    m_downloadFile->open(QIODevice::WriteOnly);
    
    m_downloadCancelled = false;
    
    connect(m_downloadReply, &QNetworkReply::readyRead, this, &AutoSetupManager::onDownloadReadyRead);
    connect(m_downloadReply, &QNetworkReply::finished, this, &AutoSetupManager::onDownloadFinished);
    connect(m_downloadReply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::error),
            this, &AutoSetupManager::onDownloadError);
}

void AutoSetupManager::onDownloadFinished() {
    if (m_downloadFile) {
        m_downloadFile->close();
    }
    
    if (m_downloadCancelled) {
        qInfo() << "[AutoSetupManager] Download cancelled";
        return;
    }
    
    if (m_downloadReply->error() == QNetworkReply::NoError) {
        qInfo() << "[AutoSetupManager] Download completed successfully";
        emit downloadProgress(m_totalBytesToDownload, m_totalBytesToDownload);
        saveConfig();
    }
    
    m_downloadReply->deleteLater();
    m_downloadReply = nullptr;
}

void AutoSetupManager::onDownloadReadyRead() {
    if (m_downloadFile && m_downloadReply) {
        m_downloadFile->write(m_downloadReply->readAll());
        
        qint64 received = m_downloadReply->bytesAvailable();
        if (m_totalBytesToDownload > 0) {
            emit downloadProgress(received, m_totalBytesToDownload);
        }
    }
}

void AutoSetupManager::onDownloadError(QNetworkReply::NetworkError error) {
    qWarning() << "[AutoSetupManager] Download error:" << error;
    m_lastError = QString("Download error: %1").arg(error);
    emit errorOccurred(m_lastError);
}

} // namespace VirtualPhonePro