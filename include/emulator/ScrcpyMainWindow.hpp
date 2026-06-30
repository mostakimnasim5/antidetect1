/**
 * ScrcpyMainWindow.hpp
 * 
 * Main Window with Scrcpy Integration for VirtualPhonePro
 * Complete integration demonstrating embedded Scrcpy display with full VM control
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include "../vmcontroller/VMController.hpp"
#include "../vmcontroller/VMDisplayWidget.hpp"

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QStackedLayout>
#include <QPushButton>
#include <QListWidget>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QTabWidget>
#include <QTableWidget>
#include <QProgressBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QToolBar>
#include <QDialog>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>

namespace VirtualPhonePro {

class ScrcpyMainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit ScrcpyMainWindow(QWidget* parent = nullptr);
    ~ScrcpyMainWindow();

    bool initialize();

signals:

public slots:
    // VM Management
    void launchSelectedVM();
    void stopSelectedVM();
    void restartSelectedVM();
    
    // Profile Management
    void createNewProfile();
    void deleteSelectedProfile();
    void editSelectedProfile();
    
    // APK Installation
    void installAPK(const QString& apkPath = QString());
    void onAPKDropped(const QString& apkPath);
    
    // Settings
    void showSettingsDialog();
    void saveSettings();
    void loadSettings();

protected:
    // Drag-drop for APK
    void dragEnterEvent(QDragEnterEvent* event) override;
    void dropEvent(QDropEvent* event) override;
    
    // Menu/Toolbar creation
    void createActions();
    void createMenus();
    void createToolbars();
    void createStatusBar();
    
    // UI Components
    void setupUI();
    void setupProfileList();
    void setupDisplayArea();
    void setupControlPanel();
    void setupSettingsPage();
    
    // Profile handling
    void refreshProfileList();
    void loadProfileDetails(const QString& profileId);
    void saveCurrentProfile();
    
    // VM Display
    void updateDisplayForProfile(const QString& profileId);
    
private slots:
    // Profile List
    void onProfileSelected(int row);
    void onProfileDoubleClicked(int row, int column);
    
    // Control Panel Buttons
    void onHomePressed();
    void onBackPressed();
    void onPowerPressed();
    void onVolumeUpPressed();
    void onVolumeDownPressed();
    void onRotatePressed();
    void onScreenshotPressed();
    void onNotificationsPressed();
    
    // Menu Actions
    void onNewProfile();
    void onOpenProfile();
    void onSaveProfile();
    void onImportProfile();
    void onExportProfile();
    void onExit();
    void onAbout();
    
    // VM Controller Signals
    void onVMLaunching(const QString& instanceId);
    void onVMReady(const QString& instanceId);
    void onVMStopped(const QString& instanceId);
    void onVMError(const QString& instanceId, const QString& error);
    void onDisplayReady(const QString& instanceId, VMDisplayWidget* widget);
    void onSpoofApplied(const QString& instanceId);
    
    // Settings
    void onBrowseVBoxPath();
    void onBrowseADBPath();
    void onBrowseScrcpyPath();

private:
    // UI Components
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;
    
    // Left Panel - Profile List
    QWidget* m_profilePanel;
    QVBoxLayout* m_profilePanelLayout;
    QListWidget* m_profileList;
    QPushButton* m_btnNewProfile;
    QPushButton* m_btnEditProfile;
    QPushButton* m_btnDeleteProfile;
    QPushButton* m_btnCloneProfile;
    
    // Center - VM Display Area
    QWidget* m_displayArea;
    QVBoxLayout* m_displayLayout;
    QStackedLayout* m_displayStack;
    QLabel* m_emptyDisplayLabel;
    QMap<QString, VMDisplayWidget*> m_activeDisplays;
    
    // Right Panel - Controls
    QWidget* m_controlPanel;
    QVBoxLayout* m_controlPanelLayout;
    QTabWidget* m_controlTabs;
    
    // Profile Info Tab
    QWidget* m_profileInfoTab;
    QFormLayout* m_profileInfoLayout;
    QLineEdit* m_editProfileName;
    QLineEdit* m_editManufacturer;
    QLineEdit* m_editModel;
    QLineEdit* m_editAndroidVersion;
    QLineEdit* m_editSecurityPatch;
    QLineEdit* m_editTimezone;
    QLineEdit* m_editLocale;
    
    // Device Config Tab
    QWidget* m_deviceConfigTab;
    QFormLayout* m_deviceConfigLayout;
    QLineEdit* m_editGPSLat;
    QLineEdit* m_editGPSLon;
    QLineEdit* m_editProxyHost;
    QSpinBox* m_editProxyPort;
    QLineEdit* m_editMACAddress;
    
    // Screen Config Tab
    QWidget* m_screenConfigTab;
    QFormLayout* m_screenConfigLayout;
    QSpinBox* m_editScreenWidth;
    QSpinBox* m_editScreenHeight;
    QSpinBox* m_editScreenDPI;
    QComboBox* m_comboOrientation;
    
    // Control Buttons
    QWidget* m_hardwareButtonsTab;
    QGridLayout* m_hardwareButtonsLayout;
    QPushButton* m_btnHome;
    QPushButton* m_btnBack;
    QPushButton* m_btnPower;
    QPushButton* m_btnVolumeUp;
    QPushButton* m_btnVolumeDown;
    QPushButton* m_btnRotate;
    QPushButton* m_btnScreenshot;
    QPushButton* m_btnNotifications;
    QPushButton* m_btnInstallAPK;
    
    // Settings Tab
    QWidget* m_settingsTab;
    QFormLayout* m_settingsLayout;
    QLineEdit* m_editVBoxPath;
    QLineEdit* m_editADBPath;
    QLineEdit* m_editScrcpyPath;
    QSpinBox* m_editBootTimeout;
    QSpinBox* m_editMaxFPS;
    QCheckBox* m_chkStayAwake;
    QCheckBox* m_chkShowTouches;
    QCheckBox* m_chkDisableAudio;
    
    // Status Bar
    QLabel* m_statusLabel;
    QLabel* m_vmCountLabel;
    QProgressBar* m_progressBar;
    
    // VM Actions
    QPushButton* m_btnLaunch;
    QPushButton* m_btnStop;
    QPushButton* m_btnRestart;
    
    // State
    QString m_currentProfileId;
    QString m_currentInstanceId;
    QMap<QString, FingerprintConfig> m_profiles;
    
    // VM Controller Reference
    VMController& m_vmController;
    
    // Actions
    QAction* m_actionNewProfile;
    QAction* m_actionOpenProfile;
    QAction* m_actionSaveProfile;
    QAction* m_actionImportProfile;
    QAction* m_actionExportProfile;
    QAction* m_actionLaunchVM;
    QAction* m_actionStopVM;
    QAction* m_actionInstallAPK;
    QAction* m_actionSettings;
    QAction* m_actionExit;
    QAction* m_actionAbout;
};

} // namespace VirtualPhonePro