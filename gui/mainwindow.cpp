#include "mainwindow.h"
#include "AntiDetectEngine.hpp"
#include <QHeaderView>
#include <QInputDialog>
#include <QFileDialog>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(nullptr)
    , m_engine(nullptr)
{
    setWindowTitle("AntiDetectPro - Enterprise Anti-Detection System");
    setMinimumSize(1200, 800);
    
    m_engine = new AntiDetectEngine();
    m_engine->initialize();
    
    setupUI();
    setupMenuBar();
    setupStatusBar();
    
    updateDeviceList();
    updateProfileList();
    
    QTimer *refreshTimer = new QTimer(this);
    connect(refreshTimer, &QTimer::timeout, this, &MainWindow::onRefreshDevices);
    refreshTimer->start(5000);
}

MainWindow::~MainWindow() {
    if (m_engine) {
        m_engine->shutdown();
        delete m_engine;
    }
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    
    m_tabWidget = new QTabWidget(this);
    
    QWidget *deviceTab = createDeviceTab();
    QWidget *fingerprintTab = createFingerprintTab();
    QWidget *networkTab = createNetworkTab();
    QWidget *systemTab = createSystemTab();
    QWidget *profileTab = createProfileTab();
    QWidget *advancedTab = createAdvancedTab();
    QWidget *logTab = createLogTab();
    
    m_tabWidget->addTab(deviceTab, "📱 Device");
    m_tabWidget->addTab(fingerprintTab, "🔐 Fingerprint");
    m_tabWidget->addTab(networkTab, "🌐 Network");
    m_tabWidget->addTab(systemTab, "⚙️ System");
    m_tabWidget->addTab(profileTab, "👤 Profiles");
    m_tabWidget->addTab(advancedTab, "🔧 Advanced");
    m_tabWidget->addTab(logTab, "📋 Log");
    
    mainLayout->addWidget(m_tabWidget);
    setCentralWidget(centralWidget);
}

QWidget* MainWindow::createDeviceTab() {
    QWidget *tab = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(tab);
    
    QVBoxLayout *leftPanel = new QVBoxLayout();
    QGroupBox *deviceGroup = new QGroupBox("Connected Devices");
    QVBoxLayout *deviceLayout = new QVBoxLayout();
    
    m_deviceTable = new QTableWidget();
    m_deviceTable->setColumnCount(4);
    m_deviceTable->setHorizontalHeaderLabels({"Serial", "Model", "State", "Product"});
    m_deviceTable->horizontalHeader()->setStretchLastSection(true);
    m_deviceTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_deviceTable->setSelectionMode(QAbstractItemView::SingleSelection);
    connect(m_deviceTable, &QTableWidget::cellClicked, this, &MainWindow::onSelectDevice);
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_btnRefreshDevices = new QPushButton("🔄 Refresh");
    m_btnConnect = new QPushButton("🔗 Connect");
    m_btnDisconnect = new QPushButton("⏏️ Disconnect");
    
    connect(m_btnRefreshDevices, &QPushButton::clicked, this, &MainWindow::onRefreshDevices);
    connect(m_btnConnect, &QPushButton::clicked, this, &MainWindow::onConnectDevice);
    connect(m_btnDisconnect, &QPushButton::clicked, this, &MainWindow::onDisconnectDevice);
    
    btnLayout->addWidget(m_btnRefreshDevices);
    btnLayout->addWidget(m_btnConnect);
    btnLayout->addWidget(m_btnDisconnect);
    
    deviceLayout->addWidget(m_deviceTable);
    deviceLayout->addLayout(btnLayout);
    deviceGroup->setLayout(deviceLayout);
    
    m_lblDeviceStatus = new QLabel("Status: No device connected");
    m_lblDeviceStatus->setStyleSheet("QLabel { color: #666; font-style: italic; }");
    
    leftPanel->addWidget(deviceGroup);
    leftPanel->addWidget(m_lblDeviceStatus);
    
    QVBoxLayout *rightPanel = new QVBoxLayout();
    QGroupBox *quickActions = new QGroupBox("Quick Actions");
    QVBoxLayout *actionsLayout = new QVBoxLayout();
    
    QPushButton *btnReset = new QPushButton("🔄 Reset All Changes");
    QPushButton *btnBackup = new QPushButton("💾 Backup State");
    QPushButton *btnRestore = new QPushButton("📥 Restore State");
    QPushButton *btnGetFingerprint = new QPushButton("📋 Get Current Fingerprint");
    QPushButton *btnGetStatus = new QPushButton("📊 Get System Status");
    
    connect(btnReset, &QPushButton::clicked, this, &MainWindow::onResetAll);
    connect(btnBackup, &QPushButton::clicked, this, &MainWindow::onBackupState);
    connect(btnRestore, &QPushButton::clicked, this, &MainWindow::onRestoreState);
    connect(btnGetFingerprint, &QPushButton::clicked, this, &MainWindow::onGetCurrentFingerprint);
    connect(btnGetStatus, &QPushButton::clicked, this, &MainWindow::onGetSystemStatus);
    
    actionsLayout->addWidget(btnReset);
    actionsLayout->addWidget(btnBackup);
    actionsLayout->addWidget(btnRestore);
    actionsLayout->addWidget(btnGetFingerprint);
    actionsLayout->addWidget(btnGetStatus);
    actionsLayout->addStretch();
    
    quickActions->setLayout(actionsLayout);
    
    rightPanel->addWidget(quickActions);
    
    layout->addLayout(leftPanel, 2);
    layout->addLayout(rightPanel, 1);
    
    return tab;
}

QWidget* MainWindow::createFingerprintTab() {
    QWidget *tab = new QWidget();
    QGridLayout *layout = new QGridLayout(tab);
    
    int row = 0;
    
    QGroupBox *deviceGroup = new QGroupBox("Device Information");
    QGridLayout *deviceLayout = new QGridLayout();
    
    deviceLayout->addWidget(new QLabel("Manufacturer:"), row, 0);
    m_edtManufacturer = new QLineEdit();
    m_edtManufacturer->setPlaceholderText("e.g., Samsung");
    deviceLayout->addWidget(m_edtManufacturer, row++, 1);
    
    deviceLayout->addWidget(new QLabel("Model:"), row, 0);
    m_edtModel = new QLineEdit();
    m_edtModel->setPlaceholderText("e.g., SM-G998B");
    deviceLayout->addWidget(m_edtModel, row++, 1);
    
    deviceLayout->addWidget(new QLabel("Brand:"), row, 0);
    m_edtBrand = new QLineEdit();
    m_edtBrand->setPlaceholderText("e.g., Samsung");
    deviceLayout->addWidget(m_edtBrand, row++, 1);
    
    deviceLayout->addWidget(new QLabel("Android Version:"), row, 0);
    m_edtAndroidVersion = new QLineEdit();
    m_edtAndroidVersion->setPlaceholderText("e.g., 13");
    deviceLayout->addWidget(m_edtAndroidVersion, row++, 1);
    
    deviceLayout->addWidget(new QLabel("Build Fingerprint:"), row, 0);
    m_edtBuildFingerprint = new QLineEdit();
    deviceLayout->addWidget(m_edtBuildFingerprint, row++, 1);
    
    QPushButton *btnSpoofDevice = new QPushButton("🚀 Apply Device Spoofing");
    connect(btnSpoofDevice, &QPushButton::clicked, this, &MainWindow::onSpoofManufacturer);
    
    deviceGroup->setLayout(deviceLayout);
    
    QGroupBox *idsGroup = new QGroupBox("Device IDs");
    QGridLayout *idsLayout = new QGridLayout();
    int idRow = 0;
    
    idsLayout->addWidget(new QLabel("Device ID:"), idRow, 0);
    m_edtDeviceId = new QLineEdit();
    idsLayout->addWidget(m_edtDeviceId, idRow++, 1);
    
    idsLayout->addWidget(new QLabel("Android ID:"), idRow, 0);
    m_edtAndroidId = new QLineEdit();
    idsLayout->addWidget(m_edtAndroidId, idRow++, 1);
    
    idsLayout->addWidget(new QLabel("Serial Number:"), idRow, 0);
    m_edtSerialNumber = new QLineEdit();
    idsLayout->addWidget(m_edtSerialNumber, idRow++, 1);
    
    QPushButton *btnSpoofIds = new QPushButton("🔑 Apply ID Spoofing");
    idsLayout->addWidget(btnSpoofIds, idRow, 0, 1, 2);
    connect(btnSpoofIds, &QPushButton::clicked, this, &MainWindow::onSpoofDeviceId);
    
    idsGroup->setLayout(idsLayout);
    
    QGroupBox *screenGroup = new QGroupBox("Screen Configuration");
    QGridLayout *screenLayout = new QGridLayout();
    int screenRow = 0;
    
    screenLayout->addWidget(new QLabel("Width:"), screenRow, 0);
    m_spinWidth = new QSpinBox();
    m_spinWidth->setRange(320, 3840);
    m_spinWidth->setValue(1440);
    screenLayout->addWidget(m_spinWidth, screenRow++, 1);
    
    screenLayout->addWidget(new QLabel("Height:"), screenRow, 0);
    m_spinHeight = new QSpinBox();
    m_spinHeight->setRange(480, 3840);
    m_spinHeight->setValue(3200);
    screenLayout->addWidget(m_spinHeight, screenRow++, 1);
    
    screenLayout->addWidget(new QLabel("Density (DPI):"), screenRow, 0);
    m_spinDensity = new QSpinBox();
    m_spinDensity->setRange(120, 640);
    m_spinDensity->setValue(640);
    screenLayout->addWidget(m_spinDensity, screenRow++, 1);
    
    QPushButton *btnSpoofScreen = new QPushButton("🖥️ Apply Screen Spoofing");
    connect(btnSpoofScreen, &QPushButton::clicked, this, &MainWindow::onSpoofScreenResolution);
    screenLayout->addWidget(btnSpoofScreen, screenRow, 0, 1, 2);
    
    screenGroup->setLayout(screenLayout);
    
    layout->addWidget(deviceGroup, 0, 0);
    layout->addWidget(idsGroup, 0, 1);
    layout->addWidget(screenGroup, 1, 0, 1, 2);
    
    return tab;
}

QWidget* MainWindow::createNetworkTab() {
    QWidget *tab = new QWidget();
    QGridLayout *layout = new QGridLayout(tab);
    
    QGroupBox *macGroup = new QGroupBox("MAC Address Spoofing");
    QGridLayout *macLayout = new QGridLayout();
    
    macLayout->addWidget(new QLabel("MAC Address:"), 0, 0);
    m_edtMACAddress = new QLineEdit();
    m_edtMACAddress->setPlaceholderText("AA:BB:CC:DD:EE:FF");
    macLayout->addWidget(m_edtMACAddress, 0, 1);
    
    QPushButton *btnSpoofMAC = new QPushButton("📡 Spoof MAC");
    connect(btnSpoofMAC, &QPushButton::clicked, this, &MainWindow::onSpoofMACAddress);
    macLayout->addWidget(btnSpoofMAC, 1, 0, 1, 2);
    
    macGroup->setLayout(macLayout);
    
    QGroupBox *carrierGroup = new QGroupBox("Carrier Configuration");
    QGridLayout *carrierLayout = new QGridLayout();
    
    carrierLayout->addWidget(new QLabel("Carrier Name:"), 0, 0);
    m_edtCarrier = new QLineEdit();
    m_edtCarrier->setPlaceholderText("e.g., Verizon");
    carrierLayout->addWidget(m_edtCarrier, 0, 1);
    
    carrierLayout->addWidget(new QLabel("Country:"), 1, 0);
    m_edtCountry = new QLineEdit();
    m_edtCountry->setPlaceholderText("e.g., US");
    carrierLayout->addWidget(m_edtCountry, 1, 1);
    
    QPushButton *btnSpoofCarrier = new QPushButton("📱 Spoof Carrier");
    connect(btnSpoofCarrier, &QPushButton::clicked, this, &MainWindow::onSpoofCarrier);
    carrierLayout->addWidget(btnSpoofCarrier, 2, 0, 1, 2);
    
    carrierGroup->setLayout(carrierLayout);
    
    QGroupBox *locationGroup = new QGroupBox("Location Spoofing");
    QGridLayout *locationLayout = new QGridLayout();
    int locRow = 0;
    
    locationLayout->addWidget(new QLabel("Latitude:"), locRow, 0);
    m_edtLatitude = new QLineEdit("40.7128");
    locationLayout->addWidget(m_edtLatitude, locRow++, 1);
    
    locationLayout->addWidget(new QLabel("Longitude:"), locRow, 0);
    m_edtLongitude = new QLineEdit("-74.0060");
    locationLayout->addWidget(m_edtLongitude, locRow++, 1);
    
    m_chkMockLocation = new QCheckBox("Enable Mock Location");
    locationLayout->addWidget(m_chkMockLocation, locRow++, 0, 1, 2);
    
    QPushButton *btnSpoofLocation = new QPushButton("🌍 Spoof Location");
    connect(btnSpoofLocation, &QPushButton::clicked, this, &MainWindow::onSpoofLocation);
    locationLayout->addWidget(btnSpoofLocation, locRow, 0, 1, 2);
    
    locationGroup->setLayout(locationLayout);
    
    QGroupBox *browserGroup = new QGroupBox("Browser Spoofing");
    QGridLayout *browserLayout = new QGridLayout();
    
    browserLayout->addWidget(new QLabel("User-Agent:"), 0, 0);
    m_edtUserAgent = new QLineEdit();
    browserLayout->addWidget(m_edtUserAgent, 0, 1);
    
    browserLayout->addWidget(new QLabel("WebRTC Local IP:"), 1, 0);
    m_edtWebRTCIP = new QLineEdit();
    browserLayout->addWidget(m_edtWebRTCIP, 1, 1);
    
    QPushButton *btnSpoofBrowser = new QPushButton("🌐 Apply Browser Spoofing");
    connect(btnSpoofBrowser, &QPushButton::clicked, this, &MainWindow::onSpoofUserAgent);
    browserLayout->addWidget(btnSpoofBrowser, 2, 0, 1, 2);
    
    browserGroup->setLayout(browserLayout);
    
    layout->addWidget(macGroup, 0, 0);
    layout->addWidget(carrierGroup, 0, 1);
    layout->addWidget(locationGroup, 1, 0);
    layout->addWidget(browserGroup, 1, 1);
    
    return tab;
}

QWidget* MainWindow::createSystemTab() {
    QWidget *tab = new QWidget();
    QGridLayout *layout = new QGridLayout(tab);
    
    QGroupBox *localeGroup = new QGroupBox("Locale & Language");
    QGridLayout *localeLayout = new QGridLayout();
    int localeRow = 0;
    
    localeLayout->addWidget(new QLabel("Timezone:"), localeRow, 0);
    m_cmbTimezone = new QComboBox();
    m_cmbTimezone->addItems({"America/New_York", "America/Los_Angeles", "Europe/London", 
                            "Europe/Paris", "Asia/Tokyo", "Asia/Shanghai", "Asia/Dhaka"});
    localeLayout->addWidget(m_cmbTimezone, localeRow++, 1);
    
    localeLayout->addWidget(new QLabel("Locale:"), localeRow, 0);
    m_edtLocale = new QLineEdit("en_US");
    localeLayout->addWidget(m_edtLocale, localeRow++, 1);
    
    localeLayout->addWidget(new QLabel("Language:"), localeRow, 0);
    m_edtLanguage = new QLineEdit("en");
    localeLayout->addWidget(m_edtLanguage, localeRow++, 1);
    
    QPushButton *btnApplyLocale = new QPushButton("🌍 Apply Locale");
    connect(btnApplyLocale, &QPushButton::clicked, this, &MainWindow::onSetTimezone);
    localeLayout->addWidget(btnApplyLocale, localeRow, 0, 1, 2);
    
    localeGroup->setLayout(localeLayout);
    
    QGroupBox *batteryGroup = new QGroupBox("Battery Simulation");
    QGridLayout *batteryLayout = new QGridLayout();
    int batteryRow = 0;
    
    batteryLayout->addWidget(new QLabel("Battery Level (%):"), batteryRow, 0);
    m_spinBatteryLevel = new QSpinBox();
    m_spinBatteryLevel->setRange(0, 100);
    m_spinBatteryLevel->setValue(85);
    batteryLayout->addWidget(m_spinBatteryLevel, batteryRow++, 1);
    
    batteryLayout->addWidget(new QLabel("Status:"), batteryRow, 0);
    m_cmbBatteryStatus = new QComboBox();
    m_cmbBatteryStatus->addItems({"Charging", "Discharging", "Full", "Not Charging"});
    batteryLayout->addWidget(m_cmbBatteryStatus, batteryRow++, 1);
    
    QPushButton *btnApplyBattery = new QPushButton("🔋 Apply Battery Status");
    connect(btnApplyBattery, &QPushButton::clicked, this, &MainWindow::onSetBatteryStatus);
    batteryLayout->addWidget(btnApplyBattery, batteryRow, 0, 1, 2);
    
    batteryGroup->setLayout(batteryLayout);
    
    QGroupBox *selinuxGroup = new QGroupBox("SELinux Control");
    QHBoxLayout *selinuxLayout = new QHBoxLayout();
    
    QPushButton *btnEnableSELinux = new QPushButton("🔒 Enable SELinux");
    QPushButton *btnDisableSELinux = new QPushButton("🔓 Disable SELinux");
    
    connect(btnEnableSELinux, &QPushButton::clicked, this, &MainWindow::onEnableSELinux);
    connect(btnDisableSELinux, &QPushButton::clicked, this, &MainWindow::onDisableSELinux);
    
    selinuxLayout->addWidget(btnEnableSELinux);
    selinuxLayout->addWidget(btnDisableSELinux);
    
    selinuxGroup->setLayout(selinuxLayout);
    
    layout->addWidget(localeGroup, 0, 0);
    layout->addWidget(batteryGroup, 0, 1);
    layout->addWidget(selinuxGroup, 1, 0, 1, 2);
    
    return tab;
}

QWidget* MainWindow::createProfileTab() {
    QWidget *tab = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(tab);
    
    QVBoxLayout *leftPanel = new QVBoxLayout();
    
    QGroupBox *profileListGroup = new QGroupBox("Saved Profiles");
    QVBoxLayout *profileListLayout = new QVBoxLayout();
    
    m_profileList = new QListWidget();
    m_profileList->setMinimumWidth(300);
    
    QHBoxLayout *profileBtnLayout = new QHBoxLayout();
    QPushButton *btnRefreshProfiles = new QPushButton("🔄");
    QPushButton *btnApplyProfile = new QPushButton("✅ Apply");
    QPushButton *btnDeleteProfile = new QPushButton("🗑️");
    
    connect(btnRefreshProfiles, &QPushButton::clicked, this, &MainWindow::onLoadProfiles);
    connect(btnApplyProfile, &QPushButton::clicked, this, &MainWindow::onApplyProfile);
    connect(btnDeleteProfile, &QPushButton::clicked, this, &MainWindow::onDeleteProfile);
    
    profileBtnLayout->addWidget(btnRefreshProfiles);
    profileBtnLayout->addWidget(btnApplyProfile);
    profileBtnLayout->addWidget(btnDeleteProfile);
    
    profileListLayout->addWidget(m_profileList);
    profileListLayout->addLayout(profileBtnLayout);
    
    profileListGroup->setLayout(profileListLayout);
    
    QGroupBox *createGroup = new QGroupBox("Create Profile");
    QVBoxLayout *createLayout = new QVBoxLayout();
    
    createLayout->addWidget(new QLabel("Profile Name:"));
    m_edtProfileName = new QLineEdit();
    createLayout->addWidget(m_edtProfileName);
    
    QPushButton *btnCreateProfile = new QPushButton("➕ Create Profile");
    connect(btnCreateProfile, &QPushButton::clicked, this, &MainWindow::onCreateProfile);
    createLayout->addWidget(btnCreateProfile);
    
    createLayout->addStretch();
    
    createGroup->setLayout(createLayout);
    
    leftPanel->addWidget(profileListGroup);
    leftPanel->addWidget(createGroup);
    
    QVBoxLayout *rightPanel = new QVBoxLayout();
    
    QGroupBox *importExportGroup = new QGroupBox("Import / Export");
    QVBoxLayout *importExportLayout = new QVBoxLayout();
    
    QPushButton *btnExport = new QPushButton("📤 Export Profiles");
    QPushButton *btnImport = new QPushButton("📥 Import Profiles");
    
    connect(btnExport, &QPushButton::clicked, this, &MainWindow::onExportProfiles);
    connect(btnImport, &QPushButton::clicked, this, &MainWindow::onImportProfiles);
    
    importExportLayout->addWidget(btnExport);
    importExportLayout->addWidget(btnImport);
    
    importExportGroup->setLayout(importExportLayout);
    
    QGroupBox *presetsGroup = new QGroupBox("Quick Presets");
    QVBoxLayout *presetsLayout = new QVBoxLayout();
    
    QPushButton *btnSamsung = new QPushButton("📱 Samsung Galaxy S21 Ultra");
    QPushButton *btnPixel = new QPushButton("📱 Google Pixel 7 Pro");
    QPushButton *btnXiaomi = new QPushButton("📱 Xiaomi Mi 13 Pro");
    QPushButton *btnOnePlus = new QPushButton("📱 OnePlus 11");
    
    presetsLayout->addWidget(btnSamsung);
    presetsLayout->addWidget(btnPixel);
    presetsLayout->addWidget(btnXiaomi);
    presetsLayout->addWidget(btnOnePlus);
    
    presetsGroup->setLayout(presetsLayout);
    
    rightPanel->addWidget(importExportGroup);
    rightPanel->addWidget(presetsGroup);
    rightPanel->addStretch();
    
    layout->addLayout(leftPanel);
    layout->addLayout(rightPanel);
    
    return tab;
}

QWidget* MainWindow::createAdvancedTab() {
    QWidget *tab = new QWidget();
    QGridLayout *layout = new QGridLayout(tab);
    
    QGroupBox *hardwareGroup = new QGroupBox("Hardware Spoofing");
    QGridLayout *hardwareLayout = new QGridLayout();
    int hwRow = 0;
    
    hardwareLayout->addWidget(new QLabel("CPU Model:"), hwRow, 0);
    m_edtCPUModel = new QLineEdit();
    hardwareLayout->addWidget(m_edtCPUModel, hwRow++, 1);
    
    hardwareLayout->addWidget(new QLabel("GPU Renderer:"), hwRow, 0);
    m_edtGPU = new QLineEdit();
    hardwareLayout->addWidget(m_edtGPU, hwRow++, 1);
    
    hardwareLayout->addWidget(new QLabel("Total Memory (MB):"), hwRow, 0);
    m_spinMemory = new QSpinBox();
    m_spinMemory->setRange(1024, 65536);
    m_spinMemory->setValue(8192);
    hardwareLayout->addWidget(m_spinMemory, hwRow++, 1);
    
    QPushButton *btnSpoofHardware = new QPushButton("🖥️ Apply Hardware Spoofing");
    connect(btnSpoofHardware, &QPushButton::clicked, this, &MainWindow::onSpoofCPU);
    hardwareLayout->addWidget(btnSpoofHardware, hwRow, 0, 1, 2);
    
    hardwareGroup->setLayout(hardwareLayout);
    
    QGroupBox *sensorGroup = new QGroupBox("Sensor Simulation");
    QGridLayout *sensorLayout = new QGridLayout();
    int sensorRow = 0;
    
    sensorLayout->addWidget(new QLabel("Accelerometer X/Y/Z:"), sensorRow++, 0, 1, 3);
    
    m_spinAccelX = new QDoubleSpinBox();
    m_spinAccelY = new QDoubleSpinBox();
    m_spinAccelZ = new QDoubleSpinBox();
    m_spinAccelX->setRange(-20, 20);
    m_spinAccelY->setRange(-20, 20);
    m_spinAccelZ->setRange(-20, 20);
    
    sensorLayout->addWidget(m_spinAccelX, sensorRow, 0);
    sensorLayout->addWidget(m_spinAccelY, sensorRow, 1);
    sensorLayout->addWidget(m_spinAccelZ, sensorRow++, 2);
    
    sensorLayout->addWidget(new QLabel("Gyroscope X/Y/Z:"), sensorRow++, 0, 1, 3);
    
    m_spinGyroX = new QDoubleSpinBox();
    m_spinGyroY = new QDoubleSpinBox();
    m_spinGyroZ = new QDoubleSpinBox();
    m_spinGyroX->setRange(-10, 10);
    m_spinGyroY->setRange(-10, 10);
    m_spinGyroZ->setRange(-10, 10);
    
    sensorLayout->addWidget(m_spinGyroX, sensorRow, 0);
    sensorLayout->addWidget(m_spinGyroY, sensorRow, 1);
    sensorLayout->addWidget(m_spinGyroZ, sensorRow++, 2);
    
    QPushButton *btnSpoofSensors = new QPushButton("📡 Apply Sensor Spoofing");
    connect(btnSpoofSensors, &QPushButton::clicked, this, &MainWindow::onSpoofAccelerometer);
    sensorLayout->addWidget(btnSpoofSensors, sensorRow, 0, 1, 3);
    
    sensorGroup->setLayout(sensorLayout);
    
    QGroupBox *drmGroup = new QGroupBox("DRM & Integrity");
    QGridLayout *drmLayout = new QGridLayout();
    int drmRow = 0;
    
    drmLayout->addWidget(new QLabel("Widevine Level:"), drmRow, 0);
    m_cmbWidevineLevel = new QComboBox();
    m_cmbWidevineLevel->addItems({"L1", "L2", "L3"});
    drmLayout->addWidget(m_cmbWidevineLevel, drmRow++, 1);
    
    drmLayout->addWidget(new QLabel("HDCP Level:"), drmRow, 0);
    m_cmbHDCPLevel = new QComboBox();
    m_cmbHDCPLevel->addItems({"1.0", "1.1", "2.0", "2.1", "2.2", "2.3"});
    drmLayout->addWidget(m_cmbHDCPLevel, drmRow++, 1);
    
    m_chkBasicIntegrity = new QCheckBox("Basic Integrity");
    m_chkDeviceIntegrity = new QCheckBox("Device Integrity");
    m_chkNoCtsMismatch = new QCheckBox("No CTS Mismatch");
    
    drmLayout->addWidget(m_chkBasicIntegrity, drmRow++, 0, 1, 2);
    drmLayout->addWidget(m_chkDeviceIntegrity, drmRow++, 0, 1, 2);
    drmLayout->addWidget(m_chkNoCtsMismatch, drmRow++, 0, 1, 2);
    
    QPushButton *btnSpoofDRM = new QPushButton("🔐 Apply DRM Spoofing");
    connect(btnSpoofDRM, &QPushButton::clicked, this, &MainWindow::onSpoofWidevine);
    drmLayout->addWidget(btnSpoofDRM, drmRow, 0, 1, 2);
    
    drmGroup->setLayout(drmLayout);
    
    QGroupBox *trackerGroup = new QGroupBox("Tracker Blocking");
    QGridLayout *trackerLayout = new QGridLayout();
    
    m_trackerList = new QListWidget();
    m_trackerList->addItems({
        "google-analytics.com",
        "doubleclick.net",
        "facebook.com/tr",
        "hotjar.com",
        "mixpanel.com",
        "appsflyer.com"
    });
    
    trackerLayout->addWidget(m_trackerList, 0, 0, 3, 2);
    
    QPushButton *btnBlockTracker = new QPushButton("🚫 Block Selected");
    QPushButton *btnLoadBlocklist = new QPushButton("📂 Load Blocklist");
    
    connect(btnBlockTracker, &QPushButton::clicked, this, &MainWindow::onBlockTracker);
    connect(btnLoadBlocklist, &QPushButton::clicked, this, &MainWindow::onLoadBlocklist);
    
    trackerLayout->addWidget(btnBlockTracker, 3, 0);
    trackerLayout->addWidget(btnLoadBlocklist, 3, 1);
    
    trackerGroup->setLayout(trackerLayout);
    
    layout->addWidget(hardwareGroup, 0, 0);
    layout->addWidget(sensorGroup, 0, 1);
    layout->addWidget(drmGroup, 1, 0);
    layout->addWidget(trackerGroup, 1, 1);
    
    return tab;
}

QWidget* MainWindow::createLogTab() {
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    
    m_logText = new QTextEdit();
    m_logText->setReadOnly(true);
    m_logText->setFont(QFont("Consolas", 9));
    
    QPushButton *btnClearLog = new QPushButton("🗑️ Clear Log");
    QPushButton *btnSaveLog = new QPushButton("💾 Save Log");
    
    connect(btnClearLog, &QPushButton::clicked, [this]() { m_logText->clear(); });
    connect(btnSaveLog, &QPushButton::clicked, [this]() {
        QString filename = QFileDialog::getSaveFileName(this, "Save Log", "", "Log Files (*.log)");
        if (!filename.isEmpty()) {
            QFile file(filename);
            if (file.open(QIODevice::WriteOnly)) {
                file.write(m_logText->toPlainText().toUtf8());
                file.close();
                showSuccess("Saved", "Log saved successfully");
            }
        }
    });
    
    QHBoxLayout *btnLayout = new QHBoxLayout();
    btnLayout->addWidget(btnClearLog);
    btnLayout->addWidget(btnSaveLog);
    btnLayout->addStretch();
    
    layout->addWidget(m_logText);
    layout->addLayout(btnLayout);
    
    return tab;
}

void MainWindow::setupMenuBar() {
    QMenuBar *menuBar = this->menuBar();
    
    QMenu *fileMenu = menuBar->addMenu("📁 File");
    fileMenu->addAction("💾 Export Configuration...", this, &MainWindow::onExportProfiles);
    fileMenu->addAction("📂 Import Configuration...", this, &MainWindow::onImportProfiles);
    fileMenu->addSeparator();
    fileMenu->addAction("❌ Exit", this, &QMainWindow::close);
    
    QMenu *deviceMenu = menuBar->addMenu("📱 Device");
    deviceMenu->addAction("🔄 Refresh Devices", this, &MainWindow::onRefreshDevices);
    deviceMenu->addAction("🔗 Connect...", this, &MainWindow::onConnectDevice);
    deviceMenu->addAction("⏏️ Disconnect", this, &MainWindow::onDisconnectDevice);
    deviceMenu->addSeparator();
    deviceMenu->addAction("🔄 Reset All", this, &MainWindow::onResetAll);
    
    QMenu *profileMenu = menuBar->addMenu("👤 Profiles");
    profileMenu->addAction("📋 Load Profiles", this, &MainWindow::onLoadProfiles);
    profileMenu->addAction("➕ Create Profile...", this, &MainWindow::onCreateProfile);
    profileMenu->addAction("🗑️ Delete Profile", this, &MainWindow::onDeleteProfile);
    
    QMenu *helpMenu = menuBar->addMenu("❓ Help");
    helpMenu->addAction("📖 Documentation", this, &MainWindow::onHelp);
    helpMenu->addAction("ℹ️ About", this, &MainWindow::onAbout);
}

void MainWindow::setupStatusBar() {
    QStatusBar *statusBar = this->statusBar();
    
    m_lblStatus = new QLabel("Ready");
    m_progressBar = new QProgressBar();
    m_progressBar->setMaximumWidth(150);
    m_progressBar->setVisible(false);
    
    statusBar->addWidget(m_lblStatus);
    statusBar->addPermanentWidget(m_progressBar);
}

void MainWindow::updateDeviceList() {
    m_deviceTable->setRowCount(0);
    
    auto devices = m_engine->listDevices();
    
    for (const auto& device : devices) {
        int row = m_deviceTable->rowCount();
        m_deviceTable->insertRow(row);
        
        m_deviceTable->setItem(row, 0, new QTableWidgetItem(device.serial));
        m_deviceTable->setItem(row, 1, new QTableWidgetItem(device.model));
        m_deviceTable->setItem(row, 2, new QTableWidgetItem(device.state));
        m_deviceTable->setItem(row, 3, new QTableWidgetItem(device.product));
    }
    
    if (devices.empty()) {
        m_lblDeviceStatus->setText("Status: No devices connected");
    } else {
        m_lblDeviceStatus->setText(QString("Status: %1 device(s) connected").arg(devices.size()));
    }
}

void MainWindow::updateProfileList() {
    m_profileList->clear();
    
    auto profiles = m_engine->listProfiles();
    
    for (const auto& profile : profiles) {
        m_profileList->addItem(QString::fromStdString(profile.name) + " (" + 
                              QString::fromStdString(profile.category) + ")");
    }
}

void MainWindow::updateLog(const QString &message) {
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    m_logText->append(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::updateStatus(const QString &status) {
    m_lblStatus->setText(status);
}

void MainWindow::showError(const QString &title, const QString &message) {
    QMessageBox::critical(this, title, message);
    updateLog(QString("[ERROR] %1: %2").arg(title, message));
}

void MainWindow::showSuccess(const QString &title, const QString &message) {
    QMessageBox::information(this, title, message);
    updateLog(QString("[SUCCESS] %1: %2").arg(title, message));
}

QString MainWindow::getCurrentDeviceSerial() {
    int row = m_deviceTable->currentRow();
    if (row >= 0) {
        return m_deviceTable->item(row, 0)->text();
    }
    return QString();
}

bool MainWindow::isDeviceConnected() {
    return m_deviceTable->rowCount() > 0;
}

void MainWindow::onRefreshDevices() {
    updateDeviceList();
    updateLog("Device list refreshed");
}

void MainWindow::onConnectDevice() {
    bool ok;
    QString address = QInputDialog::getText(this, "Connect Device", 
                                           "Enter device address (IP:PORT or USB):",
                                           QLineEdit::Normal, "192.168.1.100:5555", &ok);
    if (ok && !address.isEmpty()) {
        auto result = m_engine->connect(address.toStdString());
        if (result.success) {
            showSuccess("Connected", QString::fromStdString(result.message));
            updateDeviceList();
        } else {
            showError("Connection Failed", QString::fromStdString(result.error));
        }
    }
}

void MainWindow::onDisconnectDevice() {
    auto result = m_engine->disconnect();
    if (result.success) {
        showSuccess("Disconnected", "Device disconnected");
        updateDeviceList();
    }
}

void MainWindow::onSelectDevice(int row) {
    if (row >= 0) {
        QString serial = m_deviceTable->item(row, 0)->text();
        m_engine->selectDevice(serial.toStdString());
        updateLog("Selected device: " + serial);
    }
}

void MainWindow::onSpoofManufacturer() {
    std::string manufacturer = m_edtManufacturer->text().toStdString();
    std::string model = m_edtModel->text().toStdString();
    std::string brand = m_edtBrand->text().toStdString();
    std::string version = m_edtAndroidVersion->text().toStdString();
    
    if (!manufacturer.empty()) m_engine->spoofManufacturer(manufacturer);
    if (!model.empty()) m_engine->spoofModel(model);
    if (!brand.empty()) m_engine->spoofBrand(brand);
    if (!version.empty()) m_engine->spoofAndroidVersion(version);
    
    updateLog("Device spoofing applied");
    showSuccess("Spoofing Applied", "Device information spoofed");
}

void MainWindow::onSpoofDeviceId() {
    std::string deviceId = m_edtDeviceId->text().toStdString();
    std::string androidId = m_edtAndroidId->text().toStdString();
    std::string serial = m_edtSerialNumber->text().toStdString();
    
    if (!deviceId.empty()) m_engine->spoofDeviceId(deviceId);
    if (!androidId.empty()) m_engine->spoofAndroidId(androidId);
    if (!serial.empty()) m_engine->spoofSerialNumber(serial);
    
    updateLog("Device IDs spoofed");
    showSuccess("ID Spoofing Applied", "Device IDs have been spoofed");
}

void MainWindow::onSpoofScreenResolution() {
    int width = m_spinWidth->value();
    int height = m_spinHeight->value();
    int density = m_spinDensity->value();
    
    m_engine->spoofScreenResolution(width, height);
    m_engine->spoofScreenDensity(density);
    
    updateLog(QString("Screen spoofed: %1x%2 @ %3 DPI").arg(width).arg(height).arg(density));
    showSuccess("Screen Spoofing", "Screen resolution spoofed");
}

void MainWindow::onSpoofMACAddress() {
    std::string mac = m_edtMACAddress->text().toStdString();
    if (!mac.empty()) {
        m_engine->spoofMACAddress(mac);
        updateLog("MAC address spoofed: " + QString::fromStdString(mac));
        showSuccess("MAC Spoofed", "MAC address has been spoofed");
    }
}

void MainWindow::onSpoofCarrier() {
    std::string carrier = m_edtCarrier->text().toStdString();
    std::string country = m_edtCountry->text().toStdString();
    
    if (!carrier.empty()) m_engine->spoofCarrier(carrier);
    if (!country.empty()) m_engine->spoofCountry(country);
    
    updateLog("Carrier spoofed: " + QString::fromStdString(carrier));
    showSuccess("Carrier Spoofed", "Carrier information spoofed");
}

void MainWindow::onSpoofLocation() {
    bool ok1, ok2;
    double lat = m_edtLatitude->text().toDouble(&ok1);
    double lon = m_edtLongitude->text().toDouble(&ok2);
    
    if (ok1 && ok2) {
        m_engine->spoofLocation(lat, lon);
        if (m_chkMockLocation->isChecked()) {
            m_engine->enableMockLocation();
        }
        updateLog(QString("Location spoofed: %1, %2").arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6));
        showSuccess("Location Spoofed", "GPS location has been spoofed");
    }
}

void MainWindow::onSpoofUserAgent() {
    std::string ua = m_edtUserAgent->text().toStdString();
    std::string webrtc = m_edtWebRTCIP->text().toStdString();
    
    if (!ua.empty()) m_engine->spoofUserAgent(ua);
    if (!webrtc.empty()) m_engine->spoofWebRTCIP(webrtc);
    
    updateLog("Browser spoofing applied");
    showSuccess("Browser Spoofing", "User-Agent and WebRTC IP spoofed");
}

void MainWindow::onSetTimezone() {
    std::string timezone = m_cmbTimezone->currentText().toStdString();
    std::string locale = m_edtLocale->text().toStdString();
    std::string language = m_edtLanguage->text().toStdString();
    
    m_engine->setTimezone(timezone);
    if (!locale.empty()) m_engine->setLocale(locale);
    if (!language.empty()) m_engine->setLanguage(language);
    
    updateLog("Locale settings applied");
    showSuccess("Locale Set", "Timezone and locale updated");
}

void MainWindow::onSetBatteryStatus() {
    int level = m_spinBatteryLevel->value();
    std::string status = m_cmbBatteryStatus->currentText().toStdString();
    
    m_engine->setBatteryStatus(level, status);
    updateLog(QString("Battery status: %1% %2").arg(level).arg(QString::fromStdString(status)));
    showSuccess("Battery Set", "Battery status simulated");
}

void MainWindow::onEnableSELinux() {
    m_engine->enableSELinux();
    updateLog("SELinux enabled");
    showSuccess("SELinux", "SELinux set to Enforcing");
}

void MainWindow::onDisableSELinux() {
    m_engine->disableSELinux();
    updateLog("SELinux disabled");
    showSuccess("SELinux", "SELinux set to Permissive");
}

void MainWindow::onSpoofWidevine() {
    int level = m_cmbWidevineLevel->currentIndex() + 1;
    std::string hdcp = m_cmbHDCPLevel->currentText().toStdString();
    
    m_engine->spoofWidevineLevel(level);
    m_engine->spoofHDCPLevel(hdcp);
    
    if (m_chkBasicIntegrity->isChecked()) m_engine->enableBasicIntegrity();
    if (m_chkDeviceIntegrity->isChecked()) m_engine->enableDeviceIntegrity();
    if (m_chkNoCtsMismatch->isChecked()) m_engine->enableNoCtsMismatch();
    
    updateLog("DRM spoofing applied");
    showSuccess("DRM Spoofing", "Widevine and Integrity checks spoofed");
}

void MainWindow::onSpoofCPU() {
    std::string cpu = m_edtCPUModel->text().toStdString();
    std::string gpu = m_edtGPU->text().toStdString();
    int memory = m_spinMemory->value();
    
    if (!cpu.empty()) m_engine->spoofCPUModel(cpu);
    if (!gpu.empty()) m_engine->spoofGPU(gpu);
    m_engine->spoofTotalMemory(memory);
    
    updateLog("Hardware spoofing applied");
    showSuccess("Hardware Spoofing", "CPU, GPU, and memory spoofed");
}

void MainWindow::onSpoofAccelerometer() {
    float ax = m_spinAccelX->value();
    float ay = m_spinAccelY->value();
    float az = m_spinAccelZ->value();
    float gx = m_spinGyroX->value();
    float gy = m_spinGyroY->value();
    float gz = m_spinGyroZ->value();
    
    m_engine->spoofAccelerometer(ax, ay, az);
    m_engine->spoofGyroscope(gx, gy, gz);
    
    updateLog(QString("Sensor data spoofed"));
    showSuccess("Sensor Spoofing", "Accelerometer and Gyroscope spoofed");
}

void MainWindow::onLoadProfiles() {
    updateProfileList();
    updateLog("Profiles loaded");
}

void MainWindow::onApplyProfile() {
    int currentRow = m_profileList->currentRow();
    if (currentRow >= 0) {
        auto profiles = m_engine->listProfiles();
        if (currentRow < static_cast<int>(profiles.size())) {
            m_engine->applyProfile(profiles[currentRow].id);
            updateLog("Profile applied: " + m_profileList->currentItem()->text());
            showSuccess("Profile Applied", "Profile applied successfully");
        }
    }
}

void MainWindow::onCreateProfile() {
    QString name = m_edtProfileName->text();
    if (!name.isEmpty()) {
        m_engine->createProfile(name.toStdString(), {});
        updateProfileList();
        updateLog("Profile created: " + name);
        showSuccess("Profile Created", "Profile created successfully");
    }
}

void MainWindow::onDeleteProfile() {
    int currentRow = m_profileList->currentRow();
    if (currentRow >= 0) {
        auto profiles = m_engine->listProfiles();
        if (currentRow < static_cast<int>(profiles.size())) {
            m_engine->deleteProfile(profiles[currentRow].id);
            updateProfileList();
            updateLog("Profile deleted");
            showSuccess("Profile Deleted", "Profile deleted successfully");
        }
    }
}

void MainWindow::onExportProfiles() {
    QString dir = QFileDialog::getExistingDirectory(this, "Export Profiles Directory");
    if (!dir.isEmpty()) {
        m_engine->exportProfiles(dir.toStdString());
        updateLog("Profiles exported to: " + dir);
        showSuccess("Exported", "Profiles exported successfully");
    }
}

void MainWindow::onImportProfiles() {
    QString dir = QFileDialog::getExistingDirectory(this, "Import Profiles Directory");
    if (!dir.isEmpty()) {
        m_engine->importProfiles(dir.toStdString());
        updateProfileList();
        updateLog("Profiles imported from: " + dir);
        showSuccess("Imported", "Profiles imported successfully");
    }
}

void MainWindow::onBlockTracker() {
    auto selectedItems = m_trackerList->selectedItems();
    for (auto item : selectedItems) {
        m_engine->blockTracker(item->text().toStdString());
        updateLog("Tracker blocked: " + item->text());
    }
    showSuccess("Trackers Blocked", QString("%1 tracker(s) blocked").arg(selectedItems.size()));
}

void MainWindow::onUnblockTracker() {
    auto selectedItems = m_trackerList->selectedItems();
    for (auto item : selectedItems) {
        m_engine->unblockTracker(item->text().toStdString());
        updateLog("Tracker unblocked: " + item->text());
    }
}

void MainWindow::onLoadBlocklist() {
    QString file = QFileDialog::getOpenFileName(this, "Load Blocklist", "", "Text Files (*.txt)");
    if (!file.isEmpty()) {
        m_engine->loadBlocklist(file.toStdString());
        updateLog("Blocklist loaded from: " + file);
        showSuccess("Blocklist Loaded", "Tracker blocklist loaded");
    }
}

void MainWindow::onResetAll() {
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Reset All",
        "Are you sure you want to reset all spoofing changes?",
        QMessageBox::Yes | QMessageBox::No);
    
    if (reply == QMessageBox::Yes) {
        m_engine->resetAll();
        updateLog("All spoofing changes reset");
        showSuccess("Reset Complete", "All changes have been reset");
    }
}

void MainWindow::onBackupState() {
    m_engine->backupState();
    updateLog("State backed up");
    showSuccess("Backup Complete", "Current state backed up");
}

void MainWindow::onRestoreState() {
    m_engine->restoreState();
    updateLog("State restored");
    showSuccess("Restore Complete", "State restored from backup");
}

void MainWindow::onGetCurrentFingerprint() {
    auto fingerprint = m_engine->getCurrentFingerprint();
    QString info = "Current Device Fingerprint:\n\n";
    for (const auto& [key, value] : fingerprint) {
        info += QString("%1: %2\n").arg(QString::fromStdString(key))
                               .arg(QString::fromStdString(value));
    }
    
    QMessageBox::information(this, "Current Fingerprint", info);
    updateLog("Fingerprint information displayed");
}

void MainWindow::onGetSystemStatus() {
    auto status = m_engine->getSystemStatus();
    QString info = "System Status:\n\n";
    for (const auto& [key, value] : status) {
        info += QString("%1: %2\n").arg(QString::fromStdString(key))
                               .arg(QString::fromStdString(value));
    }
    
    QMessageBox::information(this, "System Status", info);
    updateLog("System status displayed");
}

void MainWindow::onAbout() {
    QString about = "AntiDetectPro v1.0.0\n\n";
    about += "Enterprise Anti-Detection System\n";
    about += "For Android Device Fingerprinting & Spoofing\n\n";
    about += "Copyright (c) 2024\n";
    about += "Commercial License - Elite Level";
    
    QMessageBox::about(this, "About AntiDetectPro", about);
}

void MainWindow::onHelp() {
    QString help = "AntiDetectPro - Help\n\n";
    help += "1. Connect to a device using the Connect button or menu\n";
    help += "2. Use the tabs to configure different spoofing options\n";
    help += "3. Create and manage profiles for quick setup\n";
    help += "4. Use Reset to restore original device state\n\n";
    help += "For more information, see the README.md file.";
    
    QMessageBox::information(this, "Help", help);
}
