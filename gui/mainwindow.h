#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QLabel>
#include <QTextEdit>
#include <QGroupBox>
#include <QCheckBox>
#include <QStatusBar>
#include <QToolBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QProgressBar>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QTimer>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class AntiDetectEngine;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Device Management
    void onRefreshDevices();
    void onConnectDevice();
    void onDisconnectDevice();
    void onSelectDevice(int row);
    
    // Device Spoofing
    void onSpoofManufacturer();
    void onSpoofModel();
    void onSpoofBrand();
    void onSpoofAndroidVersion();
    void onSpoofBuildFingerprint();
    void onSpoofDeviceId();
    void onSpoofAndroidId();
    void onSpoofSerialNumber();
    
    // Screen Spoofing
    void onSpoofScreenResolution();
    void onSpoofScreenDensity();
    
    // Network Spoofing
    void onSpoofMACAddress();
    void onSpoofCarrier();
    void onSpoofLocation();
    void onSpoofCountry();
    void onEnableMockLocation();
    void onSpoofUserAgent();
    void onSpoofWebRTCIP();
    
    // Sensor Spoofing
    void onSpoofAccelerometer();
    void onSpoofGyroscope();
    void onSpoofGPS();
    
    // Hardware Spoofing
    void onSpoofCPU();
    void onSpoofGPU();
    void onSpoofMemory();
    
    // System Control
    void onSetTimezone();
    void onSetLocale();
    void onSetLanguage();
    void onSetBatteryStatus();
    void onEnableSELinux();
    void onDisableSELinux();
    
    // DRM & SafetyNet
    void onSpoofWidevine();
    void onSpoofHDCP();
    void onSpoofSafetyNet();
    void onEnableIntegrity();
    
    // Profile Management
    void onLoadProfiles();
    void onApplyProfile();
    void onCreateProfile();
    void onDeleteProfile();
    void onExportProfiles();
    void onImportProfiles();
    
    // Tracker Blocking
    void onBlockTracker();
    void onUnblockTracker();
    void onLoadBlocklist();
    
    // Actions
    void onResetAll();
    void onBackupState();
    void onRestoreState();
    void onGetCurrentFingerprint();
    void onGetSystemStatus();
    
    // About
    void onAbout();
    void onHelp();

private:
    void setupUI();
    void setupDevicePanel();
    void setupFingerprintPanel();
    void setupNetworkPanel();
    void setupSystemPanel();
    void setupProfilePanel();
    void setupAdvancedPanel();
    void setupMenuBar();
    void setupStatusBar();
    
    void updateDeviceList();
    void updateProfileList();
    void updateLog(const QString &message);
    void updateStatus(const QString &status);
    
    void showError(const QString &title, const QString &message);
    void showSuccess(const QString &title, const QString &message);
    
    QString getCurrentDeviceSerial();
    bool isDeviceConnected();

private:
    Ui::MainWindow *ui;
    
    QTabWidget *m_tabWidget;
    
    // Device Panel
    QTableWidget *m_deviceTable;
    QPushButton *m_btnRefreshDevices;
    QPushButton *m_btnConnect;
    QPushButton *m_btnDisconnect;
    QLabel *m_lblDeviceStatus;
    
    // Device Spoofing
    QLineEdit *m_edtManufacturer;
    QLineEdit *m_edtModel;
    QLineEdit *m_edtBrand;
    QLineEdit *m_edtAndroidVersion;
    QLineEdit *m_edtBuildFingerprint;
    QLineEdit *m_edtDeviceId;
    QLineEdit *m_edtAndroidId;
    QLineEdit *m_edtSerialNumber;
    
    // Screen Spoofing
    QSpinBox *m_spinWidth;
    QSpinBox *m_spinHeight;
    QSpinBox *m_spinDensity;
    
    // Network Spoofing
    QLineEdit *m_edtMACAddress;
    QLineEdit *m_edtCarrier;
    QLineEdit *m_edtCountry;
    QLineEdit *m_edtLatitude;
    QLineEdit *m_edtLongitude;
    QLineEdit *m_edtUserAgent;
    QLineEdit *m_edtWebRTCIP;
    QCheckBox *m_chkMockLocation;
    
    // Hardware
    QLineEdit *m_edtCPUModel;
    QLineEdit *m_edtGPU;
    QSpinBox *m_spinMemory;
    
    // Sensors
    QDoubleSpinBox *m_spinAccelX;
    QDoubleSpinBox *m_spinAccelY;
    QDoubleSpinBox *m_spinAccelZ;
    QDoubleSpinBox *m_spinGyroX;
    QDoubleSpinBox *m_spinGyroY;
    QDoubleSpinBox *m_spinGyroZ;
    QDoubleSpinBox *m_spinGPSLat;
    QDoubleSpinBox *m_spinGPSLon;
    
    // System
    QComboBox *m_cmbTimezone;
    QLineEdit *m_edtLocale;
    QLineEdit *m_edtLanguage;
    QSpinBox *m_spinBatteryLevel;
    QComboBox *m_cmbBatteryStatus;
    
    // DRM
    QComboBox *m_cmbWidevineLevel;
    QComboBox *m_cmbHDCPLevel;
    QCheckBox *m_chkBasicIntegrity;
    QCheckBox *m_chkDeviceIntegrity;
    QCheckBox *m_chkNoCtsMismatch;
    
    // Profiles
    QListWidget *m_profileList;
    QLineEdit *m_edtProfileName;
    
    // Trackers
    QListWidget *m_trackerList;
    QLineEdit *m_edtTrackerDomain;
    
    // Log
    QTextEdit *m_logText;
    
    // Status
    QLabel *m_lblStatus;
    QProgressBar *m_progressBar;
    
    AntiDetectEngine *m_engine;
};
