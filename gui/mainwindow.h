#pragma once

/**
 * AntiDetectPro - Modern Cyberpunk GUI
 * Enterprise Android Anti-Detection System v1.0
 */

#include <QMainWindow>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>
#include <QLabel>
#include <QPushButton>
#include <QSlider>
#include <QLineEdit>
#include <QTextEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QProgressBar>
#include <QTableWidget>
#include <QGroupBox>
#include <QTabWidget>
#include <QSplitter>
#include <QStatusBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QIcon>
#include <QTimer>
#include <QProcess>
#include <QSystemTrayIcon>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QDialog>
#include <QPainter>
#include <QPaintEvent>
#include <QFrame>
#include <QListWidget>
#include <QTreeWidget>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QNetworkAccessManager>
#include <QNetworkReply>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

// Forward declarations
class AntiDetectEngine;
class KernelLevelHardener;
class HumanTouchEmulator;
class BatteryMaskingManager;

/**
 * Custom Cyberpunk Styled Widgets
 */

// Neon Glow Button
class NeonButton : public QPushButton {
    Q_OBJECT
public:
    explicit NeonButton(const QString& text, QWidget* parent = nullptr);
    void setNeonColor(const QColor& color);
    void setAccentColor(const QString& colorName);
    void setActive(bool active);

protected:
    void enterEvent(QEvent* event) override;
    void leaveEvent(QEvent* event) override;
    void paintEvent(QPaintEvent* event) override;

private:
    QColor m_neonColor = QColor(0, 245, 255);
    bool m_isHovered = false;
    bool m_isActive = false;
};

// Glass Morphism Card
class GlassCard : public QFrame {
    Q_OBJECT
public:
    explicit GlassCard(QWidget* parent = nullptr);
    void setGlowColor(const QColor& color);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QColor m_glowColor = QColor(0, 245, 255);
};

// Status Indicator Dot
class StatusIndicator : public QWidget {
    Q_OBJECT
public:
    enum StatusType { Online = 0, Warning = 1, Error = 2, Offline = 3 };
    Q_ENUM(StatusType)

    explicit StatusIndicator(QWidget* parent = nullptr);
    void setStatus(StatusType type);
    StatusType getStatus() const { return m_currentStatus; }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    StatusType m_currentStatus = Offline;
    void updateColor();
};

// Circular Progress Indicator
class CircularProgress : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int value READ value WRITE setValue)
    Q_PROPERTY(int maxValue READ maxValue WRITE setMaxValue)
    Q_PROPERTY(QString displayText READ displayText WRITE setDisplayText)
    Q_PROPERTY(QColor progressColor READ progressColor WRITE setProgressColor)
    Q_PROPERTY(QColor backgroundColor READ backgroundColor WRITE setBackgroundColor)

public:
    explicit CircularProgress(QWidget* parent = nullptr);

    int value() const { return m_value; }
    void setValue(int val);
    int maxValue() const { return m_maxValue; }
    void setMaxValue(int val) { m_maxValue = val; update(); }
    QString displayText() const { return m_displayText; }
    void setDisplayText(const QString& text) { m_displayText = text; update(); }
    QColor progressColor() const { return m_progressColor; }
    void setProgressColor(const QColor& color) { m_progressColor = color; update(); }
    QColor backgroundColor() const { return m_backgroundColor; }
    void setBackgroundColor(const QColor& color) { m_backgroundColor = color; update(); }

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_value = 0;
    int m_maxValue = 100;
    QString m_displayText;
    QColor m_progressColor = QColor(0, 245, 255);
    QColor m_backgroundColor = QColor(26, 26, 36);
};

// Battery Level Indicator
class BatteryIndicator : public QWidget {
    Q_OBJECT
    Q_PROPERTY(int level READ level WRITE setLevel)
    Q_PROPERTY(bool charging READ charging WRITE setCharging)

public:
    explicit BatteryIndicator(QWidget* parent = nullptr);

    int level() const { return m_level; }
    void setLevel(int level);
    bool charging() const { return m_charging; }
    void setCharging(bool charging);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    int m_level = 85;
    bool m_charging = false;
};

// Phone Preview with Touch Simulation
class PhonePreviewWidget : public QWidget {
    Q_OBJECT
public:
    explicit PhonePreviewWidget(QWidget* parent = nullptr);
    void drawSwipePath(const QList<QPoint>& path);
    void showTouch(const QPoint& point);
    void clearTouch();
    void setPhoneModel(const QString& model);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<QPoint> m_swipePath;
    QPoint m_touchPoint;
    bool m_showTouch = false;
    QString m_phoneModel = "Generic";
};

// Real-time Battery Graph
class BatteryGraph : public QWidget {
    Q_OBJECT
public:
    explicit BatteryGraph(QWidget* parent = nullptr);
    void addDataPoint(double value);
    void setGraphLabel(const QString& label);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QList<double> m_dataPoints;
    QString m_label;
    static const int MAX_POINTS = 60;
};

/**
 * Page Widgets
 */

// Dashboard - Main overview
class DashboardPage : public QWidget {
    Q_OBJECT
public:
    explicit DashboardPage(QWidget* parent = nullptr);
    void refresh();

private slots:
    void onNewProfile();
    void onRandomize();
    void onFullHarden();
    void onRefreshTimer();

private:
    void setupUI();
    void updateDeviceInfo();

    QLabel* m_deviceStatusLabel;
    QLabel* m_detectionRateLabel;
    QLabel* m_batteryLabel;
    QLabel* m_networkLabel;
    CircularProgress* m_detectionProgress;
    BatteryIndicator* m_batteryIndicator;
    PhonePreviewWidget* m_phonePreview;
    QTimer* m_refreshTimer;
};

// Profile Manager
class ProfileManagerPage : public QWidget {
    Q_OBJECT
public:
    explicit ProfileManagerPage(QWidget* parent = nullptr);
    void refreshProfiles();

private slots:
    void onCreateProfile();
    void onImportProfile();
    void onExportProfile();
    void onApplyProfile();
    void onCloneProfile();
    void onDeleteProfile();
    void onProfileSelected(QListWidgetItem* item);

private:
    void setupUI();
    void loadProfileDetails(const QString& profileId);

    QListWidget* m_profileList;
    QStackedWidget* m_detailStack;
    QMap<QString, QVariantMap> m_profiles;
};

// Device Spoofing Controls
class DeviceSpoofingPage : public QWidget {
    Q_OBJECT
public:
    explicit DeviceSpoofingPage(QWidget* parent = nullptr);

private slots:
    void onSpoofAll();
    void onResetAll();
    void onDeviceSelected(int index);
    void onRefreshDevices();

private:
    void setupUI();
    void setupBuildSection();
    void setupProductSection();
    void setupHardwareSection();
    void setupDisplaySection();

    QComboBox* m_deviceCombo;
    QTabWidget* m_tabWidget;
};

// Human Touch Simulator
class TouchSimulatorPage : public QWidget {
    Q_OBJECT
public:
    explicit TouchSimulatorPage(QWidget* parent = nullptr);

private slots:
    void onTestTouch();
    void onRecordSequence();
    void onPlaySequence();
    void onClearSequence();
    void onSaveSequence();
    void onPlaybackStep();

private:
    void setupUI();
    void executeTouch(const QVariantMap& event);

    PhonePreviewWidget* m_phonePreview;
    QListWidget* m_sequenceList;
    QList<QVariantMap> m_recordedSequence;
    QTimer* m_playbackTimer;
    int m_currentStep = 0;
    bool m_isRecording = false;
};

// Battery Simulator
class BatterySimulatorPage : public QWidget {
    Q_OBJECT
public:
    explicit BatterySimulatorPage(QWidget* parent = nullptr);

private slots:
    void onApplyPreset(const QString& preset);
    void onStartSimulation();
    void onStopSimulation();
    void onSimulationTick();

private:
    void setupUI();
    void updateBatteryDisplay();

    CircularProgress* m_batteryProgress;
    BatteryGraph* m_batteryGraph;
    QLabel* m_temperatureLabel;
    QLabel* m_voltageLabel;
    QLabel* m_statusLabel;
    QTimer* m_simulationTimer;
    int m_currentLevel = 85;
    float m_currentTemp = 32.0f;
};

// Network Spoofing
class NetworkSpoofingPage : public QWidget {
    Q_OBJECT
public:
    explicit NetworkSpoofingPage(QWidget* parent = nullptr);

private slots:
    void onSpoofMAC();
    void onSpoofCarrier();
    void onSpoofLocation();
    void onConfigureProxy();
    void onTestConnection();
    void onRandomizeMAC();

private:
    void setupUI();
    void setupMacSection();
    void setupCarrierSection();
    void setupLocationSection();
    void setupProxySection();

    QLineEdit* m_macEdit;
    QComboBox* m_carrierCombo;
    QLineEdit* m_latEdit;
    QLineEdit* m_lonEdit;
    QLineEdit* m_proxyHostEdit;
    QSpinBox* m_proxyPortSpin;
};

// Security & Bypass
class SecurityBypassPage : public QWidget {
    Q_OBJECT
public:
    explicit SecurityBypassPage(QWidget* parent = nullptr);

private slots:
    void onApplyAllBypass();
    void onRunDetectionScan();
    void onRefreshIntegrity();

private:
    void setupUI();
    void setupRootSection();
    void setupIntegritySection();
    void setupSecuritySection();
    void updateBypassStatus();

    QMap<QString, QCheckBox*> m_bypassChecks;
    QTextEdit* m_scanResults;
    QTimer* m_statusTimer;
};

// Screen Mirror (scrcpy integration)
class ScreenMirrorPage : public QWidget {
    Q_OBJECT
public:
    explicit ScreenMirrorPage(QWidget* parent = nullptr);
    ~ScreenMirrorPage();

    bool isConnected() const { return m_scrcpyRunning; }

private slots:
    void onConnectDevice();
    void onDisconnectDevice();
    void onRefreshDevices();
    void onStartMirror();
    void onStopMirror();
    void onScrcpyFinished(int exitCode, QProcess::ExitStatus status);
    void onScrcpyError(QProcess::ProcessError error);

private:
    void setupUI();
    void findScrcpyBinary();
    void listConnectedDevices();

    QComboBox* m_deviceSelector;
    QPushButton* m_refreshBtn;
    QPushButton* m_connectBtn;
    QPushButton* m_startMirrorBtn;
    QPushButton* m_stopMirrorBtn;
    QLabel* m_statusLabel;
    QTextEdit* m_logOutput;
    QProcess* m_scrcpyProcess = nullptr;
    QString m_scrcpyPath;
    bool m_scrcpyRunning = false;
    QString m_connectedDevice;
};

// Settings
class SettingsPage : public QWidget {
    Q_OBJECT
public:
    explicit SettingsPage(QWidget* parent = nullptr);

private slots:
    void onSaveSettings();
    void onLoadSettings();
    void onSelectADBPath();
    void onSelectScrcpyPath();
    void onSelectProfilePath();
    void onResetDefaults();

private:
    void setupUI();

    QLineEdit* m_adbPathEdit;
    QLineEdit* m_scrcpyPathEdit;
    QLineEdit* m_profilePathEdit;
    QCheckBox* m_autoConnectCheck;
    QCheckBox* m_minimizeTrayCheck;
    QCheckBox* m_startMinimizedCheck;
    QComboBox* m_themeCombo;
};

/**
 * Main Window
 */
class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    // Navigation
    void onNavDashboard();
    void onNavProfiles();
    void onNavSpoofing();
    void onNavTouch();
    void onNavBattery();
    void onNavNetwork();
    void onNavSecurity();
    void onNavScreenMirror();
    void onNavSettings();

    // System
    void onMinimizeToTray();
    void onShowFromTray();
    void onExit();
    void onAbout();
    void onDocumentation();

    // Device
    void onDeviceConnected();
    void onDeviceDisconnected();

    // Status updates
    void onStatusUpdateTimer();

private:
    void setupUi();
    void setupTheme();
    void setupNavigation();
    void setupConnections();
    void setupSystemTray();
    void loadSettings();
    void saveSettings();
    void updateStatusBar();

    // Main layout
    QWidget* m_centralWidget;
    QHBoxLayout* m_mainLayout;

    // Navigation
    QWidget* m_sidebar;
    QVBoxLayout* m_sidebarLayout;
    QList<QPushButton*> m_navButtons;
    QLabel* m_logoLabel;
    QLabel* m_versionLabel;

    // Content
    QStackedWidget* m_contentStack;
    QMap<QString, QWidget*> m_pages;

    // Status bar
    QLabel* m_statusDeviceLabel;
    QLabel* m_statusBatteryLabel;
    QLabel* m_statusNetworkLabel;
    StatusIndicator* m_statusIndicator;
    QLabel* m_detectionRateLabel;

    // Pages
    DashboardPage* m_dashboardPage;
    ProfileManagerPage* m_profilePage;
    DeviceSpoofingPage* m_spoofingPage;
    TouchSimulatorPage* m_touchPage;
    BatterySimulatorPage* m_batteryPage;
    NetworkSpoofingPage* m_networkPage;
    SecurityBypassPage* m_securityPage;
    ScreenMirrorPage* m_screenMirrorPage;
    SettingsPage* m_settingsPage;

    // System tray
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;

    // Timers
    QTimer* m_statusUpdateTimer;

    // Engine
    AntiDetectEngine* m_engine;
    KernelLevelHardener* m_hardener;
    HumanTouchEmulator* m_touchEmulator;
    BatteryMaskingManager* m_batteryManager;
};
