/**
 * ScrcpyManager.cpp
 * 
 * Implementation of Scrcpy Process Manager
 */

#include "vmcontroller/ScrcpyManager.hpp"
#include <QDebug>
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
ScrcpyManager& ScrcpyManager::getInstance() {
    static ScrcpyManager instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
ScrcpyManager::ScrcpyManager(QObject* parent)
    : QObject(parent)
    , m_titlePrefix("VPhone_Window_")
    , m_titleCounter(0)
    , m_initialized(false)
{
#ifdef _WIN32
    m_scrcpyPath = "scrcpy.exe";
    m_adbPath = "adb.exe";
#else
    m_scrcpyPath = "scrcpy";
    m_adbPath = "adb";
#endif
    
    // Default configuration
    m_defaultConfig.maxFPS = 60;
    m_defaultConfig.bitrate = 8000000;
    m_defaultConfig.showTouches = false;
    m_defaultConfig.stayAwake = true;
    m_defaultConfig.disableScreenOff = true;
    m_defaultConfig.disableAudio = true;
}

ScrcpyManager::~ScrcpyManager() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool ScrcpyManager::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Check if scrcpy exists
    QFileInfo scrcpyFile(m_scrcpyPath);
    if (!scrcpyFile.exists()) {
        qWarning() << "[ScrcpyManager] scrcpy not found at:" << m_scrcpyPath;
        // Don't fail - user might use relative path or add to PATH
    }
    
    m_initialized = true;
    qInfo() << "[ScrcpyManager] Initialized successfully";
    return true;
}

void ScrcpyManager::shutdown() {
    qInfo() << "[ScrcpyManager] Shutting down...";
    
    // Stop all scrcpy instances
    stopAllScrcpy();
    
    // Stop all detection timers
    for (auto timer : m_detectionTimers) {
        timer->stop();
        delete timer;
    }
    m_detectionTimers.clear();
    
    m_initialized = false;
    qInfo() << "[ScrcpyManager] Shutdown complete";
}

bool ScrcpyManager::isInitialized() const {
    return m_initialized;
}

// ============================================================================
// Instance Management
// ============================================================================
QString ScrcpyManager::launchScrcpy(const QString& profileId, int adbPort, const ScrcpyConfig& config) {
    QString windowTitle = generateWindowTitle(profileId);
    
    // Check if already running
    if (m_instances.contains(profileId)) {
        ScrcpyInfo& info = m_instances[profileId];
        if (info.status == ScrcpyStatus::RUNNING || info.status == ScrcpyStatus::STARTING) {
            qInfo() << "[ScrcpyManager] Already running:" << profileId;
            return info.windowTitle;
        }
    }
    
    qInfo() << "[ScrcpyManager] Launching scrcpy for profile:" << profileId << "port:" << adbPort;
    
    // Build configuration
    ScrcpyConfig finalConfig = config;
    finalConfig.profileId = profileId;
    finalConfig.adbPort = adbPort;
    finalConfig.windowTitle = windowTitle;
    
    // Store window title
    m_windowTitles[profileId] = windowTitle;
    
    // Create process
    QProcess* process = new QProcess(this);
    m_processes[profileId] = process;
    
    // Connect signals
    connect(process, &QProcess::started, [this, profileId]() {
        onProcessStarted(profileId);
    });
    connect(process, QOverload<QProcess::ProcessError>::of(&QProcess::errorOccurred),
            [this, profileId](QProcess::ProcessError error) {
        onProcessError(profileId, error);
    });
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this, profileId](int exitCode, QProcess::ExitStatus exitStatus) {
        onProcessFinished(profileId, exitCode, exitStatus);
    });
    connect(process, &QProcess::readyReadStandardOutput,
            [this, profileId]() {
        onProcessReadyRead(profileId);
    });
    connect(process, &QProcess::readyReadStandardError,
            [this, profileId]() {
        onProcessReadyRead(profileId);
    });
    
    // Build command
    QString command = buildScrcpyCommand(profileId, finalConfig);
    qDebug() << "[ScrcpyManager] Command:" << command;
    
    // Update instance info
    ScrcpyInfo info;
    info.profileId = profileId;
    info.adbPort = adbPort;
    info.status = ScrcpyStatus::STARTING;
    info.windowTitle = windowTitle;
    info.startTime = QDateTime::currentDateTime();
    m_instances[profileId] = info;
    
    // Start process
    QStringList args = parseScrcpyArgs(finalConfig);
    process->start(m_scrcpyPath, args);
    
    // Start window detection
    startWindowDetection(profileId, windowTitle);
    
    emit scrcpyStatusChanged(profileId, ScrcpyStatus::STARTING);
    
    return windowTitle;
}

bool ScrcpyManager::launchScrcpyAsync(const QString& profileId, int adbPort, const ScrcpyConfig& config) {
    launchScrcpy(profileId, adbPort, config);
    return true;
}

bool ScrcpyManager::stopScrcpy(const QString& profileId) {
    QMutexLocker locker(&m_mutex);
    
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    qInfo() << "[ScrcpyManager] Stopping scrcpy for profile:" << profileId;
    
    // Stop window detection
    stopWindowDetection(profileId);
    
    // Kill process
    if (m_processes.contains(profileId)) {
        QProcess* proc = m_processes.take(profileId);
        if (proc->state() != QProcess::NotRunning) {
            proc->kill();
            proc->waitForFinished(2000);
        }
        proc->deleteLater();
    }
    
    // Update status
    m_instances[profileId].status = ScrcpyStatus::STOPPED;
    
    emit scrcpyStopped(profileId);
    emit scrcpyStatusChanged(profileId, ScrcpyStatus::STOPPED);
    
    return true;
}

bool ScrcpyManager::stopAllScrcpy() {
    QStringList profiles = m_instances.keys();
    for (const QString& profileId : profiles) {
        stopScrcpy(profileId);
    }
    return true;
}

bool ScrcpyManager::restartScrcpy(const QString& profileId) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    ScrcpyInfo info = m_instances[profileId];
    int adbPort = info.adbPort;
    
    stopScrcpy(profileId);
    
    // Small delay before restart
    QThread::msleep(500);
    
    return !launchScrcpy(profileId, adbPort).isEmpty();
}

// ============================================================================
// Scrcpy Configuration
// ============================================================================
void ScrcpyManager::setScrcpyPath(const QString& path) {
    m_scrcpyPath = path;
}

QString ScrcpyManager::getScrcpyPath() const {
    return m_scrcpyPath;
}

void ScrcpyManager::setDefaultConfig(const ScrcpyConfig& config) {
    m_defaultConfig = config;
}

ScrcpyConfig ScrcpyManager::getDefaultConfig() const {
    return m_defaultConfig;
}

// ============================================================================
// Status
// ============================================================================
ScrcpyStatus ScrcpyManager::getStatus(const QString& profileId) const {
    if (m_instances.contains(profileId)) {
        return m_instances[profileId].status;
    }
    return ScrcpyStatus::STOPPED;
}

ScrcpyInfo ScrcpyManager::getInfo(const QString& profileId) const {
    return m_instances.value(profileId);
}

QList<ScrcpyInfo> ScrcpyManager::getAllRunningInstances() const {
    QList<ScrcpyInfo> running;
    for (const auto& info : m_instances) {
        if (info.status == ScrcpyStatus::RUNNING) {
            running.append(info);
        }
    }
    return running;
}

int ScrcpyManager::getRunningCount() const {
    int count = 0;
    for (const auto& info : m_instances) {
        if (info.status == ScrcpyStatus::RUNNING) {
            count++;
        }
    }
    return count;
}

bool ScrcpyManager::isRunning(const QString& profileId) const {
    if (m_instances.contains(profileId)) {
        return m_instances[profileId].status == ScrcpyStatus::RUNNING;
    }
    return false;
}

// ============================================================================
// Window Control
// ============================================================================
bool ScrcpyManager::bringToFront(const QString& profileId) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    HWND hwnd = m_instances[profileId].windowHandle;
    if (!hwnd) {
        return false;
    }
    
#ifdef _WIN32
    SetWindowPos(hwnd, HWND_TOP, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
    SetForegroundWindow(hwnd);
    SetActiveWindow(hwnd);
#endif
    
    return true;
}

bool ScrcpyManager::setWindowTitle(const QString& profileId, const QString& title) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    m_instances[profileId].windowTitle = title;
    m_windowTitles[profileId] = title;
    
    HWND hwnd = m_instances[profileId].windowHandle;
    if (hwnd) {
#ifdef _WIN32
        std::wstring wTitle = title.toStdWString();
        SetWindowTextW(hwnd, wTitle.c_str());
#endif
    }
    
    return true;
}

QString ScrcpyManager::getWindowTitle(const QString& profileId) const {
    return m_windowTitles.value(profileId);
}

HWND ScrcpyManager::getWindowHandle(const QString& profileId) const {
    if (m_instances.contains(profileId)) {
        return m_instances[profileId].windowHandle;
    }
    return nullptr;
}

// ============================================================================
// Display Control
// ============================================================================
bool ScrcpyManager::setDisplayOrientation(const QString& profileId, bool landscape) {
    // Scrcpy doesn't have direct orientation control
    // Would need to send shell command to Android
    Q_UNUSED(profileId);
    Q_UNUSED(landscape);
    return false;
}

bool ScrcpyManager::rotateDisplay(const QString& profileId) {
    return sendKeyEvent(profileId, 18); // KEYCODE_ROTATE_SCREEN or similar
}

// ============================================================================
// Input Control
// ============================================================================
bool ScrcpyManager::sendText(const QString& profileId, const QString& text) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    // For text input, we need to use scrcpy's control features
    // This would typically go through a control socket or adb
    QString escaped = escapeTextForScrcpy(text);
    
    // Use ADB to input text via input text command
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    proc.start(m_adbPath, { "-s", device, "shell", "input", "text", escaped });
    proc.waitForFinished(2000);
    
    return proc.exitCode() == 0;
}

bool ScrcpyManager::sendKeyEvent(const QString& profileId, int keyCode) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    proc.start(m_adbPath, { "-s", device, "shell", "input", "keyevent", QString::number(keyCode) });
    proc.waitForFinished(2000);
    
    return proc.exitCode() == 0;
}

bool ScrcpyManager::sendTap(const QString& profileId, int x, int y) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    proc.start(m_adbPath, { "-s", device, "shell", "input", "tap", 
                           QString::number(x), QString::number(y) });
    proc.waitForFinished(2000);
    
    return proc.exitCode() == 0;
}

bool ScrcpyManager::sendSwipe(const QString& profileId, int x1, int y1, int x2, int y2, int durationMs) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    proc.start(m_adbPath, { "-s", device, "shell", "input", "swipe",
                           QString::number(x1), QString::number(y1),
                           QString::number(x2), QString::number(y2),
                           QString::number(durationMs) });
    proc.waitForFinished(2000);
    
    return proc.exitCode() == 0;
}

bool ScrcpyManager::sendScroll(const QString& profileId, int x, int y, int hScroll, int vScroll) {
    // Android doesn't have a direct scroll command
    // Use swipe with fast movement for scrolling effect
    int swipeDistance = vScroll > 0 ? -300 : 300;
    return sendSwipe(profileId, x, y, x, y + swipeDistance, 200);
}

// ============================================================================
// Hardware Buttons (via ADB)
// ============================================================================
bool ScrcpyManager::pressPowerButton(const QString& profileId) {
    // KeyEvent 26 = KEYCODE_POWER
    return sendKeyEvent(profileId, 26);
}

bool ScrcpyManager::pressHomeButton(const QString& profileId) {
    // KeyEvent 3 = KEYCODE_HOME
    return sendKeyEvent(profileId, 3);
}

bool ScrcpyManager::pressBackButton(const QString& profileId) {
    // KeyEvent 4 = KEYCODE_BACK
    return sendKeyEvent(profileId, 4);
}

bool ScrcpyManager::pressVolumeUp(const QString& profileId) {
    // KeyEvent 24 = KEYCODE_VOLUME_UP
    return sendKeyEvent(profileId, 24);
}

bool ScrcpyManager::pressVolumeDown(const QString& profileId) {
    // KeyEvent 25 = KEYCODE_VOLUME_DOWN
    return sendKeyEvent(profileId, 25);
}

bool ScrcpyManager::pullNotificationBar(const QString& profileId) {
    // Pull down notification bar (swipe from top)
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    
    // Get screen size first
    proc.start(m_adbPath, { "-s", device, "shell", "wm", "size" });
    proc.waitForFinished(2000);
    QString output = proc.readAllStandardOutput();
    
    // Parse resolution and calculate center-top position
    int width = 1080, height = 1920; // Default
    QRegularExpression re("(\\d+)x(\\d+)");
    auto match = re.match(output);
    if (match.hasMatch()) {
        width = match.captured(1).toInt();
        height = match.captured(2).toInt();
    }
    
    // Swipe from center-top
    return sendSwipe(profileId, width/2, height/4, width/2, height/2, 300);
}

bool ScrcpyManager::expandNotificationBar(const QString& profileId) {
    return pullNotificationBar(profileId);
}

// ============================================================================
// Scrcpy-specific controls
// ============================================================================
bool ScrcpyManager::setMaxFPS(const QString& profileId, int fps) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    // Scrcpy doesn't support runtime FPS change
    // Would need to restart with new config
    Q_UNUSED(profileId);
    Q_UNUSED(fps);
    return false;
}

bool ScrcpyManager::setBitrate(const QString& profileId, int bitrate) {
    Q_UNUSED(profileId);
    Q_UNUSED(bitrate);
    return false; // Would need restart
}

bool ScrcpyManager::setScreenPowerMode(const QString& profileId, bool on) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    QString mode = on ? "on" : "off";
    proc.start(m_adbPath, { "-s", device, "shell", "svc", "power", mode });
    proc.waitForFinished(2000);
    
    return proc.exitCode() == 0;
}

bool ScrcpyManager::clipboardSetText(const QString& profileId, const QString& text) {
    if (!m_instances.contains(profileId)) {
        return false;
    }
    
    QProcess proc;
    QString device = QString("127.0.0.1:%1").arg(m_instances[profileId].adbPort);
    QString escaped = escapeTextForScrcpy(text);
    proc.start(m_adbPath, { "-s", device, "shell", "input", "text", escaped });
    proc.waitForFinished(2000);
    
    return proc.exitCode() == 0;
}

bool ScrcpyManager::clipboardGetText(const QString& profileId) {
    Q_UNUSED(profileId);
    // Would need to read clipboard via scrcpy control socket
    return false;
}

// ============================================================================
// Process Management
// ============================================================================
void ScrcpyManager::onProcessStarted(const QString& profileId) {
    qInfo() << "[ScrcpyManager] Process started for profile:" << profileId;
    
    if (m_instances.contains(profileId)) {
        m_instances[profileId].status = ScrcpyStatus::STARTING;
        m_instances[profileId].pid = m_processes[profileId]->processId();
        emit scrcpyStatusChanged(profileId, ScrcpyStatus::STARTING);
    }
}

void ScrcpyManager::onProcessError(const QString& profileId, QProcess::ProcessError error) {
    qWarning() << "[ScrcpyManager] Process error for profile:" << profileId << "error:" << error;
    
    if (m_instances.contains(profileId)) {
        m_instances[profileId].status = ScrcpyStatus::ERROR;
        m_instances[profileId].errorMessage = QString("Process error: %1").arg(error);
        emit scrcpyError(profileId, m_instances[profileId].errorMessage);
        emit scrcpyStatusChanged(profileId, ScrcpyStatus::ERROR);
    }
}

void ScrcpyManager::onProcessFinished(const QString& profileId, int exitCode, QProcess::ExitStatus exitStatus) {
    qInfo() << "[ScrcpyManager] Process finished for profile:" << profileId 
            << "exitCode:" << exitCode << "status:" << exitStatus;
    
    // Stop window detection
    stopWindowDetection(profileId);
    
    if (m_instances.contains(profileId)) {
        m_instances[profileId].status = ScrcpyStatus::STOPPED;
        emit scrcpyStopped(profileId);
        emit scrcpyStatusChanged(profileId, ScrcpyStatus::STOPPED);
    }
}

void ScrcpyManager::onProcessReadyRead(const QString& profileId) {
    QProcess* proc = m_processes.value(profileId);
    if (!proc) return;
    
    QString output = proc->readAllStandardOutput();
    QString error = proc->readAllStandardError();
    
    if (!output.isEmpty()) {
        qDebug() << "[ScrcpyManager]" << output;
    }
    if (!error.isEmpty()) {
        qDebug() << "[ScrcpyManager] ERROR:" << error;
    }
}

// ============================================================================
// Window Detection
// ============================================================================
void ScrcpyManager::startWindowDetection(const QString& profileId, const QString& windowTitle) {
    if (m_detectionTimers.contains(profileId)) {
        return;
    }
    
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, profileId, windowTitle]() {
        HWND hwnd = detectWindowHandle(windowTitle);
        
        if (hwnd && m_instances.contains(profileId)) {
            m_instances[profileId].windowHandle = hwnd;
            m_instances[profileId].status = ScrcpyStatus::RUNNING;
            
            // Stop detection once found
            stopWindowDetection(profileId);
            
            emit scrcpyStarted(profileId, hwnd);
            emit windowReady(profileId, hwnd);
            emit scrcpyStatusChanged(profileId, ScrcpyStatus::RUNNING);
            
            qInfo() << "[ScrcpyManager] Window detected for profile:" << profileId;
        }
    });
    
    timer->start(500); // Check every 500ms
    m_detectionTimers[profileId] = timer;
}

void ScrcpyManager::stopWindowDetection(const QString& profileId) {
    if (m_detectionTimers.contains(profileId)) {
        m_detectionTimers[profileId]->stop();
        delete m_detectionTimers.take(profileId);
    }
}

HWND ScrcpyManager::detectWindowHandle(const QString& windowTitle) {
#ifdef _WIN32
    std::wstring wTitle = windowTitle.toStdWString();
    return FindWindowW(nullptr, wTitle.c_str());
#else
    return nullptr;
#endif
}

// ============================================================================
// Helpers
// ============================================================================
QString ScrcpyManager::buildScrcpyCommand(const QString& profileId, const ScrcpyConfig& config) {
    Q_UNUSED(profileId);
    
    QStringList args = parseScrcpyArgs(config);
    return m_scrcpyPath + " " + args.join(" ");
}

QStringList ScrcpyManager::parseScrcpyArgs(const ScrcpyConfig& config) {
    QStringList args;
    
    // Connection
    args << "-s" << QString("127.0.0.1:%1").arg(config.adbPort);
    
    // Window title (unique per profile)
    args << "--window-title" << config.windowTitle;
    
    // Display settings
    if (config.maxFPS > 0) {
        args << "--max-fps" << QString::number(config.maxFPS);
    }
    
    if (config.bitrate > 0) {
        args << "-b" << QString("%1M").arg(config.bitrate / 1000000);
    }
    
    if (config.maxSize > 0) {
        args << "--max-size" << QString::number(config.maxSize);
    }
    
    if (config.showTouches) {
        args << "--show-touches";
    }
    
    if (config.stayAwake) {
        args << "--stay-awake";
    }
    
    if (config.disableScreenOff) {
        args << "--disable-screen-off";
    }
    
    if (config.disableAudio) {
        args << "--no-audio";
    }
    
    if (config.showFps) {
        args << "--show-fps";
    }
    
    if (!config.crop.isEmpty()) {
        args << "--crop" << config.crop;
    }
    
    if (config.fullscreen) {
        args << "-f";
    }
    
    if (config.alwaysOnTop) {
        args << "--always-on-top";
    }
    
    if (!config.windowWidth.isEmpty() && !config.windowHeight.isEmpty()) {
        args << "-w" << config.windowWidth << "-h" << config.windowHeight;
    }
    
    if (!config.windowX.isEmpty() && !config.windowY.isEmpty()) {
        args << "-x" << config.windowX << "-y" << config.windowY;
    }
    
    return args;
}

QString ScrcpyManager::escapeTextForScrcpy(const QString& text) {
    // Escape special characters for Android input text
    QString escaped = text;
    escaped.replace(" ", "%s");  // Space
    escaped.replace("'", "\\'");
    escaped.replace("\"", "\\\"");
    return escaped;
}

QString ScrcpyManager::generateWindowTitle(const QString& profileId) {
    m_titleCounter++;
    return QString("%1%2_%3").arg(m_titlePrefix).arg(profileId).arg(m_titleCounter);
}

} // namespace VirtualPhonePro