/**
 * VirtualBoxController.hpp
 * 
 * Headless VirtualBox VM Controller for VirtualPhonePro
 * Handles launching, monitoring, and terminating VMs in headless mode
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QObject>
#include <QProcess>
#include <QString>
#include <QTimer>
#include <QVariantMap>
#include <atomic>
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

enum class VMLaunchState {
    IDLE,
    STARTING,
    WAITING_FOR_BOOT,
    READY,
    RUNNING,
    STOPPING,
    STOPPED,
    ERROR
};

enum class VMType {
    VIRTUALBOX,
    ANDROID_X86
};

struct VBoxVMInfo {
    QString vmName;
    QString vmId;
    QString profileId;
    int adbPort;
    VMLaunchState state;
    QString errorMessage;
    qint64 pid;
    QDateTime startTime;
};

class VirtualBoxController : public QObject {
    Q_OBJECT

public:
    static VirtualBoxController& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // VM Operations
    bool launchVM(const QString& vmName, const QString& profileId, int adbPort);
    bool launchVMHeadless(const QString& vmName);
    bool stopVM(const QString& vmName);
    bool forceStopVM(const QString& vmName);
    bool pauseVM(const QString& vmName);
    bool resumeVM(const QString& vmName);
    
    // VM Queries
    bool isVMRunning(const QString& vmName);
    VMLaunchState getVMState(const QString& vmName);
    QString getVMUUID(const QString& vmName);
    int getRunningVMCount();
    
    // Port Management
    int allocateADBPort();
    void releaseADBPort(int port);
    bool isADBPortAvailable(int port);
    
    // ADB Connection
    bool waitForADB(int port, int timeoutMs = 30000);
    bool connectADB(const QString& vmName, int port);
    bool isADBConnected(const QString& vmName);
    
    // VM Info
    VBoxVMInfo getVMInfo(const QString& vmName);
    QList<VBoxVMInfo> getAllRunningVMs();
    
    // Configuration
    void setVBoxPath(const QString& path);
    QString getVBoxPath() const;
    void setADBPath(const QString& path);
    QString getADBPath() const;
    void setBootTimeout(int milliseconds);
    int getBootTimeout() const;
    
signals:
    void vmStarted(const QString& vmName, const QString& profileId);
    void vmStopped(const QString& vmName);
    void vmError(const QString& vmName, const QString& error);
    void vmStateChanged(const QString& vmName, VMLaunchState state);
    void adbConnected(const QString& vmName, int port);
    void adbDisconnected(const QString& vmName);
    void bootCompleted(const QString& vmName);

private:
    explicit VirtualBoxController(QObject* parent = nullptr);
    ~VirtualBoxController();
    VirtualBoxController(const VirtualBoxController&) = delete;
    VirtualBoxController& operator=(const VirtualBoxController&) = delete;
    
    // Internal helpers
    QString executeVBoxCommand(const QStringList& args);
    bool executeVBoxCommandSync(const QStringList& args, QString* output = nullptr);
    bool isVBoxInstalled();
    bool vmExists(const QString& vmName);
    
    void onVMProcessStarted();
    void onVMProcessError(QProcess::ProcessError error);
    void onVMProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
    void checkVMStates();
    void monitorBootProgress(const QString& vmName);
    
    // State management
    QMap<QString, VBoxVMInfo> m_runningVMs;
    QMap<QString, QProcess*> m_processes;
    std::mutex m_mutex;
    
    // Port allocation
    QSet<int> m_allocatedPorts;
    int m_nextPort;
    std::mutex m_portMutex;
    
    // Configuration
    QString m_vboxPath;
    QString m_adbPath;
    int m_bootTimeout;
    
    // Monitoring
    QTimer* m_stateCheckTimer;
    QTimer* m_adbCheckTimer;
    
    // VBox installation check
    bool m_initialized;
    QString m_lastError;
};

} // namespace VirtualPhonePro