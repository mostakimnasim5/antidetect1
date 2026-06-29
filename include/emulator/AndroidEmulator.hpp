#pragma once

/**
 * AndroidEmulator - Complete Android Emulator with GUI
 * 
 * A full-featured Android emulator with:
 * - Qt-based GUI (phone-like display)
 * - Android-x86 VM integration
 * - APK installation and management
 * - Built-in anti-detection system
 * - Multiple profile support
 * - Real-time device control
 * 
 * Version: 4.0.0 FULL
 */

#include "../VirtualPhonePro.hpp"
#include <memory>

#if defined(_WIN32) || defined(_WIN64)
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <QtWidgets/QStackedWidget>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QSlider>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QLineEdit>
#include <QtCore/QTimer>
#include <QtCore/QProcess>
#endif

namespace VirtualPhonePro {
namespace Emulator {

#if defined(_WIN32) || defined(_WIN64)

class PhoneDisplay;
class VMController;
class AppStore;
class AntiDetectionBridge;

enum class EmulatorState {
    STOPPED,
    STARTING,
    RUNNING,
    PAUSED,
    SUSPENDED,
    ERROR
};

struct EmulatorDevice {
    std::string id;
    std::string name;
    std::string manufacturer;
    std::string model;
    std::string androidVersion;
    int screenWidth;
    int screenHeight;
    int ramMB;
    int cpuCores;
    std::string cpuModel;
    std::string imei;
    std::string androidId;
    std::string macAddress;
    double latitude;
    double longitude;
    std::string timezone;
    std::string language;
    std::string locale;
    std::string ipAddress;
    std::string countryCode;
    std::string carrier;
    std::string serialNumber;
    std::string buildFingerprint;
    std::string bootloader;
    std::string securityPatch;
    int dpi;
    float screenSize;
};

struct InstalledApp {
    std::string packageName;
    std::string appName;
    std::string version;
    std::string versionCode;
    std::string apkPath;
    bool isSystemApp;
};

struct EmulatorConfig {
    std::string vmPath;
    std::string diskImage;
    int memoryMB;
    int cpuCores;
    bool hardwareAccelerated;
    bool soundEnabled;
    int adbPort;
};

class AndroidEmulator : public QMainWindow {
    Q_OBJECT

public:
    explicit AndroidEmulator(QWidget* parent = nullptr);
    ~AndroidEmulator();

    bool initialize();
    bool start(const EmulatorDevice& device);
    bool stop();
    bool pause();
    bool resume();
    
    EmulatorState getState() const { return m_state; }
    bool isRunning() const { return m_state == EmulatorState::RUNNING; }
    
    void setDevice(const EmulatorDevice& device);
    EmulatorDevice getCurrentDevice() const { return m_currentDevice; }
    EmulatorDevice createNewDevice(const std::string& countryCode);
    
    bool installAPK(const std::string& apkPath);
    bool uninstallApp(const std::string& packageName);
    std::vector<InstalledApp> getInstalledApps();
    bool launchApp(const std::string& packageName);
    
    void setProxy(const std::string& host, int port);
    void clearProxy();
    void setRootIP(const std::string& ip);
    
    void takeScreenshot(const std::string& path);
    void sendKeyEvent(int keyCode);
    void sendTouchEvent(int x, int y, bool pressed);

signals:
    void stateChanged(EmulatorState state);
    void bootCompleted();
    void appInstalled(const QString& packageName);
    void errorOccurred(const QString& error);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onStartClicked();
    void onStopClicked();
    void onRestartClicked();
    void onInstallAPKClicked();
    void onUninstallAppClicked();
    void onLaunchAppClicked();
    void onAppSelected(QListWidgetItem* item);
    void onCountryChanged(const QString& country);
    void onCreateDeviceClicked();
    void onProxyChanged();
    void onScreenRotated(int index);
    
private:
    void setupUI();
    void setupConnections();
    void updateStatus(const QString& status);
    void updateDeviceDisplay();
    bool setupVM();
    bool configureADB();
    void applyAntiDetection();
    EmulatorDevice generateDevice(const std::string& countryCode);
    
    // UI Components
    QWidget* m_centralWidget;
    PhoneDisplay* m_phoneDisplay;
    
    QPushButton* m_btnStart;
    QPushButton* m_btnStop;
    QPushButton* m_btnRestart;
    QPushButton* m_btnScreenshot;
    QPushButton* m_btnInstallAPK;
    QPushButton* m_btnUninstall;
    QPushButton* m_btnLaunch;
    QPushButton* m_btnCreateDevice;
    
    QLabel* m_lblStatus;
    QLabel* m_lblDeviceName;
    QLabel* m_lblAndroidVersion;
    QLabel* m_lblIP;
    QLabel* m_lblGPS;
    QLabel* m_lblDetectionStatus;
    
    QListWidget* m_lstApps;
    QComboBox* m_cmbCountry;
    QComboBox* m_cmbDevice;
    QComboBox* m_cmbRotation;
    
    QLineEdit* m_txtProxyHost;
    QSpinBox* m_spnProxyPort;
    
    QSlider* m_sliderMemory;
    QSlider* m_sliderCPU;
    QSlider* m_sliderBrightness;
    
    QCheckBox* m_chkSound;
    QCheckBox* m_chkHardwareAccel;
    
    QTabWidget* m_tabWidget;
    
    // Core components
    VMController* m_vmController;
    AppStore* m_appStore;
    AntiDetectionBridge* m_antiDetection;
    
    // State
    EmulatorState m_state;
    EmulatorDevice m_currentDevice;
    EmulatorConfig m_config;
    
    // Processes
    QProcess* m_vmProcess;
    QProcess* m_adbProcess;
    
    // Timers
    QTimer* m_bootCheckTimer;
    QTimer* m_statusTimer;
};

#endif // _WIN32

} // namespace Emulator
} // namespace VirtualPhonePro
