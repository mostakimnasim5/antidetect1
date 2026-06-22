/**
 * AntiDetectPro - Modern Cyberpunk GUI Implementation
 * Enterprise Android Anti-Detection System v1.0
 */

#include "mainwindow.h"
#include "../include/AntiDetectCore.hpp"
#include "../include/KernelLevelHardener.hpp"
#include "../include/HumanTouchEmulator.hpp"
#include "../include/BatteryMaskingManager.hpp"
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include <QtMath>
#include <QSettings>

// ============================================================================
// MainWindow Implementation
// ============================================================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Initialize engines
    m_engine = nullptr; // Will be initialized with AntiDetectCore
    m_hardener = nullptr;
    m_touchEmulator = nullptr;
    m_batteryManager = nullptr;

    setupUi();
    setupTheme();
    setupNavigation();
    setupConnections();
    setupSystemTray();
    loadSettings();

    setWindowTitle("AntiDetectPro - Enterprise Anti-Detection");
    setMinimumSize(1200, 800);
    resize(1400, 900);

    // Center on screen
    QRect screenGeometry = QApplication::primaryScreen()->geometry();
    int x = (screenGeometry.width() - width()) / 2;
    int y = (screenGeometry.height() - height()) / 2;
    move(x, y);
}

MainWindow::~MainWindow() {
    saveSettings();
}

// ============================================================================
// UI Setup
// ============================================================================

void MainWindow::setupUi() {
    // Central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);

    // Sidebar
    m_sidebar = new QWidget();
    m_sidebar->setFixedWidth(220);
    m_sidebar->setStyleSheet("background: #0a0a0f;");
    m_sidebarLayout = new QVBoxLayout(m_sidebar);
    m_sidebarLayout->setContentsMargins(10, 20, 10, 10);
    m_sidebarLayout->setSpacing(5);

    // Logo
    m_logoLabel = new QLabel();
    m_logoLabel->setPixmap(QPixmap()); // Logo would go here
    m_logoLabel->setText("<span style='color:#00f5ff;font-size:18px;font-weight:bold;'>🛡️ AntiDetectPro</span>");
    m_logoLabel->setAlignment(Qt::AlignCenter);
    m_sidebarLayout->addWidget(m_logoLabel);

    m_versionLabel = new QLabel("<span style='color:#8888aa;font-size:10px;'>v1.0.0 - Elite Edition</span>");
    m_versionLabel->setAlignment(Qt::AlignCenter);
    m_sidebarLayout->addWidget(m_versionLabel);

    m_sidebarLayout->addSpacing(20);

    // Initialize navigation buttons
    struct NavItem {
        const char* icon;
        const char* text;
        const char* name;
    };

    NavItem items[] = {
        {"📊", "Dashboard", "dashboard"},
        {"📋", "Profiles", "profiles"},
        {"🔧", "Spoofing", "spoofing"},
        {"👆", "Touch Sim", "touch"},
        {"🔋", "Battery", "battery"},
        {"🌐", "Network", "network"},
        {"🛡️", "Security", "security"},
        {"📱", "Screen Mirror", "screen"},
        {"⚙️", "Settings", "settings"}
    };

    for (const auto& item : items) {
        NeonButton* btn = new NeonButton(QString("%1 %2").arg(item.icon).arg(item.text));
        btn->setObjectName(QString("nav_") + item.name);
        btn->setMinimumHeight(45);
        btn->setStyleSheet("text-align: left; padding-left: 15px;");
        m_navButtons.append(btn);
        m_sidebarLayout->addWidget(btn);
    }

    m_sidebarLayout->addStretch();

    // Status indicator in sidebar
    StatusIndicator* statusDot = new StatusIndicator();
    statusDot->setStatus(StatusIndicator::Online);
    QLabel* statusText = new QLabel("<span style='color:#00ff88;'>System Ready</span>");
    QHBoxLayout* statusRow = new QHBoxLayout();
    statusRow->addWidget(statusDot);
    statusRow->addWidget(statusText);
    m_sidebarLayout->addLayout(statusRow);

    m_mainLayout->addWidget(m_sidebar);

    // Content Stack
    m_contentStack = new QStackedWidget();

    // Create pages
    m_dashboardPage = new DashboardPage();
    m_profilePage = new ProfileManagerPage();
    m_spoofingPage = new DeviceSpoofingPage();
    m_touchPage = new TouchSimulatorPage();
    m_batteryPage = new BatterySimulatorPage();
    m_networkPage = new NetworkSpoofingPage();
    m_securityPage = new SecurityBypassPage();
    m_screenMirrorPage = new ScreenMirrorPage();
    m_settingsPage = new SettingsPage();

    m_pages["dashboard"] = m_dashboardPage;
    m_pages["profiles"] = m_profilePage;
    m_pages["spoofing"] = m_spoofingPage;
    m_pages["touch"] = m_touchPage;
    m_pages["battery"] = m_batteryPage;
    m_pages["network"] = m_networkPage;
    m_pages["security"] = m_securityPage;
    m_pages["screen"] = m_screenMirrorPage;
    m_pages["settings"] = m_settingsPage;

    for (auto* page : m_pages.values()) {
        m_contentStack->addWidget(page);
    }

    m_mainLayout->addWidget(m_contentStack, 1);

    // Status bar
    QStatusBar* statusBar = new QStatusBar();
    statusBar->setStyleSheet("background: #0a0a0f; color: #8888aa;");
    setStatusBar(statusBar);

    m_statusIndicator = new StatusIndicator();
    m_statusIndicator->setStatus(StatusIndicator::Online);
    statusBar->addPermanentWidget(m_statusIndicator);

    m_statusDeviceLabel = new QLabel("No Device Connected");
    statusBar->addPermanentWidget(m_statusDeviceLabel);
    statusBar->addPermanentWidget(new QLabel(" | "));

    m_statusBatteryLabel = new QLabel("Battery: 85%");
    statusBar->addPermanentWidget(m_statusBatteryLabel);
    statusBar->addPermanentWidget(new QLabel(" | "));

    m_detectionRateLabel = new QLabel("Detection: 79%");
    m_detectionRateLabel->setStyleSheet("color: #00ff88; font-weight: bold;");
    statusBar->addPermanentWidget(m_detectionRateLabel);

    // Menu bar
    QMenuBar* menuBar = new QMenuBar();
    menuBar->setStyleSheet("background: #12121a; color: #ffffff;");

    QMenu* fileMenu = menuBar->addMenu("File");
    fileMenu->addAction("New Profile", this, [](){});
    fileMenu->addAction("Import Profile...", this, [](){});
    fileMenu->addAction("Export Profile...", this, [](){});
    fileMenu->addSeparator();
    fileMenu->addAction("Exit", this, &MainWindow::onExit);

    QMenu* toolsMenu = menuBar->addMenu("Tools");
    toolsMenu->addAction("Randomize Device", this, &MainWindow::onNavDashboard);
    toolsMenu->addAction("Full Hardening", this, &MainWindow::onNavSecurity);
    toolsMenu->addAction("Run Detection Scan", this, &MainWindow::onNavSecurity);
    toolsMenu->addSeparator();
    toolsMenu->addAction("Screen Mirror", this, &MainWindow::onNavScreenMirror);

    QMenu* helpMenu = menuBar->addMenu("Help");
    helpMenu->addAction("Documentation", this, &MainWindow::onDocumentation);
    helpMenu->addAction("About", this, &MainWindow::onAbout);

    setMenuBar(menuBar);
}

void MainWindow::setupTheme() {
    qApp->setStyle("Fusion");

    QPalette darkPalette;
    darkPalette.setColor(QPalette::Window, QColor(10, 10, 15));
    darkPalette.setColor(QPalette::WindowText, Qt::white);
    darkPalette.setColor(QPalette::Base, QColor(18, 18, 26));
    darkPalette.setColor(QPalette::AlternateBase, QColor(26, 26, 36));
    darkPalette.setColor(QPalette::ToolTipBase, Qt::white);
    darkPalette.setColor(QPalette::ToolTipText, Qt::white);
    darkPalette.setColor(QPalette::Text, Qt::white);
    darkPalette.setColor(QPalette::Button, QColor(26, 26, 36));
    darkPalette.setColor(QPalette::ButtonText, Qt::white);
    darkPalette.setColor(QPalette::BrightText, Qt::red);
    darkPalette.setColor(QPalette::Link, QColor(0, 245, 255));
    darkPalette.setColor(QPalette::LinkVisited, QColor(255, 0, 255));
    darkPalette.setColor(QPalette::Highlight, QColor(0, 245, 255));
    darkPalette.setColor(QPalette::HighlightedText, Qt::black);

    qApp->setPalette(darkPalette);
}

void MainWindow::setupNavigation() {
    // Connect navigation buttons
    if (m_navButtons.size() >= 9) {
        connect(m_navButtons[0], &QPushButton::clicked, this, &MainWindow::onNavDashboard);
        connect(m_navButtons[1], &QPushButton::clicked, this, &MainWindow::onNavProfiles);
        connect(m_navButtons[2], &QPushButton::clicked, this, &MainWindow::onNavSpoofing);
        connect(m_navButtons[3], &QPushButton::clicked, this, &MainWindow::onNavTouch);
        connect(m_navButtons[4], &QPushButton::clicked, this, &MainWindow::onNavBattery);
        connect(m_navButtons[5], &QPushButton::clicked, this, &MainWindow::onNavNetwork);
        connect(m_navButtons[6], &QPushButton::clicked, this, &MainWindow::onNavSecurity);
        connect(m_navButtons[7], &QPushButton::clicked, this, &MainWindow::onNavScreenMirror);
        connect(m_navButtons[8], &QPushButton::clicked, this, &MainWindow::onNavSettings);
    }

    // Select first button
    if (!m_navButtons.isEmpty()) {
        m_navButtons[0]->setActive(true);
    }
}

void MainWindow::setupConnections() {
    // Status update timer
    m_statusUpdateTimer = new QTimer(this);
    connect(m_statusUpdateTimer, &QTimer::timeout, this, &MainWindow::onStatusUpdateTimer);
    m_statusUpdateTimer->start(10000); // Update every 10 seconds
}

void MainWindow::setupSystemTray() {
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setIcon(QIcon()); // Icon would go here

    m_trayMenu = new QMenu(this);
    m_trayMenu->addAction("Show Window", this, &MainWindow::onShowFromTray);
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("Quick Spoof", this, [](){});
    m_trayMenu->addAction("Toggle Screen Mirror", this, [](){});
    m_trayMenu->addSeparator();
    m_trayMenu->addAction("Exit", this, &MainWindow::onExit);

    m_trayIcon->setContextMenu(m_trayMenu);
    m_trayIcon->setToolTip("AntiDetectPro - Running");

    connect(m_trayIcon, &QSystemTrayIcon::activated, [this](QSystemTrayIcon::Reason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            onShowFromTray();
        }
    });
}

void MainWindow::loadSettings() {
    QSettings settings("AntiDetectPro", "Settings");
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("windowState").toByteArray());
}

void MainWindow::saveSettings() {
    QSettings settings("AntiDetectPro", "Settings");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("windowState", saveState());
}

void MainWindow::updateStatusBar() {
    // Update status indicators
    if (m_screenMirrorPage->isConnected()) {
        m_statusIndicator->setStatus(StatusIndicator::Online);
    } else {
        m_statusIndicator->setStatus(StatusIndicator::Warning);
    }
}

// ============================================================================
// Navigation Slots
// ============================================================================

void MainWindow::onNavDashboard() {
    m_contentStack->setCurrentWidget(m_dashboardPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 0) m_navButtons[0]->setActive(true);
    m_dashboardPage->refresh();
}

void MainWindow::onNavProfiles() {
    m_contentStack->setCurrentWidget(m_profilePage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 1) m_navButtons[1]->setActive(true);
    m_profilePage->refreshProfiles();
}

void MainWindow::onNavSpoofing() {
    m_contentStack->setCurrentWidget(m_spoofingPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 2) m_navButtons[2]->setActive(true);
}

void MainWindow::onNavTouch() {
    m_contentStack->setCurrentWidget(m_touchPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 3) m_navButtons[3]->setActive(true);
}

void MainWindow::onNavBattery() {
    m_contentStack->setCurrentWidget(m_batteryPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 4) m_navButtons[4]->setActive(true);
}

void MainWindow::onNavNetwork() {
    m_contentStack->setCurrentWidget(m_networkPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 5) m_navButtons[5]->setActive(true);
}

void MainWindow::onNavSecurity() {
    m_contentStack->setCurrentWidget(m_securityPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 6) m_navButtons[6]->setActive(true);
}

void MainWindow::onNavScreenMirror() {
    m_contentStack->setCurrentWidget(m_screenMirrorPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 7) m_navButtons[7]->setActive(true);
}

void MainWindow::onNavSettings() {
    m_contentStack->setCurrentWidget(m_settingsPage);
    for (auto* btn : m_navButtons) btn->setActive(false);
    if (m_navButtons.size() > 8) m_navButtons[8]->setActive(true);
}

// ============================================================================
// System Slots
// ============================================================================

void MainWindow::onMinimizeToTray() {
    hide();
    m_trayIcon->show();
}

void MainWindow::onShowFromTray() {
    show();
    activateWindow();
    raise();
}

void MainWindow::onExit() {
    // Stop any running processes
    if (m_screenMirrorPage->isConnected()) {
        m_screenMirrorPage->~ScreenMirrorPage();
    }

    saveSettings();
    qApp->quit();
}

void MainWindow::onAbout() {
    QMessageBox about(this);
    about.setWindowTitle("About AntiDetectPro");
    about.setText(
        "<h2 style='color:#00f5ff;'>AntiDetectPro v1.0.0</h2>"
        "<p>Enterprise-Grade Android Anti-Detection System</p>"
        "<p>Commercial License - Elite Level</p>"
        "<p style='color:#8888aa;'>"
        "Advanced device fingerprinting, network spoofing, "
        "and behavioral simulation for professional application testing."
        "</p>"
        "<p style='color:#ff00ff;'>"
        "🛡️ Built for Enterprise Security Testing"
        "</p>"
    );
    about.exec();
}

void MainWindow::onDocumentation() {
    QDesktopServices::openUrl(QUrl("https://github.com/mostakimnasim5/antidetect1"));
}

void MainWindow::onDeviceConnected() {
    m_statusDeviceLabel->setText("Device: Connected");
    m_statusIndicator->setStatus(StatusIndicator::Online);
}

void MainWindow::onDeviceDisconnected() {
    m_statusDeviceLabel->setText("No Device Connected");
    m_statusIndicator->setStatus(StatusIndicator::Warning);
}

void MainWindow::onStatusUpdateTimer() {
    updateStatusBar();
}
