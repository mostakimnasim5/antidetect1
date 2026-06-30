/**
 * VMController.cpp
 * 
 * Main Orchestrator Implementation
 */

#include "vmcontroller/VMController.hpp"
#include <QDebug>
#include <QCoreApplication>
#include <QFileInfo>
#include <QDir>
#include <QThread>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
VMController& VMController::getInstance() {
    static VMController instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
VMController::VMController(QObject* parent)
    : QObject(parent)
    , m_bootTimeout(60000)
    , m_initialized(false)
{
#ifdef _WIN32
    m_vboxPath = "VBoxManage.exe";
    m_adbPath = "adb.exe";
    m_scrcpyPath = "scrcpy.exe";
#else
    m_vboxPath = "VBoxManage";
    m_adbPath = "adb";
    m_scrcpyPath = "scrcpy";
#endif
}

VMController::~VMController() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool VMController::initialize() {
    if (m_initialized) {
        return true;
    }
    
    qInfo() << "[VMController] Initializing...";
    
    // Initialize all subsystems
    if (!VirtualBoxController::getInstance().initialize()) {
        qWarning() << "[VMController] VirtualBox initialization failed";
        // Don't fail - VBox might not be installed yet
    }
    
    if (!ADBSpoofer::getInstance().initialize()) {
        qWarning() << "[VMController] ADB Spoofer initialization failed";
        // Don't fail - ADB might not be available yet
    }
    
    if (!ScrcpyManager::getInstance().initialize()) {
        qWarning() << "[VMController] Scrcpy Manager initialization failed";
        // Don't fail - scrcpy might not be available yet
    }
    
    WindowEmbedder::getInstance().initialize();
    
    // Set paths from environment or defaults
    VirtualBoxController::getInstance().setVBoxPath(m_vboxPath);
    VirtualBoxController::getInstance().setADBPath(m_adbPath);
    ADBSpoofer::getInstance().setADBPath(m_adbPath);
    ScrcpyManager::getInstance().setScrcpyPath(m_scrcpyPath);
    
    m_initialized = true;
    qInfo() << "[VMController] Initialization complete";
    
    return true;
}

void VMController::shutdown() {
    qInfo() << "[VMController] Shutting down...";
    
    // Stop all VMs
    stopAllVMs();
    
    // Shutdown subsystems
    VirtualBoxController::getInstance().shutdown();
    ADBSpoofer::getInstance().shutdown();
    ScrcpyManager::getInstance().shutdown();
    WindowEmbedder::getInstance().shutdown();
    
    // Clear display widgets
    for (auto widget : m_displayWidgets) {
        if (widget) {
            widget->deleteLater();
        }
    }
    m_displayWidgets.clear();
    
    m_initialized = false;
    qInfo() << "[VMController] Shutdown complete";
}

bool VMController::isInitialized() const {
    return m_initialized;
}

// ============================================================================
// VM Instance Management
// ============================================================================
QString VMController::launchVM(const FingerprintConfig& profile, const QString& vmName) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_initialized) {
        qWarning() << "[VMController] Not initialized";
        return "";
    }
    
    QString instanceId = generateInstanceId();
    
    qInfo() << "[VMController] Launching VM:" << instanceId << "profile:" << profile.manufacturer << profile.model;
    
    // Create instance info
    VMInstance instance;
    instance.profileId = instanceId;
    instance.vmName = vmName.isEmpty() ? QString("VPhone_%1").arg(instanceId) : vmName;
    instance.profileName = QString("%1 %2").arg(profile.manufacturer).arg(profile.model);
    instance.state = VMControllerState::IDLE;
    instance.windowTitle = generateUniqueWindowTitle(instanceId);
    instance.fingerprint = profile;
    
    // Convert FingerprintConfig to SpoofConfig
    instance.spoofConfig.latitude = profile.latitude;
    instance.spoofConfig.longitude = profile.longitude;
    instance.spoofConfig.timezone = profile.timezone;
    instance.spoofConfig.language = profile.locale.split('_').first();
    instance.spoofConfig.country = profile.locale.split('_').last();
    instance.spoofConfig.manufacturer = profile.manufacturer;
    instance.spoofConfig.model = profile.model;
    instance.spoofConfig.brand = profile.brand;
    instance.spoofConfig.wifiMac = profile.macAddress;
    instance.spoofConfig.screenWidth = profile.screenWidth;
    instance.spoofConfig.screenHeight = profile.screenHeight;
    instance.spoofConfig.screenDPI = profile.screenDPI;
    
    // Allocate ADB port
    instance.adbPort = VirtualBoxController::getInstance().allocateADBPort();
    
    // Store instance
    m_instances[instanceId] = instance;
    
    // Start launch sequence
    QTimer::singleShot(0, this, [this, instanceId]() {
        startLaunchSequence(instanceId);
    });
    
    emit vmLaunching(instanceId);
    updateInstanceState(instanceId, VMControllerState::LAUNCHING);
    
    return instanceId;
}

bool VMController::stopVM(const QString& instanceId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    
    qInfo() << "[VMController] Stopping VM:" << instanceId;
    
    VMInstance& instance = m_instances[instanceId];
    updateInstanceState(instanceId, VMControllerState::STOPPING);
    
    // Stop Scrcpy
    ScrcpyManager::getInstance().stopScrcpy(instanceId);
    
    // Unembed window
    if (m_displayWidgets.contains(instanceId)) {
        auto widget = m_displayWidgets[instanceId];
        widget->clearEmbeddedWindow();
    }
    WindowEmbedder::getInstance().unembedWindow(instance.windowTitle);
    
    // Stop VirtualBox VM
    VirtualBoxController::getInstance().forceStopVM(instance.vmName);
    
    // Release ADB port
    VirtualBoxController::getInstance().releaseADBPort(instance.adbPort);
    
    // Cleanup
    cleanupInstance(instanceId);
    
    emit vmStopped(instanceId);
    emit allVMsStopped();
    
    return true;
}

bool VMController::stopAllVMs() {
    QStringList ids = m_instances.keys();
    for (const QString& id : ids) {
        stopVM(id);
    }
    return true;
}

bool VMController::restartVM(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    
    VMInstance instance = m_instances[instanceId];
    FingerprintConfig profile = instance.fingerprint;
    
    stopVM(instanceId);
    
    // Small delay
    QThread::msleep(1000);
    
    return !launchVM(profile, instance.vmName).isEmpty();
}

// ============================================================================
// Instance Queries
// ============================================================================
bool VMController::isVMReady(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    return m_instances[instanceId].state == VMControllerState::READY ||
           m_instances[instanceId].state == VMControllerState::RUNNING;
}

bool VMController::isVMRunning(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    return m_instances[instanceId].state == VMControllerState::RUNNING;
}

VMInstance VMController::getInstance(const QString& instanceId) {
    return m_instances.value(instanceId);
}

QList<VMInstance> VMController::getAllInstances() {
    return m_instances.values();
}

int VMController::getRunningInstanceCount() {
    int count = 0;
    for (const auto& instance : m_instances) {
        if (instance.state == VMControllerState::RUNNING) {
            count++;
        }
    }
    return count;
}

QString VMController::getInstanceIdByPort(int adbPort) {
    for (auto it = m_instances.begin(); it != m_instances.end(); ++it) {
        if (it.value().adbPort == adbPort) {
            return it.key();
        }
    }
    return QString();
}

// ============================================================================
// Display Integration
// ============================================================================
VMDisplayWidget* VMController::createDisplayWidget(const QString& instanceId, QWidget* parent) {
    if (!m_instances.contains(instanceId)) {
        return nullptr;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    // Create widget
    VMDisplayWidget* widget = new VMDisplayWidget(parent);
    widget->setProfileId(instance.profileId);
    widget->setDeviceModel(instance.profileName);
    widget->setDeviceScreenSize(instance.fingerprint.screenWidth, instance.fingerprint.screenHeight);
    widget->setStatus("Initializing...");
    
    // Store reference
    m_displayWidgets[instanceId] = widget;
    
    emit displayReady(instanceId, widget);
    
    return widget;
}

bool VMController::attachDisplay(const QString& instanceId, VMDisplayWidget* widget) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    // Get Scrcpy window handle
    HWND hwnd = ScrcpyManager::getInstance().getWindowHandle(instanceId);
    
    if (hwnd) {
        // Embed window into widget
        WId containerId = widget->winId();
        
        if (WindowEmbedder::getInstance().embedWindowByHandle(hwnd, containerId)) {
            widget->setEmbeddedWindow(hwnd);
            widget->setStatus("Connected");
            
            // Start resize monitoring
            WindowEmbedder::getInstance().startWindowMonitoring(instance.windowTitle, 100);
            
            return true;
        }
    }
    
    return false;
}

bool VMController::detachDisplay(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    // Stop monitoring
    WindowEmbedder::getInstance().stopWindowMonitoring(instance.windowTitle);
    
    // Unembed window
    WindowEmbedder::getInstance().unembedWindow(instance.windowTitle);
    
    // Clear widget
    if (m_displayWidgets.contains(instanceId)) {
        m_displayWidgets[instanceId]->clearEmbeddedWindow();
    }
    
    return true;
}

// ============================================================================
// Hardware Controls
// ============================================================================
void VMController::sendTap(const QString& instanceId, int x, int y) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "shell", "input", "tap", 
                           QString::number(x), QString::number(y) });
    proc.waitForFinished(2000);
}

void VMController::sendSwipe(const QString& instanceId, int x1, int y1, int x2, int y2, int durationMs) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "shell", "input", "swipe",
                           QString::number(x1), QString::number(y1),
                           QString::number(x2), QString::number(y2),
                           QString::number(durationMs) });
    proc.waitForFinished(2000);
}

void VMController::sendText(const QString& instanceId, const QString& text) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    // Escape special characters
    QString escaped = text;
    escaped.replace(" ", "%s");
    escaped.replace("'", "\\'");
    
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "shell", "input", "text", escaped });
    proc.waitForFinished(2000);
}

void VMController::sendKey(const QString& instanceId, int keyCode) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "shell", "input", "keyevent", 
                           QString::number(keyCode) });
    proc.waitForFinished(2000);
}

void VMController::pressHome(const QString& instanceId) {
    sendKey(instanceId, 3); // KEYCODE_HOME
}

void VMController::pressBack(const QString& instanceId) {
    sendKey(instanceId, 4); // KEYCODE_BACK
}

void VMController::pressPower(const QString& instanceId) {
    sendKey(instanceId, 26); // KEYCODE_POWER
}

void VMController::volumeUp(const QString& instanceId) {
    sendKey(instanceId, 24); // KEYCODE_VOLUME_UP
}

void VMController::volumeDown(const QString& instanceId) {
    sendKey(instanceId, 25); // KEYCODE_VOLUME_DOWN
}

void VMController::pullNotifications(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) return;
    
    // Get screen size
    VMInstance& instance = m_instances[instanceId];
    int width = instance.fingerprint.screenWidth;
    int height = instance.fingerprint.screenHeight;
    
    // Swipe from top to pull notification bar
    sendSwipe(instanceId, width / 2, 100, width / 2, height / 2, 300);
}

void VMController::rotateScreen(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    // Set rotation via settings
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "shell", "settings", "put", "system",
                           "user_rotation", "1" });
    proc.waitForFinished(2000);
}

// ============================================================================
// APK Installation
// ============================================================================
void VMController::installAPK(const QString& instanceId, const QString& apkPath) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    qInfo() << "[VMController] Installing APK:" << apkPath << "to" << instanceId;
    
    // Update status
    if (m_displayWidgets.contains(instanceId)) {
        m_displayWidgets[instanceId]->setStatus("Installing APK...");
    }
    
    // Install via ADB
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "install", "-r", apkPath });
    proc.waitForFinished(60000); // 60 second timeout for large APKs
    
    QString output = proc.readAllStandardOutput();
    QString error = proc.readAllStandardError();
    
    if (proc.exitCode() == 0 && output.contains("Success")) {
        qInfo() << "[VMController] APK installed successfully";
        if (m_displayWidgets.contains(instanceId)) {
            m_displayWidgets[instanceId]->setStatus("APK Installed");
        }
    } else {
        qWarning() << "[VMController] APK installation failed:" << error;
        if (m_displayWidgets.contains(instanceId)) {
            m_displayWidgets[instanceId]->setStatus("Installation Failed");
        }
    }
}

void VMController::uninstallPackage(const QString& instanceId, const QString& packageName) {
    if (!m_instances.contains(instanceId)) return;
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    QProcess proc;
    proc.start(m_adbPath, { "-s", device, "uninstall", packageName });
    proc.waitForFinished(10000);
}

// ============================================================================
// Profile Management
// ============================================================================
bool VMController::applyProfile(const QString& instanceId, const FingerprintConfig& profile) {
    if (!m_instances.contains(instanceId)) {
        return false;
    }
    
    VMInstance& instance = m_instances[instanceId];
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    qInfo() << "[VMController] Applying profile to instance:" << instanceId;
    
    // Convert to SpoofConfig and apply
    SpoofConfig config;
    config.latitude = profile.latitude;
    config.longitude = profile.longitude;
    config.timezone = profile.timezone;
    config.language = profile.locale.split('_').first();
    config.country = profile.locale.split('_').last();
    config.manufacturer = profile.manufacturer;
    config.model = profile.model;
    config.brand = profile.brand;
    config.wifiMac = profile.macAddress;
    config.screenWidth = profile.screenWidth;
    config.screenHeight = profile.screenHeight;
    config.screenDPI = profile.screenDPI;
    
    bool success = ADBSpoofer::getInstance().applySpoofToDevice(device, config);
    
    if (success) {
        instance.fingerprint = profile;
        emit spoofApplied(instanceId);
    } else {
        emit spoofError(instanceId, "Failed to apply profile");
    }
    
    return success;
}

FingerprintConfig VMController::generateRandomProfile(const QString& country) {
    // Use the existing DeviceIdentityGenerator or create a simple one
    FingerprintConfig profile;
    
    // Device profiles
    QStringList manufacturers = {"Samsung", "Google", "Xiaomi", "OnePlus", "Huawei", "Sony", "LG"};
    QStringList samsungModels = {"SM-G998B", "SM-N986B", "SM-G973F", "SM-A515F"};
    QStringList pixelModels = {"Pixel 7", "Pixel 6", "Pixel 5", "Pixel 4"};
    QStringList xiaomiModels = {"Mi 11", "Redmi Note 10", "POCO F3", "Mi 10"};
    
    QString manufacturer = manufacturers[QRandomGenerator::global()->bounded(manufacturers.size())];
    profile.manufacturer = manufacturer;
    profile.brand = manufacturer;
    
    if (manufacturer == "Samsung") {
        profile.model = samsungModels[QRandomGenerator::global()->bounded(samsungModels.size())];
    } else if (manufacturer == "Google") {
        profile.model = pixelModels[QRandomGenerator::global()->bounded(pixelModels.size())];
    } else if (manufacturer == "Xiaomi") {
        profile.model = xiaomiModels[QRandomGenerator::global()->bounded(xiaomiModels.size())];
    } else {
        profile.model = QString("%1 %2").arg(manufacturer).arg(QRandomGenerator::global()->bounded(1, 12));
    }
    
    profile.androidVersion = QString::number(QRandomGenerator::global()->bounded(11, 15));
    profile.securityPatch = "2024-06-01";
    profile.buildId = QString("RP1A.%1").arg(QRandomGenerator::global()->bounded(200000, 999999));
    profile.bootloader = QString("G%1BXXU1").arg(QRandomGenerator::global()->bounded(1, 10));
    
    // Hardware
    profile.cpuModel = "qcom";
    profile.cpuABI = "arm64-v8a";
    profile.cpuCores = 8;
    profile.gpuVendor = "Qualcomm";
    profile.gpuRenderer = "Adreno 660";
    
    // Screen
    profile.screenWidth = 1080;
    profile.screenHeight = 2400;
    profile.screenDPI = 480;
    
    // Location
    profile.latitude = 23.8103 + (QRandomGenerator::global()->bounded(-10, 10) * 0.1);
    profile.longitude = 90.4125 + (QRandomGenerator::global()->bounded(-10, 10) * 0.1);
    profile.timezone = "Asia/Dhaka";
    profile.locale = "en_US";
    
    // Network
    profile.macAddress = QString("%1:%2:%3:%4:%5:%6")
        .arg(2, 2, 16, QChar('0')).arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
        .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
        .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
        .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'))
        .arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0'));
    
    return profile;
}

// ============================================================================
// Configuration
// ============================================================================
void VMController::setVBoxPath(const QString& path) {
    m_vboxPath = path;
    VirtualBoxController::getInstance().setVBoxPath(path);
}

void VMController::setADBPath(const QString& path) {
    m_adbPath = path;
    VirtualBoxController::getInstance().setADBPath(path);
    ADBSpoofer::getInstance().setADBPath(path);
}

void VMController::setScrcpyPath(const QString& path) {
    m_scrcpyPath = path;
    ScrcpyManager::getInstance().setScrcpyPath(path);
}

void VMController::setAssetsPath(const QString& path) {
    m_assetsPath = path;
}

void VMController::setBootTimeout(int milliseconds) {
    m_bootTimeout = milliseconds;
    VirtualBoxController::getInstance().setBootTimeout(milliseconds);
}

// ============================================================================
// Callbacks
// ============================================================================
void VMController::setLaunchCallback(std::function<void(const QString&, bool)> callback) {
    m_launchCallback = callback;
}

void VMController::setErrorCallback(std::function<void(const QString&, const QString&)> callback) {
    m_errorCallback = callback;
}

void VMController::setStatusCallback(std::function<void(const QString&, const QString&)> callback) {
    m_statusCallback = callback;
}

// ============================================================================
// Launch Sequence
// ============================================================================
void VMController::startLaunchSequence(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    qInfo() << "[VMController] Starting launch sequence for:" << instanceId;
    
    // Step 1: Launch VirtualBox VM in headless mode
    updateInstanceState(instanceId, VMControllerState::LAUNCHING);
    emit vmLaunching(instanceId);
    
    bool launchSuccess = VirtualBoxController::getInstance().launchVM(
        instance.vmName, instanceId, instance.adbPort);
    
    if (!launchSuccess) {
        instance.errorMessage = "Failed to launch VirtualBox VM";
        updateInstanceState(instanceId, VMControllerState::ERROR);
        emit vmError(instanceId, instance.errorMessage);
        if (m_errorCallback) m_errorCallback(instanceId, instance.errorMessage);
        return;
    }
    
    // Step 2: Wait for boot and apply spoofing
    QTimer::singleShot(5000, this, [this, instanceId]() {
        onBootCompleted(instanceId);
    });
}

void VMController::onVMLaunched(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    qInfo() << "[VMController] VM launched:" << instanceId;
    
    // Connect ADB
    VirtualBoxController::getInstance().connectADB(instance.vmName, instance.adbPort);
}

void VMController::onBootCompleted(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    qInfo() << "[VMController] Boot completed, applying spoofing:" << instanceId;
    
    updateInstanceState(instanceId, VMControllerState::SPOOFING);
    
    // Apply ADB spoofing
    QString device = QString("127.0.0.1:%1").arg(instance.adbPort);
    
    bool spoofSuccess = ADBSpoofer::getInstance().applySpoofToDevice(device, instance.spoofConfig);
    
    if (spoofSuccess) {
        qInfo() << "[VMController] Spoofing applied successfully";
        emit spoofApplied(instanceId);
    }
    
    // Step 3: Start Scrcpy
    onSpoofingComplete(instanceId);
}

void VMController::onSpoofingComplete(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    qInfo() << "[VMController] Starting Scrcpy for:" << instanceId;
    
    updateInstanceState(instanceId, VMControllerState::CONNECTING_SCRCPY);
    
    // Configure Scrcpy
    ScrcpyConfig scrcpyConfig;
    scrcpyConfig.windowTitle = instance.windowTitle;
    scrcpyConfig.adbPort = instance.adbPort;
    scrcpyConfig.maxFPS = 60;
    scrcpyConfig.disableAudio = true;
    scrcpyConfig.stayAwake = true;
    scrcpyConfig.disableScreenOff = true;
    
    // Launch Scrcpy
    ScrcpyManager::getInstance().launchScrcpy(instanceId, instance.adbPort, scrcpyConfig);
    
    // Connect Scrcpy signals
    connect(&ScrcpyManager::getInstance(), &ScrcpyManager::scrcpyStarted,
            this, [this, instanceId](const QString& profileId, HWND hwnd) {
        if (profileId == instanceId) {
            onScrcpyReady(instanceId);
        }
    });
    
    // Also handle via timer fallback
    QTimer::singleShot(5000, this, [this, instanceId]() {
        onScrcpyReady(instanceId);
    });
}

void VMController::onScrcpyReady(const QString& instanceId) {
    if (!m_instances.contains(instanceId)) {
        return;
    }
    
    VMInstance& instance = m_instances[instanceId];
    
    qInfo() << "[VMController] Scrcpy ready for:" << instanceId;
    
    updateInstanceState(instanceId, VMControllerState::READY);
    
    // Attach to display widget if exists
    if (m_displayWidgets.contains(instanceId)) {
        attachDisplay(instanceId, m_displayWidgets[instanceId]);
    }
    
    updateInstanceState(instanceId, VMControllerState::RUNNING);
    emit vmReady(instanceId);
    
    if (m_launchCallback) {
        m_launchCallback(instanceId, true);
    }
    
    qInfo() << "[VMController] VM fully ready:" << instanceId;
}

// ============================================================================
// Private Helpers
// ============================================================================
QString VMController::generateInstanceId() {
    static int counter = 0;
    return QString("vm_%1_%2").arg(QDateTime::currentMSecsSinceEpoch()).arg(++counter);
}

QString VMController::generateUniqueWindowTitle(const QString& profileId) {
    static int windowCounter = 0;
    return QString("VPhone_Window_%1_%2").arg(profileId).arg(++windowCounter);
}

void VMController::updateInstanceState(const QString& instanceId, VMControllerState state) {
    if (!m_instances.contains(instanceId)) {
        return;
    }
    
    VMInstance& instance = m_instances[instanceId];
    instance.state = state;
    
    emit stateChanged(instanceId, state);
    
    // Update display widget status if exists
    if (m_displayWidgets.contains(instanceId)) {
        QString status;
        switch (state) {
            case VMControllerState::IDLE: status = "Idle"; break;
            case VMControllerState::LAUNCHING: status = "Launching..."; break;
            case VMControllerState::SPOOFING: status = "Applying Spoofing..."; break;
            case VMControllerState::CONNECTING_SCRCPY: status = "Connecting Display..."; break;
            case VMControllerState::READY: status = "Ready"; break;
            case VMControllerState::RUNNING: status = "Running"; break;
            case VMControllerState::STOPPING: status = "Stopping..."; break;
            case VMControllerState::ERROR: status = "Error"; break;
        }
        m_displayWidgets[instanceId]->setStatus(status);
    }
    
    if (m_statusCallback) {
        m_statusCallback(instanceId, "");
    }
}

void VMController::cleanupInstance(const QString& instanceId) {
    if (m_instances.contains(instanceId)) {
        VMInstance& instance = m_instances[instanceId];
        VirtualBoxController::getInstance().releaseADBPort(instance.adbPort);
        m_instances.remove(instanceId);
    }
    
    if (m_displayWidgets.contains(instanceId)) {
        VMDisplayWidget* widget = m_displayWidgets.take(instanceId);
        if (widget) {
            widget->deleteLater();
        }
    }
}

} // namespace VirtualPhonePro