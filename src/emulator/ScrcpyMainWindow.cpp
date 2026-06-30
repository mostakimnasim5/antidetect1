/**
 * ScrcpyMainWindow.cpp
 * 
 * Implementation of Scrcpy-based Main Window for VirtualPhonePro
 */

#include "emulator/ScrcpyMainWindow.hpp"
#include "vmcontroller/VMController.hpp"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QApplication>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Constructor & Destructor
// ============================================================================
ScrcpyMainWindow::ScrcpyMainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_vmController(VMController::getInstance())
    , m_currentProfileId("")
    , m_currentInstanceId("")
{
    setWindowTitle("VirtualPhonePro - Scrcpy Edition");
    setMinimumSize(1200, 800);
    resize(1400, 900);
    
    // Set window flags for frameless look option
    setWindowFlags(Qt::Window | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);
    
    setupUI();
    createActions();
    createMenus();
    createToolbars();
    createStatusBar();
    
    // Connect VM Controller signals
    connect(&m_vmController, &VMController::vmLaunching, this, &ScrcpyMainWindow::onVMLaunching);
    connect(&m_vmController, &VMController::vmReady, this, &ScrcpyMainWindow::onVMReady);
    connect(&m_vmController, &VMController::vmStopped, this, &ScrcpyMainWindow::onVMStopped);
    connect(&m_vmController, &VMController::vmError, this, &ScrcpyMainWindow::onVMError);
    connect(&m_vmController, &VMController::displayReady, this, &ScrcpyMainWindow::onDisplayReady);
    connect(&m_vmController, &VMController::spoofApplied, this, &ScrcpyMainWindow::onSpoofApplied);
    
    loadSettings();
    refreshProfileList();
}

ScrcpyMainWindow::~ScrcpyMainWindow() {
    saveSettings();
    
    // Stop all VMs on exit
    m_vmController.stopAllVMs();
}

// ============================================================================
// Initialization
// ============================================================================
bool ScrcpyMainWindow::initialize() {
    return m_vmController.initialize();
}

// ============================================================================
// UI Setup
// ============================================================================
void ScrcpyMainWindow::setupUI() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(5, 5, 5, 5);
    m_mainLayout->setSpacing(5);
    
    // Left Panel - Profile List
    setupProfileList();
    
    // Center - Display Area
    setupDisplayArea();
    
    // Right Panel - Controls
    setupControlPanel();
}

void ScrcpyMainWindow::setupProfileList() {
    m_profilePanel = new QWidget();
    m_profilePanel->setMaximumWidth(280);
    m_profilePanel->setMinimumWidth(200);
    m_profilePanel->setStyleSheet("QWidget { background: #2d2d2d; border-radius: 4px; }");
    
    m_profilePanelLayout = new QVBoxLayout(m_profilePanel);
    m_profilePanelLayout->setContentsMargins(10, 10, 10, 10);
    
    // Title
    QLabel* titleLabel = new QLabel("📱 Profiles");
    titleLabel->setStyleSheet("QLabel { font-size: 14px; font-weight: bold; color: #ffffff; padding: 5px; }");
    m_profilePanelLayout->addWidget(titleLabel);
    
    // Profile list
    m_profileList = new QListWidget();
    m_profileList->setStyleSheet(R"(
        QListWidget {
            background: #1e1e1e;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            color: #ffffff;
        }
        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #333;
        }
        QListWidget::item:selected {
            background: #0078d4;
        }
        QListWidget::item:hover {
            background: #3a3a3a;
        }
    )");
    connect(m_profileList, &QListWidget::itemClicked, this, &ScrcpyMainWindow::onProfileSelected);
    connect(m_profileList, &QListWidget::itemDoubleClicked, this, &ScrcpyMainWindow::onProfileDoubleClicked);
    m_profilePanelLayout->addWidget(m_profileList);
    
    // Profile buttons
    QHBoxLayout* btnRow1 = new QHBoxLayout();
    m_btnNewProfile = new QPushButton("➕ New");
    m_btnEditProfile = new QPushButton("✏️ Edit");
    m_btnNewProfile->setStyleSheet("QPushButton { padding: 5px; background: #0078d4; color: white; border: none; border-radius: 3px; }");
    m_btnEditProfile->setStyleSheet("QPushButton { padding: 5px; background: #444; color: white; border: none; border-radius: 3px; }");
    connect(m_btnNewProfile, &QPushButton::clicked, this, &ScrcpyMainWindow::onNewProfile);
    connect(m_btnEditProfile, &QPushButton::clicked, this, &ScrcpyMainWindow::editSelectedProfile);
    btnRow1->addWidget(m_btnNewProfile);
    btnRow1->addWidget(m_btnEditProfile);
    m_profilePanelLayout->addLayout(btnRow1);
    
    QHBoxLayout* btnRow2 = new QHBoxLayout();
    m_btnDeleteProfile = new QPushButton("🗑️ Delete");
    m_btnCloneProfile = new QPushButton("📋 Clone");
    m_btnDeleteProfile->setStyleSheet("QPushButton { padding: 5px; background: #d32f2f; color: white; border: none; border-radius: 3px; }");
    m_btnCloneProfile->setStyleSheet("QPushButton { padding: 5px; background: #444; color: white; border: none; border-radius: 3px; }");
    connect(m_btnDeleteProfile, &QPushButton::clicked, this, &ScrcpyMainWindow::deleteSelectedProfile);
    connect(m_btnCloneProfile, &QPushButton::clicked, this, &ScrcpyMainWindow::onNewProfile);
    btnRow2->addWidget(m_btnDeleteProfile);
    btnRow2->addWidget(m_btnCloneProfile);
    m_profilePanelLayout->addLayout(btnRow2);
    
    m_mainLayout->addWidget(m_profilePanel);
}

void ScrcpyMainWindow::setupDisplayArea() {
    m_displayArea = new QWidget();
    m_displayArea->setStyleSheet("QWidget { background: #1a1a1a; border: 1px solid #333; border-radius: 4px; }");
    
    m_displayLayout = new QVBoxLayout(m_displayArea);
    m_displayLayout->setContentsMargins(5, 5, 5, 5);
    
    // Stacked layout for multiple displays
    m_displayStack = new QStackedLayout();
    m_displayLayout->addLayout(m_displayStack);
    
    // Empty state label
    m_emptyDisplayLabel = new QLabel("Select a profile and click Launch\n\nOr drag & drop an APK to install");
    m_emptyDisplayLabel->setAlignment(Qt::AlignCenter);
    m_emptyDisplayLabel->setStyleSheet(R"(
        QLabel {
            color: #666;
            font-size: 16px;
            padding: 50px;
        }
    )");
    m_displayStack->addWidget(m_emptyDisplayLabel);
    
    // Enable drag-drop on display area
    m_displayArea->setAcceptDrops(true);
    
    m_mainLayout->addWidget(m_displayArea, 1); // Stretch factor 1
}

void ScrcpyMainWindow::setupControlPanel() {
    m_controlPanel = new QWidget();
    m_controlPanel->setMaximumWidth(300);
    m_controlPanel->setMinimumWidth(250);
    m_controlPanel->setStyleSheet("QWidget { background: #2d2d2d; border-radius: 4px; }");
    
    m_controlPanelLayout = new QVBoxLayout(m_controlPanel);
    m_controlPanelLayout->setContentsMargins(10, 10, 10, 10);
    
    // VM Action Buttons
    QHBoxLayout* actionRow = new QHBoxLayout();
    m_btnLaunch = new QPushButton("🚀 Launch");
    m_btnStop = new QPushButton("⏹️ Stop");
    m_btnRestart = new QPushButton("🔄 Restart");
    m_btnLaunch->setStyleSheet("QPushButton { padding: 10px; background: #4caf50; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    m_btnStop->setStyleSheet("QPushButton { padding: 10px; background: #f44336; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    m_btnRestart->setStyleSheet("QPushButton { padding: 10px; background: #ff9800; color: white; border: none; border-radius: 4px; font-weight: bold; }");
    m_btnStop->setEnabled(false);
    
    connect(m_btnLaunch, &QPushButton::clicked, this, &ScrcpyMainWindow::launchSelectedVM);
    connect(m_btnStop, &QPushButton::clicked, this, &ScrcpyMainWindow::stopSelectedVM);
    connect(m_btnRestart, &QPushButton::clicked, this, &ScrcpyMainWindow::restartSelectedVM);
    
    actionRow->addWidget(m_btnLaunch);
    actionRow->addWidget(m_btnStop);
    actionRow->addWidget(m_btnRestart);
    m_controlPanelLayout->addLayout(actionRow);
    
    // Control Tabs
    m_controlTabs = new QTabWidget();
    m_controlTabs->setStyleSheet(R"(
        QTabWidget {
            background: transparent;
        }
        QTabWidget::pane {
            background: #1e1e1e;
            border: 1px solid #3a3a3a;
            border-radius: 4px;
            padding: 10px;
        }
        QTabBar::tab {
            background: #2d2d2d;
            color: #ccc;
            padding: 8px 12px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background: #0078d4;
            color: white;
        }
    )");
    
    // Profile Info Tab
    setupProfileInfoTab();
    
    // Hardware Buttons Tab
    setupHardwareButtonsTab();
    
    // Settings Tab
    setupSettingsPage();
    
    m_controlTabs->addTab(m_profileInfoTab, "📋 Profile");
    m_controlTabs->addTab(m_hardwareButtonsTab, "🔘 Controls");
    m_controlTabs->addTab(m_settingsTab, "⚙️ Settings");
    
    m_controlPanelLayout->addWidget(m_controlTabs, 1);
    
    m_mainLayout->addWidget(m_controlPanel);
}

void ScrcpyMainWindow::setupProfileInfoTab() {
    m_profileInfoTab = new QWidget();
    m_profileInfoLayout = new QFormLayout(m_profileInfoTab);
    m_profileInfoLayout->setLabelAlignment(Qt::AlignLeft);
    m_profileInfoLayout->setSpacing(8);
    
    QString labelStyle = "QLabel { color: #999; font-size: 11px; }";
    QString editStyle = "QLineEdit { background: #333; color: white; border: 1px solid #444; border-radius: 3px; padding: 5px; }";
    
    m_editProfileName = new QLineEdit();
    m_editProfileName->setPlaceholderText("Profile Name");
    m_editProfileName->setStyleSheet(editStyle);
    
    m_editManufacturer = new QLineEdit();
    m_editManufacturer->setPlaceholderText("e.g., Samsung");
    m_editManufacturer->setStyleSheet(editStyle);
    
    m_editModel = new QLineEdit();
    m_editModel->setPlaceholderText("e.g., SM-G998B");
    m_editModel->setStyleSheet(editStyle);
    
    m_editAndroidVersion = new QLineEdit();
    m_editAndroidVersion->setPlaceholderText("e.g., 14");
    m_editAndroidVersion->setStyleSheet(editStyle);
    
    m_editSecurityPatch = new QLineEdit();
    m_editSecurityPatch->setPlaceholderText("e.g., 2024-06-01");
    m_editSecurityPatch->setStyleSheet(editStyle);
    
    m_editTimezone = new QLineEdit();
    m_editTimezone->setPlaceholderText("e.g., Asia/Dhaka");
    m_editTimezone->setStyleSheet(editStyle);
    
    m_editLocale = new QLineEdit();
    m_editLocale->setPlaceholderText("e.g., en_US");
    m_editLocale->setStyleSheet(editStyle);
    
    m_profileInfoLayout->addRow(new QLabel("Name:"), m_editProfileName);
    m_profileInfoLayout->addRow(new QLabel("Manufacturer:"), m_editManufacturer);
    m_profileInfoLayout->addRow(new QLabel("Model:"), m_editModel);
    m_profileInfoLayout->addRow(new QLabel("Android:"), m_editAndroidVersion);
    m_profileInfoLayout->addRow(new QLabel("Security Patch:"), m_editSecurityPatch);
    m_profileInfoLayout->addRow(new QLabel("Timezone:"), m_editTimezone);
    m_profileInfoLayout->addRow(new QLabel("Locale:"), m_editLocale);
    
    // Apply button
    QPushButton* btnApply = new QPushButton("💾 Save Profile");
    btnApply->setStyleSheet("QPushButton { padding: 10px; background: #0078d4; color: white; border: none; border-radius: 4px; }");
    connect(btnApply, &QPushButton::clicked, this, &ScrcpyMainWindow::saveCurrentProfile);
    m_profileInfoLayout->addRow(btnApply);
}

void ScrcpyMainWindow::setupHardwareButtonsTab() {
    m_hardwareButtonsTab = new QWidget();
    m_hardwareButtonsLayout = new QGridLayout(m_hardwareButtonsTab);
    m_hardwareButtonsLayout->setSpacing(8);
    m_hardwareButtonsLayout->setContentsMargins(10, 10, 10, 10);
    
    QString btnStyle = "QPushButton { min-height: 50px; font-size: 18px; background: #333; color: white; border: none; border-radius: 8px; } QPushButton:hover { background: #444; } QPushButton:pressed { background: #0078d4; }";
    
    m_btnHome = new QPushButton("🏠\nHome");
    m_btnBack = new QPushButton("◀\nBack");
    m_btnPower = new QPushButton("⏻\nPower");
    m_btnVolumeUp = new QPushButton("🔊+\nVol Up");
    m_btnVolumeDown = new QPushButton("🔊-\nVol Down");
    m_btnRotate = new QPushButton("↻\nRotate");
    m_btnScreenshot = new QPushButton("📷\nScreenshot");
    m_btnNotifications = new QPushButton("📬\nNotifications");
    m_btnInstallAPK = new QPushButton("📦\nInstall APK");
    
    m_btnHome->setStyleSheet(btnStyle);
    m_btnBack->setStyleSheet(btnStyle);
    m_btnPower->setStyleSheet(btnStyle);
    m_btnVolumeUp->setStyleSheet(btnStyle);
    m_btnVolumeDown->setStyleSheet(btnStyle);
    m_btnRotate->setStyleSheet(btnStyle);
    m_btnScreenshot->setStyleSheet(btnStyle);
    m_btnNotifications->setStyleSheet(btnStyle);
    m_btnInstallAPK->setStyleSheet("QPushButton { min-height: 50px; font-size: 16px; background: #4caf50; color: white; border: none; border-radius: 8px; }");
    
    connect(m_btnHome, &QPushButton::clicked, this, &ScrcpyMainWindow::onHomePressed);
    connect(m_btnBack, &QPushButton::clicked, this, &ScrcpyMainWindow::onBackPressed);
    connect(m_btnPower, &QPushButton::clicked, this, &ScrcpyMainWindow::onPowerPressed);
    connect(m_btnVolumeUp, &QPushButton::clicked, this, &ScrcpyMainWindow::onVolumeUpPressed);
    connect(m_btnVolumeDown, &QPushButton::clicked, this, &ScrcpyMainWindow::onVolumeDownPressed);
    connect(m_btnRotate, &QPushButton::clicked, this, &ScrcpyMainWindow::onRotatePressed);
    connect(m_btnScreenshot, &QPushButton::clicked, this, &ScrcpyMainWindow::onScreenshotPressed);
    connect(m_btnNotifications, &QPushButton::clicked, this, &ScrcpyMainWindow::onNotificationsPressed);
    connect(m_btnInstallAPK, &QPushButton::clicked, this, &ScrcpyMainWindow::installAPK);
    
    // Row 1
    m_hardwareButtonsLayout->addWidget(m_btnHome, 0, 0);
    m_hardwareButtonsLayout->addWidget(m_btnBack, 0, 1);
    m_hardwareButtonsLayout->addWidget(m_btnPower, 0, 2);
    
    // Row 2
    m_hardwareButtonsLayout->addWidget(m_btnVolumeUp, 1, 0);
    m_hardwareButtonsLayout->addWidget(m_btnRotate, 1, 1);
    m_hardwareButtonsLayout->addWidget(m_btnVolumeDown, 1, 2);
    
    // Row 3
    m_hardwareButtonsLayout->addWidget(m_btnNotifications, 2, 0);
    m_hardwareButtonsLayout->addWidget(m_btnScreenshot, 2, 1);
    m_hardwareButtonsLayout->addWidget(m_btnInstallAPK, 2, 2);
}

void ScrcpyMainWindow::setupSettingsPage() {
    m_settingsTab = new QWidget();
    m_settingsLayout = new QFormLayout(m_settingsTab);
    m_settingsLayout->setSpacing(8);
    
    QString editStyle = "QLineEdit { background: #333; color: white; border: 1px solid #444; border-radius: 3px; padding: 5px; }";
    QString spinStyle = "QSpinBox { background: #333; color: white; border: 1px solid #444; border-radius: 3px; padding: 5px; }";
    QString chkStyle = "QCheckBox { color: white; spacing: 8px; } QCheckBox::indicator { width: 18px; height: 18px; }";
    
    // Paths
    QLabel* pathsLabel = new QLabel("<b>Executable Paths</b>");
    pathsLabel->setStyleSheet("QLabel { color: #0078d4; padding: 5px 0; }");
    m_settingsLayout->addRow(pathsLabel);
    
    m_editVBoxPath = new QLineEdit("VBoxManage.exe");
    m_editVBoxPath->setStyleSheet(editStyle);
    m_settingsLayout->addRow("VBoxManage:", m_editVBoxPath);
    
    m_editADBPath = new QLineEdit("adb.exe");
    m_editADBPath->setStyleSheet(editStyle);
    m_settingsLayout->addRow("ADB:", m_editADBPath);
    
    m_editScrcpyPath = new QLineEdit("scrcpy.exe");
    m_editScrcpyPath->setStyleSheet(editStyle);
    m_settingsLayout->addRow("Scrcpy:", m_editScrcpyPath);
    
    // Performance
    QLabel* perfLabel = new QLabel("<b>Performance</b>");
    perfLabel->setStyleSheet("QLabel { color: #0078d4; padding: 5px 0; }");
    m_settingsLayout->addRow(perfLabel);
    
    m_editBootTimeout = new QSpinBox();
    m_editBootTimeout->setRange(10000, 300000);
    m_editBootTimeout->setSuffix(" ms");
    m_editBootTimeout->setValue(60000);
    m_editBootTimeout->setStyleSheet(spinStyle);
    m_settingsLayout->addRow("Boot Timeout:", m_editBootTimeout);
    
    m_editMaxFPS = new QSpinBox();
    m_editMaxFPS->setRange(30, 120);
    m_editMaxFPS->setValue(60);
    m_editMaxFPS->setStyleSheet(spinStyle);
    m_settingsLayout->addRow("Max FPS:", m_editMaxFPS);
    
    // Scrcpy Options
    QLabel* optionsLabel = new QLabel("<b>Scrcpy Options</b>");
    optionsLabel->setStyleSheet("QLabel { color: #0078d4; padding: 5px 0; }");
    m_settingsLayout->addRow(optionsLabel);
    
    m_chkStayAwake = new QCheckBox("Stay Awake");
    m_chkStayAwake->setChecked(true);
    m_chkStayAwake->setStyleSheet(chkStyle);
    m_settingsLayout->addRow(m_chkStayAwake);
    
    m_chkShowTouches = new QCheckBox("Show Touches");
    m_chkShowTouches->setChecked(false);
    m_chkShowTouches->setStyleSheet(chkStyle);
    m_settingsLayout->addRow(m_chkShowTouches);
    
    m_chkDisableAudio = new QCheckBox("Disable Audio");
    m_chkDisableAudio->setChecked(true);
    m_chkDisableAudio->setStyleSheet(chkStyle);
    m_settingsLayout->addRow(m_chkDisableAudio);
    
    // Save button
    QPushButton* btnSaveSettings = new QPushButton("💾 Save Settings");
    btnSaveSettings->setStyleSheet("QPushButton { padding: 10px; background: #0078d4; color: white; border: none; border-radius: 4px; }");
    connect(btnSaveSettings, &QPushButton::clicked, this, &ScrcpyMainWindow::saveSettings);
    m_settingsLayout->addRow(btnSaveSettings);
}

// ============================================================================
// Actions, Menus, Toolbars
// ============================================================================
void ScrcpyMainWindow::createActions() {
    m_actionNewProfile = new QAction("New Profile", this);
    m_actionNewProfile->setShortcut(QKeySequence::New);
    connect(m_actionNewProfile, &QAction::triggered, this, &ScrcpyMainWindow::onNewProfile);
    
    m_actionOpenProfile = new QAction("Open Profile...", this);
    m_actionOpenProfile->setShortcut(QKeySequence::Open);
    
    m_actionSaveProfile = new QAction("Save Profile", this);
    m_actionSaveProfile->setShortcut(QKeySequence::Save);
    connect(m_actionSaveProfile, &QAction::triggered, this, &ScrcpyMainWindow::saveCurrentProfile);
    
    m_actionImportProfile = new QAction("Import Profile...", this);
    connect(m_actionImportProfile, &QAction::triggered, this, &ScrcpyMainWindow::onImportProfile);
    
    m_actionExportProfile = new QAction("Export Profile...", this);
    connect(m_actionExportProfile, &QAction::triggered, this, &ScrcpyMainWindow::onExportProfile);
    
    m_actionLaunchVM = new QAction("Launch VM", this);
    m_actionLaunchVM->setShortcut(Qt::Key_F5);
    connect(m_actionLaunchVM, &QAction::triggered, this, &ScrcpyMainWindow::launchSelectedVM);
    
    m_actionStopVM = new QAction("Stop VM", this);
    m_actionStopVM->setShortcut(Qt::Key_F6);
    connect(m_actionStopVM, &QAction::triggered, this, &ScrcpyMainWindow::stopSelectedVM);
    
    m_actionInstallAPK = new QAction("Install APK...", this);
    m_actionInstallAPK->setShortcut(Qt::Ctrl + Qt::Key_I);
    connect(m_actionInstallAPK, &QAction::triggered, this, &ScrcpyMainWindow::installAPK);
    
    m_actionSettings = new QAction("Settings...", this);
    m_actionSettings->setShortcut(Qt::Ctrl + Qt::Key_P);
    connect(m_actionSettings, &QAction::triggered, this, &ScrcpyMainWindow::showSettingsDialog);
    
    m_actionExit = new QAction("Exit", this);
    m_actionExit->setShortcut(Qt::Ctrl + Qt::Key_Q);
    connect(m_actionExit, &QAction::triggered, this, &QMainWindow::close);
    
    m_actionAbout = new QAction("About VirtualPhonePro", this);
    connect(m_actionAbout, &QAction::triggered, this, &ScrcpyMainWindow::onAbout);
}

void ScrcpyMainWindow::createMenus() {
    QMenuBar* menuBar = this->menuBar();
    menuBar->setStyleSheet("QMenuBar { background: #2d2d2d; color: white; } QMenuBar::item:selected { background: #0078d4; }");
    
    // File Menu
    QMenu* fileMenu = menuBar->addMenu("File");
    fileMenu->setStyleSheet("QMenu { background: #2d2d2d; color: white; } QMenu::item:selected { background: #0078d4; }");
    fileMenu->addAction(m_actionNewProfile);
    fileMenu->addAction(m_actionOpenProfile);
    fileMenu->addAction(m_actionSaveProfile);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actionImportProfile);
    fileMenu->addAction(m_actionExportProfile);
    fileMenu->addSeparator();
    fileMenu->addAction(m_actionExit);
    
    // VM Menu
    QMenu* vmMenu = menuBar->addMenu("VM");
    vmMenu->setStyleSheet("QMenu { background: #2d2d2d; color: white; } QMenu::item:selected { background: #0078d4; }");
    vmMenu->addAction(m_actionLaunchVM);
    vmMenu->addAction(m_actionStopVM);
    vmMenu->addSeparator();
    vmMenu->addAction(m_actionInstallAPK);
    
    // Settings Menu
    QMenu* settingsMenu = menuBar->addMenu("Settings");
    settingsMenu->setStyleSheet("QMenu { background: #2d2d2d; color: white; } QMenu::item:selected { background: #0078d4; }");
    settingsMenu->addAction(m_actionSettings);
    
    // Help Menu
    QMenu* helpMenu = menuBar->addMenu("Help");
    helpMenu->setStyleSheet("QMenu { background: #2d2d2d; color: white; } QMenu::item:selected { background: #0078d4; }");
    helpMenu->addAction(m_actionAbout);
}

void ScrcpyMainWindow::createToolbars() {
    QToolBar* toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false);
    toolbar->setStyleSheet(R"(
        QToolBar {
            background: #2d2d2d;
            border: none;
            padding: 5px;
            spacing: 10px;
        }
        QToolButton {
            background: #333;
            color: white;
            border: none;
            border-radius: 4px;
            padding: 8px 16px;
        }
        QToolButton:hover {
            background: #444;
        }
        QToolButton:pressed {
            background: #0078d4;
        }
    )");
    
    toolbar->addAction(m_actionNewProfile);
    toolbar->addSeparator();
    toolbar->addAction(m_actionLaunchVM);
    toolbar->addAction(m_actionStopVM);
    toolbar->addSeparator();
    toolbar->addAction(m_actionInstallAPK);
    toolbar->addSeparator();
    toolbar->addAction(m_actionSettings);
}

void ScrcpyMainWindow::createStatusBar() {
    QStatusBar* statusBar = this->statusBar();
    statusBar->setStyleSheet("QStatusBar { background: #1e1e1e; color: #ccc; }");
    
    m_statusLabel = new QLabel("Ready");
    m_vmCountLabel = new QLabel("VMs: 0/0");
    m_progressBar = new QProgressBar();
    m_progressBar->setMaximumWidth(200);
    m_progressBar->setVisible(false);
    
    statusBar->addWidget(m_statusLabel);
    statusBar->addPermanentWidget(m_vmCountLabel);
    statusBar->addPermanentWidget(m_progressBar);
}

// ============================================================================
// Drag & Drop
// ============================================================================
void ScrcpyMainWindow::dragEnterEvent(QDragEnterEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QList<QUrl> urls = event->mimeData()->urls();
        if (!urls.isEmpty() && urls.first().toLocalFile().endsWith(".apk", Qt::CaseInsensitive)) {
            event->acceptProposedAction();
            m_statusLabel->setText("Drop APK to install...");
            return;
        }
    }
    QMainWindow::dragEnterEvent(event);
}

void ScrcpyMainWindow::dropEvent(QDropEvent* event) {
    if (event->mimeData()->hasUrls()) {
        QString filePath = event->mimeData()->urls().first().toLocalFile();
        if (filePath.endsWith(".apk", Qt::CaseInsensitive)) {
            onAPKDropped(filePath);
            event->acceptProposedAction();
            return;
        }
    }
    QMainWindow::dropEvent(event);
}

// ============================================================================
// Profile Management
// ============================================================================
void ScrcpyMainWindow::refreshProfileList() {
    m_profileList->clear();
    
    for (auto it = m_profiles.begin(); it != m_profiles.end(); ++it) {
        const FingerprintConfig& profile = it.value();
        QString displayText = QString("%1 %2\nAndroid %3 • %4")
            .arg(profile.manufacturer)
            .arg(profile.model)
            .arg(profile.androidVersion)
            .arg(profile.locale);
        
        QListWidgetItem* item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, it.key());
        m_profileList->addItem(item);
    }
}

void ScrcpyMainWindow::loadProfileDetails(const QString& profileId) {
    if (!m_profiles.contains(profileId)) {
        return;
    }
    
    const FingerprintConfig& profile = m_profiles[profileId];
    
    m_editProfileName->setText(profileId);
    m_editManufacturer->setText(profile.manufacturer);
    m_editModel->setText(profile.model);
    m_editAndroidVersion->setText(profile.androidVersion);
    m_editSecurityPatch->setText(profile.securityPatch);
    m_editTimezone->setText(profile.timezone);
    m_editLocale->setText(profile.locale);
}

void ScrcpyMainWindow::saveCurrentProfile() {
    if (m_currentProfileId.isEmpty()) {
        onNewProfile();
        return;
    }
    
    FingerprintConfig& profile = m_profiles[m_currentProfileId];
    
    profile.manufacturer = m_editManufacturer->text();
    profile.model = m_editModel->text();
    profile.androidVersion = m_editAndroidVersion->text();
    profile.securityPatch = m_editSecurityPatch->text();
    profile.timezone = m_editTimezone->text();
    profile.locale = m_editLocale->text();
    
    m_statusLabel->setText("Profile saved");
    refreshProfileList();
}

void ScrcpyMainWindow::onProfileSelected(int row) {
    if (row < 0) return;
    
    QListWidgetItem* item = m_profileList->item(row);
    m_currentProfileId = item->data(Qt::UserRole).toString();
    loadProfileDetails(m_currentProfileId);
}

void ScrcpyMainWindow::onProfileDoubleClicked(int row, int column) {
    Q_UNUSED(column);
    onProfileSelected(row);
    launchSelectedVM();
}

void ScrcpyMainWindow::onNewProfile() {
    bool ok;
    QString name = QInputDialog::getText(this, "New Profile", "Enter profile name:", 
                                        QLineEdit::Normal, "New Profile", &ok);
    if (!ok || name.isEmpty()) return;
    
    // Generate random profile
    FingerprintConfig profile = m_vmController.generateRandomProfile();
    profile.manufacturer = "Samsung";
    profile.model = "SM-G998B";
    profile.androidVersion = "14";
    profile.securityPatch = "2024-06-01";
    profile.timezone = "Asia/Dhaka";
    profile.locale = "en_US";
    
    m_profiles[name] = profile;
    m_currentProfileId = name;
    
    refreshProfileList();
    loadProfileDetails(name);
    saveCurrentProfile();
}

void ScrcpyMainWindow::deleteSelectedProfile() {
    if (m_currentProfileId.isEmpty()) return;
    
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Delete Profile",
        QString("Are you sure you want to delete profile '%1'?").arg(m_currentProfileId),
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_profiles.remove(m_currentProfileId);
        m_currentProfileId = "";
        refreshProfileList();
    }
}

void ScrcpyMainWindow::editSelectedProfile() {
    // Edit is handled inline in the profile info tab
    m_controlTabs->setCurrentIndex(0);
}

void ScrcpyMainWindow::onImportProfile() {
    QString fileName = QFileDialog::getOpenFileName(this, "Import Profile", "", 
                                                   "JSON Files (*.json);;All Files (*)");
    if (fileName.isEmpty()) return;
    
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "Error", "Could not open file");
        return;
    }
    
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    QJsonObject obj = doc.object();
    
    FingerprintConfig profile;
    profile.manufacturer = obj["manufacturer"].toString();
    profile.model = obj["model"].toString();
    profile.androidVersion = obj["androidVersion"].toString();
    profile.securityPatch = obj["securityPatch"].toString();
    profile.timezone = obj["timezone"].toString();
    profile.locale = obj["locale"].toString();
    profile.latitude = obj["latitude"].toDouble();
    profile.longitude = obj["longitude"].toDouble();
    
    QString name = obj["name"].toString("Imported Profile");
    m_profiles[name] = profile;
    refreshProfileList();
    
    m_statusLabel->setText("Profile imported");
}

void ScrcpyMainWindow::onExportProfile() {
    if (m_currentProfileId.isEmpty()) {
        QMessageBox::warning(this, "No Profile", "Please select a profile to export");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this, "Export Profile", 
                                                   m_currentProfileId + ".json",
                                                   "JSON Files (*.json)");
    if (fileName.isEmpty()) return;
    
    QJsonObject obj;
    obj["name"] = m_currentProfileId;
    
    const FingerprintConfig& profile = m_profiles[m_currentProfileId];
    obj["manufacturer"] = profile.manufacturer;
    obj["model"] = profile.model;
    obj["androidVersion"] = profile.androidVersion;
    obj["securityPatch"] = profile.securityPatch;
    obj["timezone"] = profile.timezone;
    obj["locale"] = profile.locale;
    obj["latitude"] = profile.latitude;
    obj["longitude"] = profile.longitude;
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(QJsonDocument(obj).toJson());
        m_statusLabel->setText("Profile exported");
    }
}

// ============================================================================
// VM Management
// ============================================================================
void ScrcpyMainWindow::launchSelectedVM() {
    if (m_currentProfileId.isEmpty()) {
        QMessageBox::warning(this, "No Profile", "Please select a profile first");
        return;
    }
    
    if (!m_profiles.contains(m_currentProfileId)) {
        QMessageBox::warning(this, "Invalid Profile", "Profile not found");
        return;
    }
    
    m_statusLabel->setText("Launching VM...");
    m_progressBar->setVisible(true);
    m_progressBar->setRange(0, 0); // Indeterminate
    
    const FingerprintConfig& profile = m_profiles[m_currentProfileId];
    m_currentInstanceId = m_vmController.launchVM(profile);
    
    if (m_currentInstanceId.isEmpty()) {
        QMessageBox::critical(this, "Launch Error", "Failed to launch VM");
        m_statusLabel->setText("Launch failed");
        m_progressBar->setVisible(false);
    }
}

void ScrcpyMainWindow::stopSelectedVM() {
    if (m_currentInstanceId.isEmpty()) {
        return;
    }
    
    m_vmController.stopVM(m_currentInstanceId);
    m_statusLabel->setText("VM stopped");
    m_progressBar->setVisible(false);
}

void ScrcpyMainWindow::restartSelectedVM() {
    if (m_currentInstanceId.isEmpty()) {
        return;
    }
    
    QString oldInstanceId = m_currentInstanceId;
    m_vmController.restartVM(oldInstanceId);
    m_statusLabel->setText("VM restarting...");
}

void ScrcpyMainWindow::installAPK(const QString& apkPath) {
    QString filePath = apkPath;
    
    if (filePath.isEmpty()) {
        filePath = QFileDialog::getOpenFileName(this, "Select APK", "", 
                                                "APK Files (*.apk);;All Files (*)");
        if (filePath.isEmpty()) return;
    }
    
    if (m_currentInstanceId.isEmpty()) {
        QMessageBox::warning(this, "No VM Running", "Please launch a VM first");
        return;
    }
    
    m_statusLabel->setText("Installing APK...");
    m_vmController.installAPK(m_currentInstanceId, filePath);
    m_statusLabel->setText("APK installation started");
}

void ScrcpyMainWindow::onAPKDropped(const QString& apkPath) {
    if (m_currentInstanceId.isEmpty()) {
        QMessageBox::information(this, "No VM Running", 
            "Please launch a VM first, then drag APK again");
        return;
    }
    
    installAPK(apkPath);
}

// ============================================================================
// Control Panel Handlers
// ============================================================================
void ScrcpyMainWindow::onHomePressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.pressHome(m_currentInstanceId);
    }
}

void ScrcpyMainWindow::onBackPressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.pressBack(m_currentInstanceId);
    }
}

void ScrcpyMainWindow::onPowerPressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.pressPower(m_currentInstanceId);
    }
}

void ScrcpyMainWindow::onVolumeUpPressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.volumeUp(m_currentInstanceId);
    }
}

void ScrcpyMainWindow::onVolumeDownPressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.volumeDown(m_currentInstanceId);
    }
}

void ScrcpyMainWindow::onRotatePressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.rotateScreen(m_currentInstanceId);
    }
}

void ScrcpyMainWindow::onScreenshotPressed() {
    // Take screenshot via ADB
    if (!m_currentInstanceId.isEmpty()) {
        VMInstance inst = m_vmController.getInstance(m_currentInstanceId);
        QString device = QString("127.0.0.1:%1").arg(inst.adbPort);
        
        QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
        QString filename = QString("%1/screenshot_%2.png")
            .arg(path)
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
        
        QProcess proc;
        proc.start("adb", { "-s", device, "exec-out", "screencap", "-p" });
        proc.waitForFinished();
        
        QFile file(filename);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(proc.readAll());
            m_statusLabel->setText(QString("Screenshot saved: %1").arg(filename));
        }
    }
}

void ScrcpyMainWindow::onNotificationsPressed() {
    if (!m_currentInstanceId.isEmpty()) {
        m_vmController.pullNotifications(m_currentInstanceId);
    }
}

// ============================================================================
// VM Controller Signal Handlers
// ============================================================================
void ScrcpyMainWindow::onVMLaunching(const QString& instanceId) {
    m_statusLabel->setText(QString("Launching VM %1...").arg(instanceId));
    m_btnLaunch->setEnabled(false);
    m_btnStop->setEnabled(true);
}

void ScrcpyMainWindow::onVMReady(const QString& instanceId) {
    m_statusLabel->setText(QString("VM Ready: %1").arg(instanceId));
    m_btnLaunch->setEnabled(false);
    m_btnStop->setEnabled(true);
    m_progressBar->setVisible(false);
    
    // Create and attach display widget
    VMDisplayWidget* widget = m_vmController.createDisplayWidget(instanceId, this);
    if (widget) {
        m_displayStack->addWidget(widget);
        m_displayStack->setCurrentWidget(widget);
        m_activeDisplays[instanceId] = widget;
        
        // Auto-attach display
        m_vmController.attachDisplay(instanceId, widget);
    }
}

void ScrcpyMainWindow::onVMStopped(const QString& instanceId) {
    m_statusLabel->setText(QString("VM Stopped: %1").arg(instanceId));
    m_btnLaunch->setEnabled(true);
    m_btnStop->setEnabled(false);
    m_progressBar->setVisible(false);
    
    // Remove display widget
    if (m_activeDisplays.contains(instanceId)) {
        VMDisplayWidget* widget = m_activeDisplays.take(instanceId);
        if (widget) {
            m_displayStack->removeWidget(widget);
            widget->deleteLater();
        }
    }
    
    // Show empty state
    m_displayStack->setCurrentWidget(m_emptyDisplayLabel);
}

void ScrcpyMainWindow::onVMError(const QString& instanceId, const QString& error) {
    m_statusLabel->setText(QString("Error: %1").arg(error));
    m_progressBar->setVisible(false);
    QMessageBox::critical(this, "VM Error", QString("Instance %1: %2").arg(instanceId).arg(error));
}

void ScrcpyMainWindow::onDisplayReady(const QString& instanceId, VMDisplayWidget* widget) {
    Q_UNUSED(instanceId);
    if (widget) {
        m_displayStack->setCurrentWidget(widget);
    }
}

void ScrcpyMainWindow::onSpoofApplied(const QString& instanceId) {
    m_statusLabel->setText(QString("Spoofing applied: %1").arg(instanceId));
}

// ============================================================================
// Settings
// ============================================================================
void ScrcpyMainWindow::showSettingsDialog() {
    m_controlTabs->setCurrentIndex(2); // Settings tab
}

void ScrcpyMainWindow::saveSettings() {
    // Save paths
    m_vmController.setVBoxPath(m_editVBoxPath->text());
    m_vmController.setADBPath(m_editADBPath->text());
    m_vmController.setScrcpyPath(m_editScrcpyPath->text());
    m_vmController.setBootTimeout(m_editBootTimeout->value());
    
    // Save to file
    QSettings settings("VirtualPhonePro", "ScrcpyMainWindow");
    settings.setValue("vboxPath", m_editVBoxPath->text());
    settings.setValue("adbPath", m_editADBPath->text());
    settings.setValue("scrcpyPath", m_editScrcpyPath->text());
    settings.setValue("bootTimeout", m_editBootTimeout->value());
    settings.setValue("maxFPS", m_editMaxFPS->value());
    settings.setValue("stayAwake", m_chkStayAwake->isChecked());
    settings.setValue("showTouches", m_chkShowTouches->isChecked());
    settings.setValue("disableAudio", m_chkDisableAudio->isChecked());
    
    m_statusLabel->setText("Settings saved");
}

void ScrcpyMainWindow::loadSettings() {
    QSettings settings("VirtualPhonePro", "ScrcpyMainWindow");
    
    m_editVBoxPath->setText(settings.value("vboxPath", "VBoxManage.exe").toString());
    m_editADBPath->setText(settings.value("adbPath", "adb.exe").toString());
    m_editScrcpyPath->setText(settings.value("scrcpyPath", "scrcpy.exe").toString());
    m_editBootTimeout->setValue(settings.value("bootTimeout", 60000).toInt());
    m_editMaxFPS->setValue(settings.value("maxFPS", 60).toInt());
    m_chkStayAwake->setChecked(settings.value("stayAwake", true).toBool());
    m_chkShowTouches->setChecked(settings.value("showTouches", false).toBool());
    m_chkDisableAudio->setChecked(settings.value("disableAudio", true).toBool());
    
    // Apply to VM Controller
    m_vmController.setVBoxPath(m_editVBoxPath->text());
    m_vmController.setADBPath(m_editADBPath->text());
    m_vmController.setScrcpyPath(m_editScrcpyPath->text());
    m_vmController.setBootTimeout(m_editBootTimeout->value());
}

void ScrcpyMainWindow::onAbout() {
    QMessageBox::about(this, "About VirtualPhonePro",
        "<h2>VirtualPhonePro v2.0 - Scrcpy Edition</h2>"
        "<p>Enterprise Android Virtualization System</p>"
        "<p>Powered by Scrcpy for seamless display integration</p>"
        "<p>© 2024 VirtualPhonePro</p>"
    );
}

} // namespace VirtualPhonePro