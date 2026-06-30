/**
 * ScrcpyManager.hpp
 * 
 * Scrcpy Process Manager for VirtualPhonePro
 * Handles launching, controlling, and monitoring Scrcpy instances
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QMap>
#include <QMutex>
#include <QRect>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

enum class ScrcpyStatus {
    STOPPED,
    STARTING,
    RUNNING,
    ERROR
};

struct ScrcpyConfig {
    QString windowTitle;
    int adbPort;
    int maxFPS = 60;
    int bitrate = 8000000;  // 8 Mbps
    int maxSize = 0;        // 0 = auto
    bool showTouches = false;
    bool stayAwake = true;
    bool disableScreenOff = true;
    bool turnScreenOff = false;
    bool showFps = false;
    bool disableAudio = true;
    bool crop = "";         // e.g., "1920:1080:0:0"
    bool fullscreen = false;
    bool alwaysOnTop = false;
    QString windowX = "";    // "" = auto
    QString windowY = "";
    QString windowWidth = "";
    QString windowHeight = "";
};

struct ScrcpyInfo {
    QString profileId;
    int adbPort;
    ScrcpyStatus status;
    qint64 pid;
    HWND windowHandle;
    QString windowTitle;
    QDateTime startTime;
    QString errorMessage;
};

class ScrcpyManager : public QObject {
    Q_OBJECT

public:
    static ScrcpyManager& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // Instance Management
    QString launchScrcpy(const QString& profileId, int adbPort, const ScrcpyConfig& config = ScrcpyConfig());
    bool launchScrcpyAsync(const QString& profileId, int adbPort, const ScrcpyConfig& config = ScrcpyConfig());
    bool stopScrcpy(const QString& profileId);
    bool stopAllScrcpy();
    bool restartScrcpy(const QString& profileId);
    
    // Scrcpy Configuration
    void setScrcpyPath(const QString& path);
    QString getScrcpyPath() const;
    void setDefaultConfig(const ScrcpyConfig& config);
    ScrcpyConfig getDefaultConfig() const;
    
    // Status
    ScrcpyStatus getStatus(const QString& profileId) const;
    ScrcpyInfo getInfo(const QString& profileId) const;
    QList<ScrcpyInfo> getAllRunningInstances() const;
    int getRunningCount() const;
    bool isRunning(const QString& profileId) const;
    
    // Window Control
    bool bringToFront(const QString& profileId);
    bool setWindowTitle(const QString& profileId, const QString& title);
    QString getWindowTitle(const QString& profileId) const;
    HWND getWindowHandle(const QString& profileId) const;
    
    // Display Control
    bool setDisplayOrientation(const QString& profileId, bool landscape);
    bool rotateDisplay(const QString& profileId);
    
    // Input Control (via Scrcpy control socket)
    bool sendText(const QString& profileId, const QString& text);
    bool sendKeyEvent(const QString& profileId, int keyCode);
    bool sendTap(const QString& profileId, int x, int y);
    bool sendSwipe(const QString& profileId, int x1, int y1, int x2, int y2, int durationMs);
    bool sendScroll(const QString& profileId, int x, int y, int hScroll, int vScroll);
    
    // Hardware Buttons (via ADB)
    bool pressPowerButton(const QString& profileId);
    bool pressHomeButton(const QString& profileId);
    bool pressBackButton(const QString& profileId);
    bool pressVolumeUp(const QString& profileId);
    bool pressVolumeDown(const QString& profileId);
    bool pullNotificationBar(const QString& profileId);
    bool expandNotificationBar(const QString& profileId);
    
    // Scrcpy-specific controls
    bool setMaxFPS(const QString& profileId, int fps);
    bool setBitrate(const QString& profileId, int bitrate);
    bool setScreenPowerMode(const QString& profileId, bool on);
    bool clipboardSetText(const QString& profileId, const QString& text);
    bool clipboardGetText(const QString& profileId);
    
signals:
    void scrcpyStarted(const QString& profileId, HWND windowHandle);
    void scrcpyStopped(const QString& profileId);
    void scrcpyError(const QString& profileId, const QString& error);
    void scrcpyStatusChanged(const QString& profileId, ScrcpyStatus status);
    void windowReady(const QString& profileId, HWND windowHandle);

private:
    explicit ScrcpyManager(QObject* parent = nullptr);
    ~ScrcpyManager();
    ScrcpyManager(const ScrcpyManager&) = delete;
    ScrcpyManager& operator=(const ScrcpyManager&) = delete;
    
    // Process management
    void onProcessStarted(const QString& profileId);
    void onProcessError(const QString& profileId, QProcess::ProcessError error);
    void onProcessFinished(const QString& profileId, int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessReadyRead(const QString& profileId);
    
    // Window detection
    void startWindowDetection(const QString& profileId, const QString& windowTitle);
    void stopWindowDetection(const QString& profileId);
    HWND detectWindowHandle(const QString& windowTitle);
    
    // Helpers
    QString buildScrcpyCommand(const QString& profileId, const ScrcpyConfig& config);
    QString escapeTextForScrcpy(const QString& text);
    QString generateWindowTitle(const QString& profileId);
    
    // State
    QMap<QString, ScrcpyInfo> m_instances;
    QMap<QString, QProcess*> m_processes;
    QMap<QString, QTimer*> m_detectionTimers;
    QMap<QString, QString> m_windowTitles;
    QMutex m_mutex;
    
    // Configuration
    QString m_scrcpyPath;
    ScrcpyConfig m_defaultConfig;
    
    // Window title prefix for uniqueness
    QString m_titlePrefix;
    int m_titleCounter;
    
    // Initialized flag
    bool m_initialized;
    
    // ADB path for hardware controls
    QString m_adbPath;
};

} // namespace VirtualPhonePro