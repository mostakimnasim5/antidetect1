#pragma once

/**
 * MainWindow - Qt-based GUI for VirtualPhonePro
 * 
 * Provides a complete Android-like interface with:
 * - Multi-profile phone cards display
 * - App installation/management
 * - Anti-detection status monitoring
 * - Real-time device control
 * - Screenshot/video capture
 * - APK installation
 * 
 * Uses Qt6/Qt5 for cross-platform GUI.
 */

#include "AndroidEmulator.hpp"
#include <QtWidgets>
#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QListWidget>
#include <QTableWidget>
#include <QTabWidget>
#include <QStackedWidget>
#include <QProgressBar>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QFrame>
#include <QScrollArea>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QStatusBar>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QColorDialog>
#include <QFontDialog>
#include <QTimer>
#include <QDateTime>
#include <QPixmap>
#include <QIcon>
#include <QSizePolicy>
#include <QSpacerItem>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shellapi.h>
#endif

namespace VirtualPhonePro {
namespace Emulator {

// Phone card widget - displays a single Android device
class PhoneCard : public QFrame {
    Q_OBJECT

public:
    explicit PhoneCard(const EmulatorDevice& device, QWidget* parent = nullptr);
    ~PhoneCard();

    void updateDevice(const EmulatorDevice& device);
    EmulatorDevice getDevice() const { return m_device; }
    void setRunning(bool running);
    void setStatus(const QString& status);
    void updateAntiDetectionStatus(bool safetyNet, bool playIntegrity, bool tls);

signals:
    void startClicked();
    void stopClicked();
    void deleteClicked();
    void settingsClicked();
    void appsClicked();

private:
    void setupUI();
    void updateDisplay();

    EmulatorDevice m_device;
    bool m_isRunning;

    // UI Elements
    QLabel* m_lblPhoneFrame;
    QLabel* m_lblStatus;
    QLabel* m_lblDeviceName;
    QLabel* m_lblAndroidVersion;
    QLabel* m_lblCountryFlag;
    QLabel* m_lblIP;
    QLabel* m_lblGPS;
    QLabel* m_lblMemory;
    
    // Status indicators
    QLabel* m_lblSafetyNet;
    QLabel* m_lblPlayIntegrity;
    QLabel* m_lblTLSFingerprint;
    QLabel* m_lblGeoIP;
    
    // Buttons
    QPushButton* m_btnStart;
    QPushButton* m_btnStop;
    QPushButton* m_btnSettings;
    QPushButton* m_btnApps;
    QPushButton* m_btnDelete;
    
    // Display preview
    QLabel* m_lblScreenPreview;
    QStackedWidget* m_screenPreview;
};

// App list item widget
class AppListItem : public QWidget {
    Q_OBJECT

public:
    explicit AppListItem(const InstalledApp& app, QWidget* parent = nullptr);
    ~AppListItem();

    InstalledApp getApp() const { return m_app; }

signals:
    void launchClicked();
    void uninstallClicked();

private:
    InstalledApp m_app;

    QLabel* m_lblIcon;
    QLabel* m_lblName;
    QLabel* m_lblVersion;
    QPushButton* m_btnLaunch;
    QPushButton* m_btnUninstall;
};

// Profile creator dialog
class ProfileCreatorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProfileCreatorDialog(QWidget* parent = nullptr);
    ~ProfileCreatorDialog();

    EmulatorDevice getCreatedDevice() const { return m_device; }

signals:
    void deviceCreated(const EmulatorDevice& device);

private slots:
    void onCountryChanged(const QString& country);
    void onDeviceTypeChanged(const QString& type);
    void onCreateClicked();
    void onCancelClicked();

private:
    void setupUI();
    void populateCountries();
    void populateDeviceTypes();
    void updateDevicePreview();

    EmulatorDevice m_device;

    // UI Elements
    QComboBox* m_cmbCountry;
    QComboBox* m_cmbDeviceType;
    QComboBox* m_cmbManufacturer;
    QComboBox* m_cmbModel;
    
    QLineEdit* m_txtDeviceName;
    QSpinBox* m_spnMemory;
    QSpinBox* m_spnCPU;
    
    QLabel* m_lblPreview;
    QLabel* m_lblAndroidVersion;
    QLabel* m_lblLanguage;
    QLabel* m_lblTimezone;
    QLabel* m_lblIPPreview;
    QLabel* m_lblGPSPreview;
    
    QPushButton* m_btnCreate;
    QPushButton* m_btnCancel;
};

// Settings dialog
class SettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog();

signals:
    void settingsChanged();

private slots:
    void onSaveClicked();
    void onCancelClicked();
    void onBrowseVMPath();
    void onBrowseADBPath();

private:
    void setupUI();
    void loadSettings();
    void saveSettings();

    QLineEdit* m_txtVMPath;
    QLineEdit* m_txtADBPath;
    QSpinBox* m_spnDefaultMemory;
    QSpinBox* m_spnDefaultCPU;
    QSpinBox* m_spnDefaultADBPort;
    QCheckBox* m_chkAutoStart;
    QCheckBox* m_chkEnableRoot;
    QCheckBox* m_chkHardwareAccel;
    QComboBox* m_cmbTheme;
};

// Proxy settings dialog
class ProxyDialog : public QDialog {
    Q_OBJECT

public:
    explicit ProxyDialog(QWidget* parent = nullptr);
    ~ProxyDialog();

    QString getProxyHost() const;
    int getProxyPort() const;
    QString getProxyUsername() const;
    QString getProxyPassword() const;
    bool isEnabled() const;

private slots:
    void onTestClicked();
    void onSaveClicked();
    void onCancelClicked();

private:
    void setupUI();

    QCheckBox* m_chkEnable;
    QLineEdit* m_txtHost;
    QSpinBox* m_spnPort;
    QLineEdit* m_txtUsername;
    QLineEdit* m_txtPassword;
    QPushButton* m_btnTest;
    QLabel* m_lblStatus;
};

// Main application window
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    bool initialize();

public slots:
    void onNewProfile();
    void onDeleteProfile();
    void onImportProfile();
    void onExportProfile();
    void onStartAll();
    void onStopAll();
    void onSettings();
    void onProxy();
    void onScreenshot();
    void onAbout();
    
    void onPhoneStart(PhoneCard* card);
    void onPhoneStop(PhoneCard* card);
    void onPhoneDelete(PhoneCard* card);
    void onPhoneSettings(PhoneCard* card);
    void onPhoneApps(PhoneCard* card);
    
    void onInstallAPK();
    void onUninstallApp(const QString& packageName);
    void onLaunchApp(const QString& packageName);
    
    void onCountryChanged(const QString& country);

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateStatusBar();
    void checkBootStatus();
    void onPhoneSelectionChanged();

private:
    void setupUI();
    void setupMenuBar();
    void setupToolBar();
    void setupStatusBar();
    void setupConnections();
    
    void loadProfiles();
    void saveProfiles();
    void refreshPhoneList();
    void addPhoneCard(const EmulatorDevice& device);
    
    void showError(const QString& message);
    void showSuccess(const QString& message);
    
    // Central widget with phone grid
    QWidget* m_centralWidget;
    QScrollArea* m_scrollArea;
    QWidget* m_phoneContainer;
    QGridLayout* m_phoneGrid;
    
    // Side panel
    QWidget* m_sidePanel;
    QTabWidget* m_sideTab;
    
    // Apps tab
    QListWidget* m_lstApps;
    QPushButton* m_btnInstallAPK;
    QPushButton* m_btnUninstall;
    QPushButton* m_btnLaunchApp;
    QPushButton* m_btnRefreshApps;
    
    // Anti-detection status tab
    QLabel* m_lblOverallStatus;
    QLabel* m_lblSafetyNetStatus;
    QLabel* m_lblPlayIntegrityStatus;
    QLabel* m_lblTLSStatus;
    QLabel* m_lblGeoIPStatus;
    QLabel* m_lblSensorStatus;
    QProgressBar* m_progressDetection;
    
    // Settings tab
    QLabel* m_lblCurrentDevice;
    QLabel* m_lblCurrentCountry;
    QLabel* m_lblCurrentIP;
    QLabel* m_lblCurrentGPS;
    QPushButton* m_btnChangeProxy;
    QPushButton* m_btnChangeCountry;
    
    // Phone cards
    QList<PhoneCard*> m_phoneCards;
    PhoneCard* m_selectedPhone;
    
    // Dialogs
    ProfileCreatorDialog* m_profileCreator;
    SettingsDialog* m_settingsDialog;
    ProxyDialog* m_proxyDialog;
    
    // Status bar info
    QLabel* m_lblTotalPhones;
    QLabel* m_lblRunningPhones;
    QLabel* m_lblTotalApps;
    QLabel* m_lblStatusMessage;
    QLabel* m_lblTime;
    
    // Timers
    QTimer* m_statusTimer;
    QTimer* m_clockTimer;
    
    // Settings
    bool m_autoSave;
    QString m_lastProfilePath;
};

// Helper function to get country flag emoji
QString getCountryFlag(const QString& countryCode);

// Helper function to get status color
QColor getStatusColor(bool ok);

} // namespace Emulator
} // namespace VirtualPhonePro
