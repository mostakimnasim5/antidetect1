#pragma once

/**
 * AndroidEmulatorManager - Complete Android VM Integration
 * 
 * This is the complete Android emulator system that provides:
 * 
 * 1. FULL ANDROID DISPLAY
 *    - Real-time screen capture from VM
 *    - Pixel-perfect rendering
 *    - Touch input forwarding
 *    - Keyboard input forwarding
 *    - Gesture support (swipe, pinch, etc.)
 * 
 * 2. APP MANAGEMENT
 *    - APK installation via ADB
 *    - Google Play Store integration
 *    - App launching and management
 *    - System app management
 * 
 * 3. GOOGLE PLAY SERVICES
 *    - GApps integration
 *    - Play Store support
 *    - SafetyNet/Play Integrity ready
 * 
 * 4. VM CONTROL
 *    - VirtualBox/QEMU control
 *    - Start/Stop/Pause/Resume
 *    - Snapshot management
 *    - State persistence
 * 
 * 5. FULL SCREEN DISPLAY
 *    - VNC/RDP connection
 *    - Native window rendering
 *    - Multiple device windows
 *    - Window management
 * 
 * Architecture:
 * 
 * ┌─────────────────────────────────────────────────────────────┐
 * │        VirtualPhonePro Manager (Qt GUI)                     │
 * ├─────────────────────────────────────────────────────────────┤
 * │                                                              │
 * │  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐        │
 * │  │  Phone 1   │  │  Phone 2   │  │  Phone 3   │        │
 * │  │ ┌─────────┐ │  │ ┌─────────┐ │  │ ┌─────────┐ │        │
 * │  │ │ Android │ │  │ │ Android │ │  │ │ Android │ │        │
 * │  │ │ Screen  │ │  │ │ Screen  │ │  │ │ Screen  │ │        │
 * │  │ └─────────┘ │  │ └─────────┘ │  │ └─────────┘ │        │
 * │  │ [Touch]    │  │ [Touch]    │  │ [Touch]    │        │
 * │  └─────────────┘  └─────────────┘  └─────────────┘        │
 * │         │               │               │                    │
 * └─────────┼───────────────┼───────────────┼────────────────────┘
 *           │               │               │
 * ┌────────┴───────────────┴───────────────┴────────────────────┐
 * │                    ADB Bridge                                  │
 * │  • Install APK     • Shell commands   • File transfer       │
 * └────────┬───────────────┬───────────────┬────────────────────┘
 *          │               │               │
 * ┌────────┴───────────────┴───────────────┴────────────────────┐
 * │              VirtualBox / QEMU / BlueStacks                    │
 * │  • VM Control    • Screen Capture    • Input Forwarding       │
 * └────────┬───────────────┬───────────────┬────────────────────┘
 *          │               │               │
 * ┌────────┴───────────────┴───────────────┴────────────────────┐
 * │              Android-x86 / Android VM                          │
 * │  • Full OS         • Play Store      • All Apps             │
 * └───────────────────────────────────────────────────────────────┘
 */

#include "AndroidEmulator.hpp"
#include <QtWidgets>
#include <QMainWindow>
#include <QLabel>
#include <QPixmap>
#include <QMouseEvent>
#include <QTimer>
#include <QProcess>
#include <QTcpSocket>
#include <QNetworkAccessManager>
#include <QJsonDocument>
#include <QJsonObject>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#endif

namespace VirtualPhonePro {
namespace Emulator {

// Forward declarations
class VBoxController;
class ADBBridge;
class ScreenCapture;
class InputForwarder;
class GAppsInstaller;

/**
 * Android Screen Widget - Displays the Android screen
 * Supports touch, keyboard input, and gesture recognition
 */
class AndroidScreenWidget : public QLabel {
    Q_OBJECT

public:
    explicit AndroidScreenWidget(QWidget* parent = nullptr);
    ~AndroidScreenWidget();

    // Screen management
    void setDeviceId(const QString& id) { m_deviceId = id; }
    QString getDeviceId() const { return m_deviceId; }
    
    void updateScreen(const QPixmap& screenshot);
    void clearScreen();
    
    // Touch input
    void sendTouchDown(int x, int y);
    void sendTouchUp(int x, int y);
    void sendTouchMove(int x, int y);
    
    // Gesture input
    void sendSwipe(int x1, int y1, int x2, int y2, int durationMs = 300);
    void sendLongPress(int x, int y, int durationMs = 1000);
    void sendPinch(const QPoint& center, int scale);
    
    // Keyboard input
    void sendKey(int androidKeyCode);
    void sendText(const QString& text);
    
    // Power buttons
    void powerButton();
    void volumeUp();
    void volumeDown();
    
    // Screen off/on
    void screenOff();
    void screenOn();

signals:
    void touchEvent(int action, int x, int y);

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QString m_deviceId;
    bool m_isPressed;
    QPoint m_lastPos;
    
    // Scale factor for screen resolution
    float m_scaleX;
    float m_scaleY;
    int m_screenWidth;
    int m_screenHeight;
    
    void sendADBCommand(const QString& command);
};

/**
 * VBoxController - Controls VirtualBox VM
 */
class VBoxController : public QObject {
    Q_OBJECT

public:
    explicit VBoxController(QObject* parent = nullptr);
    ~VBoxController();

    // VM Lifecycle
    bool startVM(const QString& vmName);
    bool stopVM(const QString& vmName);
    bool pauseVM(const QString& vmName);
    bool resumeVM(const QString& vmName);
    bool restartVM(const QString& vmName);
    
    // VM State
    enum class VMState { STOPPED, STARTING, RUNNING, PAUSED, ERROR };
    VMState getState(const QString& vmName);
    
    // Configuration
    bool createVM(const EmulatorDevice& device);
    bool deleteVM(const QString& vmName);
    bool cloneVM(const QString& source, const QString& dest);
    
    // Snapshot management
    bool createSnapshot(const QString& vmName, const QString& snapshotName);
    bool restoreSnapshot(const QString& vmName, const QString& snapshotName);
    QStringList listSnapshots(const QString& vmName);
    
    // Display settings
    bool setDisplayConfig(const QString& vmName, int width, int height, int vramMB);
    bool enableVRDE(const QString& vmName, int port = 3389);
    
    // Network settings
    bool setNetworkMode(const QString& vmName, const QString& mode, const QString& mac = "");
    bool setProxy(const QString& vmName, const QString& host, int port);
    
    // VM Info
    QString getVMInfo(const QString& vmName, const QString& key);
    QStringList listVMs();
    bool vmExists(const QString& vmName);

signals:
    void stateChanged(const QString& vmName, VMState state);
    void errorOccurred(const QString& error);

private slots:
    void onProcessFinished(int code, QProcess::ExitStatus status);

private:
    QProcess* m_vboxManage;
    QMap<QString, VMState> m_vmStates;
    
    QString executeCommand(const QStringList& args);
    bool waitForState(const QString& vmName, VMState expected, int timeoutMs = 60000);
};

/**
 * ADBBridge - Android Debug Bridge connection
 */
class ADBBridge : public QObject {
    Q_OBJECT

public:
    explicit ADBBridge(QObject* parent = nullptr);
    ~ADBBridge();

    // Connection
    bool connect(const QString& deviceIP, int port = 5555);
    bool disconnect();
    bool isConnected() const { return m_connected; }
    QString getDeviceIP() const { return m_deviceIP; }
    
    // Device connection
    bool connectToDevice(const QString& deviceIP);
    bool disconnectFromDevice();
    bool waitForDevice(int timeoutMs = 30000);
    
    // Shell commands
    QString executeShell(const QString& command);
    bool executeShellAsync(const QString& command);
    
    // App management
    bool installAPK(const QString& apkPath, bool grantPermissions = true);
    bool uninstallApp(const QString& packageName);
    bool launchApp(const QString& packageName);
    bool forceStopApp(const QString& packageName);
    bool clearAppData(const QString& packageName);
    
    // App info
    QString getPackageName(const QString& apkPath);
    QString getAppVersion(const QString& packageName);
    QStringList getInstalledApps(bool systemApps = false);
    bool isAppInstalled(const QString& packageName);
    
    // Screen operations
    QPixmap captureScreen();
    bool screenOn();
    bool screenOff();
    bool pressPower();
    bool pressHome();
    bool pressBack();
    bool pressRecent();
    
    // Input
    bool tap(int x, int y);
    bool swipe(int x1, int y1, int x2, int y2, int durationMs = 300);
    bool inputText(const QString& text);
    bool pressKey(int keyCode);
    
    // File operations
    bool pushFile(const QString& localPath, const QString& remotePath);
    bool pullFile(const QString& remotePath, const QString& localPath);
    
    // System
    bool reboot(const QString& mode = "");
    bool root();
    bool unroot();
    bool remount();
    QString getProp(const QString& key);
    bool setProp(const QString& key, const QString& value);

signals:
    void connected();
    void disconnected();
    void deviceConnected(const QString& deviceId);
    void deviceDisconnected(const QString& deviceId);
    void screenshotReady(const QPixmap& screenshot);

private:
    QString m_deviceIP;
    int m_port;
    bool m_connected;
    QProcess* m_adbProcess;
    QString m_adbPath;
    
    QString executeADB(const QStringList& args, int timeoutMs = 30000);
    bool startADBServer();
    QString getLocalIP();
};

/**
 * ScreenCapture - Captures Android screen via ADB
 */
class ScreenCapture : public QObject {
    Q_OBJECT

public:
    explicit ScreenCapture(QObject* parent = nullptr);
    ~ScreenCapture();

    void setADBBridge(ADBBridge* adb) { m_adb = adb; }
    
    // Capture modes
    QPixmap capture();
    bool startContinuousCapture(int intervalMs = 100);
    void stopContinuousCapture();
    
    // Recording
    bool startRecording(const QString& outputPath, int fps = 30);
    bool stopRecording();
    bool isRecording() const { return m_recording; }
    
    // Screenshot
    bool saveScreenshot(const QString& path);
    
    // Stream (for low-latency display)
    bool startStream(QLabel* displayWidget, int intervalMs = 50);
    void stopStream();

signals:
    void frameCaptured(const QPixmap& frame);
    void recordingStarted();
    void recordingStopped(const QString& outputPath);

private slots:
    void onCaptureTimer();
    void onFrameReady(const QPixmap& frame);

private:
    ADBBridge* m_adb;
    QTimer* m_captureTimer;
    QTimer* m_streamTimer;
    bool m_recording;
    QString m_recordingPath;
    QPixmap m_lastFrame;
    QLabel* m_streamTarget;
};

/**
 * InputForwarder - Sends touch/keyboard input to Android
 */
class InputForwarder : public QObject {
    Q_OBJECT

public:
    explicit InputForwarder(QObject* parent = nullptr);
    ~InputForwarder();

    void setADBBridge(ADBBridge* adb) { m_adb = adb; }
    void setDeviceId(const QString& id) { m_deviceId = id; }

    // Touch events
    void touchDown(int contact, int x, int y);
    void touchUp(int contact, int x, int y);
    void touchMove(int contact, int x, int y);
    
    // Gestures
    void tap(int x, int y);
    void doubleTap(int x, int y);
    void longPress(int x, int y, int durationMs = 1000);
    void swipe(int x1, int y1, int x2, int y2, int durationMs = 300);
    void pinch(int centerX, int centerY, int scale, int durationMs = 500);
    
    // Keyboard
    void keyPress(int keyCode);
    void keyDown(int keyCode);
    void keyUp(int keyCode);
    void textInput(const QString& text);
    
    // Navigation
    void home();
    void back();
    void recent();
    void power();
    void volumeUp();
    void volumeDown();
    
    // Device keys
    enum AndroidKey : int {
        KEY_HOME = 3,
        KEY_BACK = 4,
        KEY_RECENT = 187,
        KEY_POWER = 26,
        KEY_VOLUME_UP = 24,
        KEY_VOLUME_DOWN = 25,
        KEY_MENU = 82,
        KEY_ENTER = 66,
        KEY_DELETE = 67,
        KEY_SEARCH = 84,
        KEY_CAMERA = 27
    };

private:
    ADBBridge* m_adb;
    QString m_deviceId;
    int m_touchContact;
    
    void sendTapEvent(int x, int y, int duration);
    void sendSwipeEvent(int x1, int y1, int x2, int y2, int duration);
};

/**
 * GAppsInstaller - Google Play Services installation
 */
class GAppsInstaller : public QObject {
    Q_OBJECT

public:
    explicit GAppsInstaller(QObject* parent = nullptr);
    ~GAppsInstaller();

    void setADBBridge(ADBBridge* adb) { m_adb = adb; }

    // Check if GApps installed
    bool isInstalled();
    
    // Install OpenGApps
    bool install(const QString& gAppsZipPath);
    
    // Common GApps components
    bool installMinimal();      // Basic Google Play
    bool installFull();         // All Google apps
    bool installStock();        // Stock ROM GApps
    
    // Individual components
    bool installPlayStore();
    bool installPlayServices();
    bool installFramework();
    
    // Update Play Store
    bool updatePlayStore();
    
    // Sign in assistance
    bool addGoogleAccount(const QString& email, const QString& password);
    
    // Compatibility check
    QString checkCompatibility();

signals:
    void installProgress(int percent, const QString& message);
    void installCompleted(bool success, const QString& message);
    void errorOccurred(const QString& error);

private:
    ADBBridge* m_adb;
    
    bool pushAndExtract(const QString& zipPath);
    bool runGAppsScript(const QString& script);
    bool configureGApps();
};

/**
 * CompleteEmulatorWindow - Main window with full Android display
 */
class CompleteEmulatorWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit CompleteEmulatorWindow(const EmulatorDevice& device, QWidget* parent = nullptr);
    ~CompleteEmulatorWindow();

    bool initialize();
    bool start();
    bool stop();
    
    EmulatorDevice getDevice() const { return m_device; }
    bool isRunning() const { return m_running; }

public slots:
    void onScreenshot();
    void onInstallAPK();
    void onOpenSettings();
    void onOpenPlayStore();
    void onPowerMenu();
    void onRotateScreen();
    void onRefreshScreen();

signals:
    void appLaunchRequested(const QString& packageName);
    void screenshotTaken(const QString& path);

protected:
    void closeEvent(QCloseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private slots:
    void updateScreen();
    void onTouchEvent(int action, int x, int y);
    void onADBConnected();
    void onADBDisconnected();

private:
    void setupUI();
    void setupConnections();
    bool setupADBConnection();
    void updateStatus();

    EmulatorDevice m_device;
    bool m_running;
    bool m_screenOn;
    
    // Core components
    VBoxController* m_vbox;
    ADBBridge* m_adb;
    ScreenCapture* m_screenCapture;
    InputForwarder* m_input;
    GAppsInstaller* m_gapps;
    
    // UI Components
    AndroidScreenWidget* m_screenWidget;
    QLabel* m_lblDeviceName;
    QLabel* m_lblStatus;
    QPushButton* m_btnScreenshot;
    QPushButton* m_btnInstallAPK;
    QPushButton* m_btnPlayStore;
    QPushButton* m_btnSettings;
    QPushButton* m_btnPower;
    QPushButton* m_btnRotate;
    QPushButton* m_btnHome;
    QPushButton* m_btnBack;
    QPushButton* m_btnRecent;
    
    // Layout
    QWidget* m_centralWidget;
    QVBoxLayout* m_mainLayout;
    QHBoxLayout* m_buttonBar;
    
    // Timer for screen updates
    QTimer* m_screenTimer;
    
    // Screen state
    int m_currentRotation; // 0, 90, 180, 270
};

/**
 * MultiDeviceManager - Manages multiple emulator windows
 */
class MultiDeviceManager : public QObject {
    Q_OBJECT

public:
    explicit MultiDeviceManager(QObject* parent = nullptr);
    ~MultiDeviceManager();

    // Device management
    CompleteEmulatorWindow* createDevice(const EmulatorDevice& device);
    bool deleteDevice(const QString& deviceId);
    CompleteEmulatorWindow* getDevice(const QString& deviceId);
    QList<CompleteEmulatorWindow*> getAllDevices();
    
    // Batch operations
    void startAll();
    void stopAll();
    void pauseAll();
    
    // Settings
    void setADBPath(const QString& path);
    void setVBoxPath(const QString& path);

signals:
    void deviceCreated(const QString& deviceId);
    void deviceDeleted(const QString& deviceId);
    void allDevicesStarted();
    void allDevicesStopped();

private:
    QMap<QString, CompleteEmulatorWindow*> m_devices;
    QString m_adbPath;
    QString m_vboxPath;
};

} // namespace Emulator
} // namespace VirtualPhonePro
