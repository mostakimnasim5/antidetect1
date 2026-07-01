/**
 * QEMUController.hpp
 * 
 * QEMU-based VM Controller for VirtualPhonePro
 * Replaces VirtualBox with direct QEMU control
 * 
 * Features:
 * - Direct QEMU process management
 * - QMP (QEMU Monitor Protocol) for control
 * - Built-in display via VNC/SDL
 * - Device spoofing without ADB
 * - KVM acceleration support
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QObject>
#include <QProcess>
#include <QTimer>
#include <QAbstractSocket>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMutex>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

enum class QEMUState {
    STOPPED,
    STARTING,
    RUNNING,
    PAUSED,
    STOPPING,
    ERROR
};

enum class DisplayType {
    SDL,       // SDL window (best for embedding)
    GTK,        // GTK window
    VNC,        // VNC server (remote)
    NONE        // No display
};

struct QEMUConfig {
    // VM Resources
    int memoryMB = 4096;
    int cpuCores = 4;
    int vcpuCount = 4;
    
    // Display
    DisplayType displayType = DisplayType::SDL;
    int displayWidth = 1080;
    int displayHeight = 1920;
    int displayDPI = 480;
    bool fullscreen = false;
    
    // Storage
    QString diskPath;
    int diskSizeGB = 32;
    QString cdromPath;
    
    // Network
    QString networkMode = "user";  // user, bridge, tap
    QString macAddress;
    bool enableKVM = true;
    
    // Android-specific
    QString androidVersion = "14";
    QString kernelPath;
    QString initrdPath;
    QString append;  // Kernel command line
    
    // Spoofing parameters (without ADB)
    QString deviceModel;
    QString serialNumber;
    QString imei;
    QString androidId;
    QString gsfId;
    QString manufacturer;
    QString brand;
};

struct QEMUInstance {
    QString instanceId;
    QString profileId;
    QString vmName;
    QEMUState state;
    QEMUConfig config;
    qint64 pid;
    int vncPort;
    QString vncPassword;
    QDateTime startTime;
    QString errorMessage;
};

class QEMUController : public QObject {
    Q_OBJECT

public:
    static QEMUController& getInstance();
    
    // ============================================================================
    // Lifecycle
    // ============================================================================
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    bool isQEMUInstalled();
    QString getQEMUVersion();
    
    // ============================================================================
    // VM Instance Management
    // ============================================================================
    QString launchVM(const QString& profileId, const QEMUConfig& config);
    bool stopVM(const QString& instanceId);
    bool stopVMImmediately(const QString& instanceId);
    bool pauseVM(const QString& instanceId);
    bool resumeVM(const QString& instanceId);
    
    // VM Queries
    bool isVMRunning(const QString& instanceId);
    QEMUState getVMState(const QString& instanceId);
    QEMUInstance getInstanceInfo(const QString& instanceId);
    QList<QEMUInstance> getAllInstances();
    int getRunningInstanceCount();
    
    // ============================================================================
    // QMP Control (QEMU Monitor Protocol)
    // ============================================================================
    bool sendQMPCommand(const QString& instanceId, const QString& command, QJsonObject* response = nullptr);
    bool stopQEMU(const QString& instanceId);
    bool contQEMU(const QString& instanceId);  // Continue paused VM
    bool systemReset(const QString& instanceId);
    bool systemPowerdown(const QString& instanceId);
    bool screendump(const QString& instanceId, const QString& filename);
    
    // ============================================================================
    // Display Control
    // ============================================================================
    DisplayType getDisplayType() const;
    void setDisplayType(DisplayType type);
    
    // VNC
    int getVNCPort(const QString& instanceId);
    bool connectVNC(const QString& instanceId, const QString& host, int port);
    
    // ============================================================================
    // Device Control (Direct, no ADB needed)
    // ============================================================================
    
    // Screen/Display
    bool setScreenSize(const QString& instanceId, int width, int height);
    bool rotateScreen(const QString& instanceId, bool landscape);
    
    // Input forwarding (direct QEMU input)
    bool sendKey(const QString& instanceId, int keycode);
    bool sendText(const QString& instanceId, const QString& text);
    bool sendMouseEvent(const QString& instanceId, int x, int y, bool pressed);
    
    // Power control
    bool pressPowerButton(const QString& instanceId);
    bool pressHomeButton(const QString& instanceId);
    bool pressBackButton(const QString& instanceId);
    bool pressVolumeUp(const QString& instanceId);
    bool pressVolumeDown(const QString& instanceId);
    
    // ============================================================================
    // Device Spoofing (via QEMU parameters, no ADB)
    // ============================================================================
    bool setDeviceInfo(const QString& instanceId, 
                      const QString& manufacturer,
                      const QString& model,
                      const QString& brand);
    bool setSerialNumber(const QString& instanceId, const QString& serial);
    bool setIMEI(const QString& instanceId, const QString& imei);
    bool setAndroidId(const QString& instanceId, const QString& androidId);
    
    // Hardware spoofing via kernel command line
    bool setKernelCmdLine(const QString& instanceId, const QString& cmdline);
    bool setGPSCoordinates(const QString& instanceId, double lat, double lon);
    bool setTimezone(const QString& instanceId, const QString& timezone);
    
    // Network spoofing
    bool setMACAddress(const QString& instanceId, const QString& mac);
    bool setNetworkMode(const QString& instanceId, const QString& mode);
    
    // ============================================================================
    // Android Boot Parameters
    // ============================================================================
    QString buildAndroidBootParams(const QEMUConfig& config, const FingerprintConfig& profile);
    
    // ============================================================================
    // Configuration
    // ============================================================================
    void setQEMUPath(const QString& path);
    QString getQEMUPath() const;
    void setQEMUBinary(const QString& binary);
    QString getQEMUBinary() const;
    void setKVMEnabled(bool enabled);
    bool isKVMEnabled() const;
    
    // Path configuration
    void setVMStoragePath(const QString& path);
    QString getVMStoragePath() const;
    void setKernelPath(const QString& path);
    QString getKernelPath() const;
    
    // ============================================================================
    // VM Template/Disk Management
    // ============================================================================
    bool createVMDisk(const QString& diskPath, int sizeGB);
    bool deleteVMDisk(const QString& diskPath);
    bool cloneVMDisk(const QString& source, const QString& dest);
    bool isDiskExists(const QString& diskPath);
    
    // Android image management
    bool downloadAndroidImage(const QString& version);
    bool setupAndroidImage(const QString& imagePath);
    QString getDefaultAndroidImagePath() const;
    
signals:
    void vmStarted(const QString& instanceId);
    void vmStopped(const QString& instanceId);
    void vmError(const QString& instanceId, const QString& error);
    void vmStateChanged(const QString& instanceId, QEMUState state);
    void displayConnected(const QString& instanceId, int port);
    void bootCompleted(const QString& instanceId);
    void qmpConnected(const QString& instanceId);
    void qmpDisconnected(const QString& instanceId);

private:
    explicit QEMUController(QObject* parent = nullptr);
    ~QEMUController();
    QEMUController(const QEMUController&) = delete;
    QEMUController& operator=(const QEMUController&) = delete;
    
    // Internal helpers
    QString generateInstanceId();
    QString generateSerialNumber();
    QString generateMAC();
    bool generateIMEI(QString* outIMEI);
    bool generateAndroidId(QString* outId);
    
    QString buildQEMUCommand(const QString& instanceId, const QEMUConfig& config);
    QString buildQEMUCPUArgs(const QEMUConfig& config);
    QString buildQEMUDisplayArgs(const QEMUConfig& config);
    QString buildQEMUNetworkArgs(const QEMUConfig& config);
    QString buildQEMUDeviceArgs(const QEMUConfig& config);
    QString buildQEMUAndroidArgs(const QEMUConfig& config, const FingerprintConfig& profile);
    
    // QMP helpers
    bool connectQMP(const QString& instanceId);
    bool handshakeQMP(const QString& instanceId);
    QJsonObject parseQMPResponse(const QString& response);
    
    // Process management
    void onProcessStarted(const QString& instanceId);
    void onProcessError(const QString& instanceId, QProcess::ProcessError error);
    void onProcessFinished(const QString& instanceId, int exitCode, QProcess::ExitStatus status);
    
    // Monitoring
    void monitorVM(const QString& instanceId);
    void checkVMHealth(const QString& instanceId);
    
    // State
    QMap<QString, QEMUInstance> m_instances;
    QMap<QString, QProcess*> m_processes;
    QMap<QString, QTcpSocket*> m_qmpSockets;
    QMap<QString, int> m_vncPorts;
    QMutex m_mutex;
    
    // Configuration
    QString m_qemuPath;
    QString m_qemuBinary;
    QString m_vmStoragePath;
    QString m_kernelPath;
    QString m_initrdPath;
    bool m_kvmEnabled;
    DisplayType m_defaultDisplayType;
    
    // Port allocation
    QSet<int> m_allocatedPorts;
    int m_nextVNCPort;
    QMutex m_portMutex;
    
    // QMP Server (for QEMU to connect back)
    QTcpServer* m_qmpServer;
    QMap<QString, int> m_qmpPorts;
    
    // Initialization
    bool m_initialized;
};

} // namespace VirtualPhonePro