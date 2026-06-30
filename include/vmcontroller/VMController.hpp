/**
 * VMController.hpp
 * 
 * Main Orchestrator for VirtualPhonePro VM Management
 * Coordinates VirtualBox, ADB Spoofing, Scrcpy, and Window Embedding
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include "VirtualBoxController.hpp"
#include "ADBSpoofer.hpp"
#include "ScrcpyManager.hpp"
#include "WindowEmbedder.hpp"
#include "VMDisplayWidget.hpp"

#include <QObject>
#include <QString>
#include <QMap>
#include <QTimer>
#include <QMutex>
#include <QThread>
#include <QWaitCondition>

namespace VirtualPhonePro {

enum class VMControllerState {
    IDLE,
    LAUNCHING,
    SPOOFING,
    CONNECTING_SCRCPY,
    READY,
    RUNNING,
    STOPPING,
    ERROR
};

struct VMInstance {
    QString profileId;
    QString vmName;
    QString profileName;
    int adbPort;
    VMControllerState state;
    QString windowTitle;
    FingerprintConfig fingerprint;
    SpoofConfig spoofConfig;
    QString errorMessage;
};

class VMController : public QObject {
    Q_OBJECT

public:
    static VMController& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // VM Instance Management
    QString launchVM(const FingerprintConfig& profile, const QString& vmName = "");
    bool stopVM(const QString& instanceId);
    bool stopAllVMs();
    bool restartVM(const QString& instanceId);
    
    // Instance Queries
    bool isVMReady(const QString& instanceId);
    bool isVMRunning(const QString& instanceId);
    VMInstance getInstance(const QString& instanceId);
    QList<VMInstance> getAllInstances();
    int getRunningInstanceCount();
    QString getInstanceIdByPort(int adbPort);
    
    // Display Integration
    VMDisplayWidget* createDisplayWidget(const QString& instanceId, QWidget* parent = nullptr);
    bool attachDisplay(const QString& instanceId, VMDisplayWidget* widget);
    bool detachDisplay(const QString& instanceId);
    
    // Hardware Controls
    void sendTap(const QString& instanceId, int x, int y);
    void sendSwipe(const QString& instanceId, int x1, int y1, int x2, int y2, int durationMs = 300);
    void sendText(const QString& instanceId, const QString& text);
    void sendKey(const QString& instanceId, int keyCode);
    void pressHome(const QString& instanceId);
    void pressBack(const QString& instanceId);
    void pressPower(const QString& instanceId);
    void volumeUp(const QString& instanceId);
    void volumeDown(const QString& instanceId);
    void pullNotifications(const QString& instanceId);
    void rotateScreen(const QString& instanceId);
    
    // APK Installation
    void installAPK(const QString& instanceId, const QString& apkPath);
    void uninstallPackage(const QString& instanceId, const QString& packageName);
    
    // Profile Management
    bool applyProfile(const QString& instanceId, const FingerprintConfig& profile);
    FingerprintConfig generateRandomProfile(const QString& country = "");
    
    // Configuration
    void setVBoxPath(const QString& path);
    void setADBPath(const QString& path);
    void setScrcpyPath(const QString& path);
    void setAssetsPath(const QString& path);
    void setBootTimeout(int milliseconds);
    
    // Callbacks
    void setLaunchCallback(std::function<void(const QString&, bool)> callback);
    void setErrorCallback(std::function<void(const QString&, const QString&)> callback);
    void setStatusCallback(std::function<void(const QString&, const QString&)> callback);

signals:
    void vmLaunching(const QString& instanceId);
    void vmReady(const QString& instanceId);
    void vmStopped(const QString& instanceId);
    void vmError(const QString& instanceId, const QString& error);
    void displayReady(const QString& instanceId, VMDisplayWidget* widget);
    void spoofApplied(const QString& instanceId);
    void spoofError(const QString& instanceId, const QString& error);
    void stateChanged(const QString& instanceId, VMControllerState state);
    void allVMsStopped();

private:
    explicit VMController(QObject* parent = nullptr);
    ~VMController();
    VMController(const VMController&) = delete;
    VMController& operator=(const VMController&) = delete;
    
    // Launch sequence
    void startLaunchSequence(const QString& instanceId);
    void onVMLaunched(const QString& instanceId);
    void onBootCompleted(const QString& instanceId);
    void onSpoofingComplete(const QString& instanceId);
    void onScrcpyReady(const QString& instanceId);
    
    // Internal helpers
    QString generateInstanceId();
    QString generateUniqueWindowTitle(const QString& profileId);
    void updateInstanceState(const QString& instanceId, VMControllerState state);
    void cleanupInstance(const QString& instanceId);
    
    // Component accessors
    VirtualBoxController& getVBox() { return VirtualBoxController::getInstance(); }
    ADBSpoofer& getADBSpoofer() { return ADBSpoofer::getInstance(); }
    ScrcpyManager& getScrcpy() { return ScrcpyManager::getInstance(); }
    WindowEmbedder& getEmbedder() { return WindowEmbedder::getInstance(); }
    
    // State
    QMap<QString, VMInstance> m_instances;
    QMap<QString, VMDisplayWidget*> m_displayWidgets;
    QMap<QString, SpoofConfig> m_pendingSpoofing;
    QMutex m_mutex;
    
    // Configuration
    QString m_vboxPath;
    QString m_adbPath;
    QString m_scrcpyPath;
    QString m_assetsPath;
    int m_bootTimeout;
    
    // Callbacks
    std::function<void(const QString&, bool)> m_launchCallback;
    std::function<void(const QString&, const QString&)> m_errorCallback;
    std::function<void(const QString&, const QString&)> m_statusCallback;
    
    // Initialization
    bool m_initialized;
    
    // Scrcpy launch wait
    QMap<QString, QThread*> m_launchThreads;
    QMap<QString, QWaitCondition*> m_launchWaits;
};

} // namespace VirtualPhonePro