/**
 * AutoSetupManager.hpp
 * 
 * Automatic Android-x86 VM Setup System for VirtualPhonePro
 * Handles ISO download, VM creation, Google Apps installation, and VM cloning
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QObject>
#include <QString>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QTimer>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMutex>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#endif

namespace VirtualPhonePro {

enum class SetupState {
    IDLE,
    CHECKING_DEPENDENCIES,
    DOWNLOADING_ISO,
    CREATING_VM,
    INSTALLING_ANDROID,
    FLASHING_GAPPS,
    SETUP_COMPLETE,
    ERROR
};

enum class SetupStep {
    NONE,
    CHECKING_VIRTUALBOX,
    CHECKING_ADB,
    CHECKING_SCRCPY,
    DOWNLOADING_ISO,
    CREATING_VM_TEMPLATE,
    INSTALLING_GUEST_ADDITIONS,
    FLASHING_GAPPS,
    CREATING_PROFILE_VM,
    FINALIZING
};

struct SetupProgress {
    SetupStep currentStep;
    int progressPercent;
    QString currentAction;
    QString message;
    qint64 bytesDownloaded;
    qint64 totalBytes;
    double downloadSpeed;
    int estimatedSecondsRemaining;
};

struct SystemRequirements {
    bool virtualBoxInstalled;
    bool adbInstalled;
    bool scrcpyInstalled;
    int cpuCores;
    int totalRAM;
    int freeDiskSpace;
    QString virtualBoxVersion;
    QString adbVersion;
    QString scrcpyVersion;
};

class AutoSetupManager : public QObject {
    Q_OBJECT

public:
    static AutoSetupManager& getInstance();
    
    // ============================================================================
    // System Requirements Check
    // ============================================================================
    SystemRequirements checkSystemRequirements();
    bool isVirtualBoxInstalled();
    bool isADBInstalled();
    bool isScrcpyInstalled();
    QString getVirtualBoxVersion();
    QString getADBVersion();
    
    // ============================================================================
    // Setup Management
    // ============================================================================
    bool isSetupComplete() const;
    bool needsSetup() const;
    QString getSetupError() const;
    
    // ============================================================================
    // ISO Management
    // ============================================================================
    QString getISOPath() const;
    bool isISODownloaded() const;
    qint64 getISOSize() const;
    
    // Download ISO (returns true if successful, false if cancelled)
    bool downloadISO(QWidget* parentWidget = nullptr);
    void cancelDownload();
    
    // Alternative: Use local ISO
    bool setLocalISO(const QString& isoPath);
    
    // ============================================================================
    // VM Template Management
    // ============================================================================
    QString getVMTemplateName() const;
    bool isVMTemplateCreated() const;
    
    // Create base VM template
    bool createVMTemplate(const QString& vmName = "VPhonePro_Template");
    
    // Install Android to template VM
    bool installAndroidToTemplate();
    
    // Clone VM from template for a profile
    QString cloneVMForProfile(const QString& profileId, const QString& profileName);
    
    // Delete profile VM
    bool deleteProfileVM(const QString& profileId);
    
    // ============================================================================
    // Google Apps (GApps) Management
    // ============================================================================
    QString getGAppsPath() const;
    bool downloadGApps();
    bool flashGApps(const QString& vmName);
    bool isGAppsInstalled(const QString& vmName);
    
    // ============================================================================
    // Android Installation
    // ============================================================================
    bool installAndroid(const QString& vmName);
    bool isAndroidInstalled(const QString& vmName);
    bool isAndroidBooted(const QString& vmName, int timeoutMs = 60000);
    
    // ============================================================================
    // Guest Additions
    // ============================================================================
    bool installGuestAdditions(const QString& vmName);
    bool isGuestAdditionsInstalled(const QString& vmName);
    
    // ============================================================================
    // VM Configuration
    // ============================================================================
    bool configureVMForAndroid(const QString& vmName);
    bool setVMResources(const QString& vmName, int cpuCores, int memoryMB, int vramMB);
    bool configureVMNetwork(const QString& vmName, const QString& mode = "nat");
    bool configureVMStorage(const QString& vmName, const QString& diskPath, int sizeGB);
    
    // ============================================================================
    // ADB Access
    // ============================================================================
    bool setupADBConnection(const QString& vmName, int& outPort);
    bool waitForADB(const QString& vmName, int timeoutMs = 60000);
    QString getVMIP(const QString& vmName);
    
    // ============================================================================
    // Path Configuration
    // ============================================================================
    void setVMStoragePath(const QString& path);
    QString getVMStoragePath() const;
    void setDownloadPath(const QString& path);
    QString getDownloadPath() const;
    
    // ============================================================================
    // Configuration
    // ============================================================================
    void setAndroidVersion(const QString& version);  // "11", "12", "13", "14"
    QString getAndroidVersion() const;
    void setGAppsVariant(const QString& variant);   // "pico", "nano", "mini", "micro", "full", "stock"
    QString getGAppsVariant() const;
    
    // ============================================================================
    // Cleanup
    // ============================================================================
    bool cleanupAllVMs();
    bool cleanupTemplate();
    
signals:
    void setupStateChanged(SetupState state);
    void setupProgressChanged(const SetupProgress& progress);
    void setupCompleted(bool success, const QString& message);
    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void stepStarted(SetupStep step, const QString& description);
    void stepCompleted(SetupStep step);
    void errorOccurred(const QString& error);

private:
    explicit AutoSetupManager(QObject* parent = nullptr);
    ~AutoSetupManager();
    AutoSetupManager(const AutoSetupManager&) = delete;
    AutoSetupManager& operator=(const AutoSetupManager&) = delete;
    
    // Internal helpers
    QString getDefaultStoragePath() const;
    QString getDefaultDownloadPath() const;
    bool executeCommand(const QString& program, const QStringList& args, QString* output = nullptr, int timeoutMs = 30000);
    bool executeCommandSync(const QString& program, const QStringList& args, int timeoutMs = 30000);
    
    // ISO Download
    void startISODownload();
    void onDownloadFinished();
    void onDownloadReadyRead();
    void onDownloadError(QNetworkReply::NetworkError error);
    
    // VM Operations
    bool createBaseVM(const QString& vmName);
    bool registerVM(const QString& vmName);
    bool deleteVM(const QString& vmName);
    bool startVM(const QString& vmName, const QString& type = "gui");
    bool stopVM(const QString& vmName, bool force = false);
    bool isVMRunning(const QString& vmName);
    bool waitForVMState(const QString& vmName, const QString& state, int timeoutMs);
    QString getVMState(const QString& vmName);
    
    // Installation helpers
    bool mountISO(const QString& vmName, const QString& isoPath);
    bool unmountISO(const QString& vmName);
    bool attachDisk(const QString& vmName, const QString& diskPath);
    
    // Configuration
    void saveConfig();
    void loadConfig();
    
    // State
    SetupState m_setupState;
    QString m_lastError;
    SetupProgress m_currentProgress;
    
    // Paths
    QString m_vmStoragePath;
    QString m_downloadPath;
    QString m_isoPath;
    QString m_gappsPath;
    QString m_vboxPath;
    QString m_adbPath;
    
    // VM Template
    QString m_vmTemplateName;
    QString m_androidVersion;
    QString m_gappsVariant;
    
    // Download
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_downloadReply;
    QFile* m_downloadFile;
    bool m_downloadCancelled;
    qint64 m_totalBytesToDownload;
    
    // Configuration
    QMutex m_configMutex;
    QString m_configPath;
    
    // VirtualBox detection
    bool m_vboxInstalled;
    QString m_vboxVersion;
};

} // namespace VirtualPhonePro