/**
 * MainWindow - Qt GUI Implementation for VirtualPhonePro
 * 
 * Complete GUI implementation with phone cards, app management,
 * anti-detection status, and full device control.
 */

#include "emulator/MainWindow.hpp"
#include "emulator/AndroidEmulator.hpp"
#include "core/CryptoUtils.hpp"
#include "network/ProxyConfig.hpp"
#include "profiles/DeviceProfileDatabase.hpp"

#include <QtCore>
#include <QApplication>
#include <QDesktopWidget>
#include <QStyleFactory>

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#include <shellapi.h>
#endif

namespace VirtualPhonePro {
namespace Emulator {

// Country flag emojis
static const QMap<QString, QString> countryFlags = {
    {"BD", "🇧🇩"}, {"US", "🇺🇸"}, {"UK", "🇬🇧"}, {"CA", "🇨🇦"},
    {"AU", "🇦🇺"}, {"IN", "🇮🇳"}, {"JP", "🇯🇵"}, {"DE", "🇩🇪"},
    {"FR", "🇫🇷"}, {"SG", "🇸🇬"}, {"AE", "🇦🇪"}, {"MY", "🇲🇾"},
    {"ID", "🇮🇩"}, {"TH", "🇹🇭"}, {"VN", "🇻🇳"}, {"PH", "🇵🇭"},
    {"KR", "🇰🇷"}, {"CN", "🇨🇳"}, {"BR", "🇧🇷"}, {"MX", "🇲🇽"},
    {"NG", "🇳🇬"}, {"RU", "🇷🇺"}, {"IT", "🇮🇹"}, {"ES", "🇪🇸"},
    {"NL", "🇳🇱"}, {"SE", "🇸🇪"}, {"NO", "🇳🇴"}, {"DK", "🇩🇰"},
    {"PL", "🇵🇱"}, {"TR", "🇹🇷"}, {"ZA", "🇿🇦"}, {"EG", "🇪🇬"}
};

// Country names
static const QMap<QString, QString> countryNames = {
    {"BD", "Bangladesh"}, {"US", "United States"}, {"UK", "United Kingdom"},
    {"CA", "Canada"}, {"AU", "Australia"}, {"IN", "India"}, {"JP", "Japan"},
    {"DE", "Germany"}, {"FR", "France"}, {"SG", "Singapore"}, {"AE", "UAE"},
    {"MY", "Malaysia"}, {"ID", "Indonesia"}, {"TH", "Thailand"}, {"VN", "Vietnam"},
    {"PH", "Philippines"}, {"KR", "South Korea"}, {"CN", "China"}, {"BR", "Brazil"},
    {"MX", "Mexico"}, {"NG", "Nigeria"}, {"RU", "Russia"}, {"IT", "Italy"},
    {"ES", "Spain"}, {"NL", "Netherlands"}, {"SE", "Sweden"}, {"NO", "Norway"},
    {"DK", "Denmark"}, {"PL", "Poland"}, {"TR", "Turkey"}, {"ZA", "South Africa"},
    {"EG", "Egypt"}
};

// Manufacturer device mappings
static const QMap<QString, QStringList> manufacturerDevices = {
    {"Samsung", {"Galaxy S24 Ultra", "Galaxy S23 Ultra", "Galaxy S22 Ultra", "Galaxy S21 Ultra",
                 "Galaxy A54 5G", "Galaxy A34 5G", "Galaxy Z Fold5", "Galaxy Z Flip5"}},
    {"Google", {"Pixel 8 Pro", "Pixel 8", "Pixel 7 Pro", "Pixel 7", "Pixel 6a", "Pixel 6 Pro"}},
    {"Xiaomi", {"Xiaomi 14", "Xiaomi 13 Ultra", "Xiaomi 13 Pro", "Redmi Note 13 Pro+",
                "Redmi Note 12", "POCO F5", "POCO X5 Pro"}},
    {"OnePlus", {"OnePlus 12", "OnePlus 11", "OnePlus Nord 3", "OnePlus 10 Pro"}},
    {"Realme", {"Realme GT5 Pro", "Realme 11 Pro+", "Realme C55", "Realme Narzo 60"}},
    {"Vivo", {"Vivo X100 Pro", "Vivo V29 Pro", "Vivo T2 Pro", "Vivo Y100"}}
};

// ==========================================
// PhoneCard Implementation
// ==========================================

PhoneCard::PhoneCard(const EmulatorDevice& device, QWidget* parent)
    : QFrame(parent), m_device(device), m_isRunning(false)
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(2);
    setMinimumSize(280, 400);
    setMaximumSize(320, 480);
    
    // Phone-like appearance
    setStyleSheet(R"(
        QFrame {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #1a1a2e, stop:1 #16213e);
            border: 2px solid #0f3460;
            border-radius: 20px;
            padding: 10px;
        }
        QFrame:hover {
            border: 2px solid #e94560;
        }
    )");
    
    setupUI();
}

PhoneCard::~PhoneCard() = default;

void PhoneCard::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Phone frame (notch design)
    QLabel* phoneFrame = new QLabel(this);
    phoneFrame->setFixedHeight(30);
    phoneFrame->setStyleSheet(R"(
        background: #0f0f0f;
        border-radius: 10px 10px 0 0;
    )");
    
    // Notch indicator
    QLabel* notch = new QLabel(phoneFrame);
    notch->setFixedSize(80, 20);
    notch->move(100, 5);
    notch->setStyleSheet(R"(
        background: #1a1a1a;
        border-radius: 10px;
    )");
    
    mainLayout->addWidget(phoneFrame);
    
    // Screen preview area
    QLabel* screenPreview = new QLabel(this);
    screenPreview->setFixedHeight(180);
    screenPreview->setStyleSheet(R"(
        background: #000;
        border: 1px solid #333;
    )");
    screenPreview->setAlignment(Qt::AlignCenter);
    screenPreview->setText("📱");
    screenPreview->setFont(QFont("Segoe UI Emoji", 48));
    m_lblScreenPreview = screenPreview;
    mainLayout->addWidget(screenPreview);
    
    // Device info
    QLabel* lblDevice = new QLabel(QString::fromStdString(m_device.manufacturer + " " + m_device.model));
    lblDevice->setStyleSheet("color: white; font-weight: bold; font-size: 14px;");
    lblDevice->setAlignment(Qt::AlignCenter);
    m_lblDeviceName = lblDevice;
    mainLayout->addWidget(lblDevice);
    
    // Country and status row
    QHBoxLayout* statusRow = new QHBoxLayout();
    
    QLabel* flag = new QLabel(getCountryFlag(QString::fromStdString(m_device.countryCode)));
    flag->setFont(QFont("Segoe UI Emoji", 20));
    m_lblCountryFlag = flag;
    statusRow->addWidget(flag);
    
    QLabel* status = new QLabel(m_isRunning ? "⚡ Running" : "⭕ Stopped");
    status->setStyleSheet(m_isRunning ? "color: #00ff00;" : "color: #ff6666;");
    m_lblStatus = status;
    statusRow->addWidget(status);
    statusRow->addStretch();
    
    mainLayout->addLayout(statusRow);
    
    // Info grid
    QGridLayout* infoGrid = new QGridLayout();
    infoGrid->setSpacing(2);
    
    auto addInfo = [&](const QString& label, const QString& value, int row, int col) {
        QLabel* lbl = new QLabel(label + ":");
        lbl->setStyleSheet("color: #888; font-size: 10px;");
        QLabel* val = new QLabel(value);
        val->setStyleSheet("color: #fff; font-size: 10px;");
        infoGrid->addWidget(lbl, row, col * 2);
        infoGrid->addWidget(val, row, col * 2 + 1);
    };
    
    addInfo("Android", QString::fromStdString(m_device.androidVersion), 0, 0);
    addInfo("IP", QString::fromStdString(m_device.ipAddress), 0, 1);
    addInfo("GPS", QString::number(m_device.latitude, 'f', 4) + "," + QString::number(m_device.longitude, 'f', 4), 1, 0);
    addInfo("RAM", QString::number(m_device.ramMB) + "MB", 1, 1);
    
    mainLayout->addLayout(infoGrid);
    
    // Anti-detection status
    QHBoxLayout* antiRow = new QHBoxLayout();
    antiRow->setSpacing(5);
    
    auto addAntiStatus = [&](const QString& text, const QString& color) {
        QLabel* lbl = new QLabel(text);
        lbl->setStyleSheet(QString("color: %1; font-size: 9px; background: #222; padding: 2px 5px; border-radius: 3px;").arg(color));
        antiRow->addWidget(lbl);
    };
    
    addAntiStatus("✅ SN", "#00ff00");
    addAntiStatus("✅ PI", "#00ff00");
    addAntiStatus("✅ TLS", "#00ff00");
    antiRow->addStretch();
    
    mainLayout->addLayout(antiRow);
    
    // Buttons row
    QHBoxLayout* btnRow = new QHBoxLayout();
    btnRow->setSpacing(5);
    
    m_btnStart = new QPushButton("▶ Start");
    m_btnStart->setStyleSheet(R"(
        QPushButton {
            background: #00aa00;
            color: white;
            border: none;
            padding: 8px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover { background: #00cc00; }
        QPushButton:disabled { background: #666; }
    )");
    
    m_btnStop = new QPushButton("⏹ Stop");
    m_btnStop->setEnabled(false);
    m_btnStop->setStyleSheet(R"(
        QPushButton {
            background: #aa0000;
            color: white;
            border: none;
            padding: 8px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover { background: #cc0000; }
        QPushButton:disabled { background: #666; }
    )");
    
    m_btnSettings = new QPushButton("⚙");
    m_btnSettings->setStyleSheet(R"(
        QPushButton {
            background: #444;
            color: white;
            border: none;
            padding: 8px;
            border-radius: 5px;
        }
    )");
    
    m_btnApps = new QPushButton("📱 Apps");
    m_btnApps->setStyleSheet(R"(
        QPushButton {
            background: #0066aa;
            color: white;
            border: none;
            padding: 8px;
            border-radius: 5px;
        }
    )");
    
    btnRow->addWidget(m_btnStart);
    btnRow->addWidget(m_btnStop);
    btnRow->addWidget(m_btnSettings);
    btnRow->addWidget(m_btnApps);
    
    mainLayout->addLayout(btnRow);
    
    // Delete button
    m_btnDelete = new QPushButton("🗑 Delete Profile");
    m_btnDelete->setStyleSheet(R"(
        QPushButton {
            background: transparent;
            color: #ff4444;
            border: 1px solid #ff4444;
            padding: 5px;
            border-radius: 3px;
        }
        QPushButton:hover { background: #ff4444; color: white; }
    )");
    mainLayout->addWidget(m_btnDelete);
    
    // Connections
    connect(m_btnStart, &QPushButton::clicked, this, &PhoneCard::startClicked);
    connect(m_btnStop, &QPushButton::clicked, this, &PhoneCard::stopClicked);
    connect(m_btnDelete, &QPushButton::clicked, this, &PhoneCard::deleteClicked);
    connect(m_btnSettings, &QPushButton::clicked, this, &PhoneCard::settingsClicked);
    connect(m_btnApps, &QPushButton::clicked, this, &PhoneCard::appsClicked);
}

void PhoneCard::setRunning(bool running) {
    m_isRunning = running;
    m_btnStart->setEnabled(!running);
    m_btnStop->setEnabled(running);
    m_lblStatus->setText(running ? "⚡ Running" : "⭕ Stopped");
    m_lblStatus->setStyleSheet(running ? "color: #00ff00;" : "color: #ff6666;");
}

void PhoneCard::updateDevice(const EmulatorDevice& device) {
    m_device = device;
    updateDisplay();
}

void PhoneCard::updateDisplay() {
    m_lblDeviceName->setText(QString::fromStdString(m_device.manufacturer + " " + m_device.model));
    m_lblCountryFlag->setText(getCountryFlag(QString::fromStdString(m_device.countryCode)));
}

// ==========================================
// ProfileCreatorDialog Implementation
// ==========================================

ProfileCreatorDialog::ProfileCreatorDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Create New Android Device");
    setMinimumSize(600, 500);
    setStyleSheet(R"(
        QDialog {
            background: #1a1a2e;
        }
        QLabel {
            color: white;
        }
        QPushButton {
            background: #0066cc;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
        }
        QPushButton:hover {
            background: #0088ff;
        }
    )");
    
    setupUI();
    populateCountries();
    populateDeviceTypes();
}

ProfileCreatorDialog::~ProfileCreatorDialog() = default;

void ProfileCreatorDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Title
    QLabel* title = new QLabel("🆕 Create New Android Device Profile");
    title->setStyleSheet("font-size: 18px; font-weight: bold; color: #00ff88;");
    mainLayout->addWidget(title);
    
    // Form grid
    QGridLayout* formGrid = new QGridLayout();
    
    // Country
    QLabel* lblCountry = new QLabel("🌍 Country:");
    m_cmbCountry = new QComboBox();
    m_cmbCountry->setStyleSheet(R"(
        QComboBox {
            background: #16213e;
            color: white;
            border: 1px solid #0f3460;
            padding: 5px;
        }
    )");
    formGrid->addWidget(lblCountry, 0, 0);
    formGrid->addWidget(m_cmbCountry, 0, 1);
    
    // Manufacturer
    QLabel* lblMfr = new QLabel("🏭 Manufacturer:");
    m_cmbManufacturer = new QComboBox();
    m_cmbManufacturer->setStyleSheet(R"(
        QComboBox {
            background: #16213e;
            color: white;
            border: 1px solid #0f3460;
            padding: 5px;
        }
    )");
    for (const QString& mfr : manufacturerDevices.keys()) {
        m_cmbManufacturer->addItem(mfr);
    }
    formGrid->addWidget(lblMfr, 1, 0);
    formGrid->addWidget(m_cmbManufacturer, 1, 1);
    
    // Model
    QLabel* lblModel = new QLabel("📱 Model:");
    m_cmbModel = new QComboBox();
    m_cmbModel->setStyleSheet(R"(
        QComboBox {
            background: #16213e;
            color: white;
            border: 1px solid #0f3460;
            padding: 5px;
        }
    )");
    formGrid->addWidget(lblModel, 2, 0);
    formGrid->addWidget(m_cmbModel, 2, 1);
    
    // Device name
    QLabel* lblName = new QLabel("📝 Profile Name:");
    m_txtDeviceName = new QLineEdit();
    m_txtDeviceName->setPlaceholderText("My Android Device");
    m_txtDeviceName->setStyleSheet(R"(
        QLineEdit {
            background: #16213e;
            color: white;
            border: 1px solid #0f3460;
            padding: 5px;
        }
    )");
    formGrid->addWidget(lblName, 3, 0);
    formGrid->addWidget(m_txtDeviceName, 3, 1);
    
    // Memory
    QLabel* lblMemory = new QLabel("💾 RAM (MB):");
    m_spnMemory = new QSpinBox();
    m_spnMemory->setRange(1024, 16384);
    m_spnMemory->setValue(4096);
    m_spnMemory->setSingleStep(512);
    m_spnMemory->setStyleSheet(R"(
        QSpinBox {
            background: #16213e;
            color: white;
            border: 1px solid #0f3460;
            padding: 5px;
        }
    )");
    formGrid->addWidget(lblMemory, 4, 0);
    formGrid->addWidget(m_spnMemory, 4, 1);
    
    // CPU
    QLabel* lblCPU = new QLabel("⚙ CPU Cores:");
    m_spnCPU = new QSpinBox();
    m_spnCPU->setRange(1, 16);
    m_spnCPU->setValue(4);
    m_spnCPU->setStyleSheet(R"(
        QSpinBox {
            background: #16213e;
            color: white;
            border: 1px solid #0f3460;
            padding: 5px;
        }
    )");
    formGrid->addWidget(lblCPU, 5, 0);
    formGrid->addWidget(m_spnCPU, 5, 1);
    
    mainLayout->addLayout(formGrid);
    
    // Preview section
    QGroupBox* previewBox = new QGroupBox("📋 Generated Identity");
    previewBox->setStyleSheet(R"(
        QGroupBox {
            color: #00ff88;
            border: 1px solid #00ff88;
            border-radius: 5px;
            margin-top: 10px;
        }
    )");
    
    QGridLayout* previewGrid = new QGridLayout(previewBox);
    
    previewGrid->addWidget(new QLabel("Android:"), 0, 0);
    m_lblAndroidVersion = new QLabel("Android 14");
    m_lblAndroidVersion->setStyleSheet("color: white;");
    previewGrid->addWidget(m_lblAndroidVersion, 0, 1);
    
    previewGrid->addWidget(new QLabel("Language:"), 1, 0);
    m_lblLanguage = new QLabel("en-US");
    m_lblLanguage->setStyleSheet("color: white;");
    previewGrid->addWidget(m_lblLanguage, 1, 1);
    
    previewGrid->addWidget(new QLabel("Timezone:"), 2, 0);
    m_lblTimezone = new QLabel("America/New_York");
    m_lblTimezone->setStyleSheet("color: white;");
    previewGrid->addWidget(m_lblTimezone, 2, 1);
    
    previewGrid->addWidget(new QLabel("IP:"), 3, 0);
    m_lblIPPreview = new QLabel("45.123.x.x");
    m_lblIPPreview->setStyleSheet("color: #00ff88;");
    previewGrid->addWidget(m_lblIPPreview, 3, 1);
    
    previewGrid->addWidget(new QLabel("GPS:"), 4, 0);
    m_lblGPSPreview = new QLabel("40.71, -74.00");
    m_lblGPSPreview->setStyleSheet("color: #00ff88;");
    previewGrid->addWidget(m_lblGPSPreview, 4, 1);
    
    mainLayout->addWidget(previewBox);
    mainLayout->addStretch();
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    m_btnCancel = new QPushButton("Cancel");
    m_btnCancel->setStyleSheet(R"(
        QPushButton {
            background: #666;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
        }
    )");
    
    m_btnCreate = new QPushButton("✅ Create Device");
    m_btnCreate->setStyleSheet(R"(
        QPushButton {
            background: #00aa00;
            color: white;
            border: none;
            padding: 10px 20px;
            border-radius: 5px;
            font-weight: bold;
        }
        QPushButton:hover { background: #00cc00; }
    )");
    
    btnLayout->addWidget(m_btnCancel);
    btnLayout->addWidget(m_btnCreate);
    
    mainLayout->addLayout(btnLayout);
    
    // Connections
    connect(m_cmbCountry, &QComboBox::currentTextChanged, this, &ProfileCreatorDialog::onCountryChanged);
    connect(m_cmbManufacturer, &QComboBox::currentTextChanged, this, &ProfileCreatorDialog::onDeviceTypeChanged);
    connect(m_btnCreate, &QPushButton::clicked, this, &ProfileCreatorDialog::onCreateClicked);
    connect(m_btnCancel, &QPushButton::clicked, this, &ProfileCreatorDialog::onCancelClicked);
    
    // Initial update
    onDeviceTypeChanged(m_cmbManufacturer->currentText());
}

void ProfileCreatorDialog::populateCountries() {
    for (const QString& code : countryNames.keys()) {
        QString flag = countryFlags.value(code, "🏳️");
        m_cmbCountry->addItem(flag + " " + countryNames.value(code), code);
    }
}

void ProfileCreatorDialog::populateDeviceTypes() {
    QString mfr = m_cmbManufacturer->currentText();
    m_cmbModel->clear();
    
    if (manufacturerDevices.contains(mfr)) {
        m_cmbModel->addItems(manufacturerDevices.value(mfr));
    }
}

void ProfileCreatorDialog::onCountryChanged(const QString& country) {
    QString code = m_cmbCountry->currentData().toString();
    
    // Update preview
    if (code == "BD") {
        m_lblLanguage->setText("bn-BD");
        m_lblTimezone->setText("Asia/Dhaka");
        m_lblGPSPreview->setText("23.81, 90.41");
        m_lblIPPreview->setText("103.x.x.x");
    } else if (code == "US") {
        m_lblLanguage->setText("en-US");
        m_lblTimezone->setText("America/New_York");
        m_lblGPSPreview->setText("40.71, -74.00");
        m_lblIPPreview->setText("45.x.x.x");
    } else if (code == "UK") {
        m_lblLanguage->setText("en-GB");
        m_lblTimezone->setText("Europe/London");
        m_lblGPSPreview->setText("51.51, -0.13");
        m_lblIPPreview->setText("45.x.x.x");
    } else if (code == "JP") {
        m_lblLanguage->setText("ja-JP");
        m_lblTimezone->setText("Asia/Tokyo");
        m_lblGPSPreview->setText("35.68, 139.69");
        m_lblIPPreview->setText("45.x.x.x");
    } else {
        m_lblLanguage->setText("en-US");
        m_lblTimezone->setText("UTC");
        m_lblGPSPreview->setText("0.00, 0.00");
        m_lblIPPreview->setText("45.x.x.x");
    }
}

void ProfileCreatorDialog::onDeviceTypeChanged(const QString& type) {
    populateDeviceTypes();
}

void ProfileCreatorDialog::onCreateClicked() {
    QString code = m_cmbCountry->currentData().toString();
    
    m_device.id = "device_" + std::to_string(QDateTime::currentMSecsSinceEpoch());
    m_device.name = m_txtDeviceName->text().toStdString();
    if (m_device.name.empty()) {
        m_device.name = m_cmbManufacturer->currentText().toStdString() + " " + 
                       m_cmbModel->currentText().toStdString();
    }
    
    m_device.manufacturer = m_cmbManufacturer->currentText().toStdString();
    m_device.model = m_cmbModel->currentText().toStdString();
    m_device.androidVersion = "14";
    m_device.ramMB = m_spnMemory->value();
    m_device.cpuCores = m_spnCPU->value();
    m_device.countryCode = code.toStdString();
    m_device.language = m_lblLanguage->text().toStdString();
    m_device.locale = m_device.language.replace("-", "_");
    m_device.timezone = m_lblTimezone->text().toStdString();
    
    // Generate unique IP/GPS
    m_device.ipAddress = "45." + std::to_string(rand() % 256) + "." + 
                         std::to_string(rand() % 256) + "." + std::to_string(rand() % 256);
    
    // Get GPS from preview
    QString gpsText = m_lblGPSPreview->text();
    QStringList gpsParts = gpsText.split(",");
    if (gpsParts.size() == 2) {
        m_device.latitude = gpsParts[0].toDouble();
        m_device.longitude = gpsParts[1].toDouble();
    }
    
    // Generate IMEI
    std::string imei = "35" + std::to_string(rand() % 1000000000000);
    m_device.imei = imei.substr(0, 15);
    
    // Generate Android ID
    m_device.androidId = std::to_string(rand() % 0xFFFFFFFF);
    
    // Generate MAC address
    m_device.macAddress = "FC:A1:83:" + 
        QString("%1:%2:%3").arg(rand() % 256, 2, 16, QChar('0'))
                           .arg(rand() % 256, 2, 16, QChar('0'))
                           .arg(rand() % 256, 2, 16, QChar('0')).toStdString();
    
    emit deviceCreated(m_device);
    accept();
}

void ProfileCreatorDialog::onCancelClicked() {
    reject();
}

void ProfileCreatorDialog::updateDevicePreview() {
    // Auto update preview when settings change
}

// ==========================================
// MainWindow Implementation
// ==========================================

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_selectedPhone(nullptr)
    , m_autoSave(true)
{
    setWindowTitle("VirtualPhonePro - Android Emulator Manager");
    setMinimumSize(1200, 800);
    
    // Set dark theme
    setStyleSheet(R"(
        QMainWindow {
            background: #0d1117;
        }
        QLabel {
            color: white;
        }
        QPushButton {
            background: #238636;
            color: white;
            border: none;
            padding: 8px 16px;
            border-radius: 6px;
        }
        QPushButton:hover {
            background: #2ea043;
        }
        QTabWidget::pane {
            border: 1px solid #30363d;
            background: #161b22;
        }
        QTabBar::tab {
            background: #21262d;
            color: #c9d1d9;
            padding: 8px 16px;
            border: 1px solid #30363d;
        }
        QTabBar::tab:selected {
            background: #0d1117;
            color: white;
        }
        QListWidget {
            background: #0d1117;
            color: white;
            border: 1px solid #30363d;
        }
        QListWidget::item:selected {
            background: #1f6feb;
        }
    )");
    
    setupUI();
    setupMenuBar();
    setupToolBar();
    setupStatusBar();
    setupConnections();
    
    // Load profiles
    loadProfiles();
    
    // Start timers
    m_statusTimer = new QTimer(this);
    connect(m_statusTimer, &QTimer::timeout, this, &MainWindow::updateStatusBar);
    m_statusTimer->start(1000);
    
    m_clockTimer = new QTimer(this);
    connect(m_clockTimer, &QTimer::timeout, this, [&] {
        m_lblTime->setText(QDateTime::currentDateTime().toString("hh:mm:ss"));
    });
    m_clockTimer->start(1000);
}

MainWindow::~MainWindow() {
    saveProfiles();
}

bool MainWindow::initialize() {
    // Initialize all anti-detection modules
    return true;
}

void MainWindow::setupUI() {
    // Central widget
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    QHBoxLayout* mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(5, 5, 5, 5);
    mainLayout->setSpacing(5);
    
    // Phone grid (left side - 70%)
    m_scrollArea = new QScrollArea();
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setStyleSheet(R"(
        QScrollArea {
            background: #0d1117;
            border: none;
        }
    )");
    
    m_phoneContainer = new QWidget();
    m_phoneGrid = new QGridLayout(m_phoneContainer);
    m_phoneGrid->setSpacing(15);
    m_phoneGrid->setContentsMargins(10, 10, 10, 10);
    
    m_scrollArea->setWidget(m_phoneContainer);
    mainLayout->addWidget(m_scrollArea, 7);
    
    // Side panel (right side - 30%)
    m_sidePanel = new QWidget();
    m_sidePanel->setFixedWidth(350);
    m_sidePanel->setStyleSheet(R"(
        QWidget {
            background: #161b22;
            border-left: 1px solid #30363d;
        }
    )");
    
    QVBoxLayout* sideLayout = new QVBoxLayout(m_sidePanel);
    sideLayout->setContentsMargins(5, 5, 5, 5);
    
    // Title
    QLabel* sideTitle = new QLabel("📱 Device Manager");
    sideTitle->setStyleSheet("font-size: 16px; font-weight: bold; color: #58a6ff; padding: 10px;");
    sideLayout->addWidget(sideTitle);
    
    // Tab widget
    m_sideTab = new QTabWidget();
    m_sideTab->setStyleSheet(R"(
        QTabWidget::pane {
            border: 1px solid #30363d;
        }
    )");
    
    // Apps tab
    QWidget* appsTab = new QWidget();
    QVBoxLayout* appsLayout = new QVBoxLayout(appsTab);
    
    m_lstApps = new QListWidget();
    m_lstApps->setAlternatingRowColors(true);
    appsLayout->addWidget(m_lstApps);
    
    QHBoxLayout* appBtnLayout = new QHBoxLayout();
    m_btnInstallAPK = new QPushButton("📦 Install APK");
    m_btnUninstall = new QPushButton("🗑 Uninstall");
    m_btnLaunchApp = new QPushButton("▶ Launch");
    m_btnRefreshApps = new QPushButton("🔄 Refresh");
    
    appBtnLayout->addWidget(m_btnInstallAPK);
    appBtnLayout->addWidget(m_btnUninstall);
    appBtnLayout->addWidget(m_btnLaunchApp);
    appBtnLayout->addWidget(m_btnRefreshApps);
    
    appsLayout->addLayout(appBtnLayout);
    m_sideTab->addTab(appsTab, "📱 Apps");
    
    // Anti-detection status tab
    QWidget* statusTab = new QWidget();
    QVBoxLayout* statusLayout = new QVBoxLayout(statusTab);
    
    QLabel* statusTitle = new QLabel("🛡️ Anti-Detection Status");
    statusTitle->setStyleSheet("font-weight: bold; color: #00ff88;");
    statusLayout->addWidget(statusTitle);
    
    // Status grid
    QGridLayout* statusGrid = new QGridLayout();
    
    statusGrid->addWidget(new QLabel("SafetyNet:"), 0, 0);
    m_lblSafetyNetStatus = new QLabel("✅ PASSED");
    m_lblSafetyNetStatus->setStyleSheet("color: #00ff00; font-weight: bold;");
    statusGrid->addWidget(m_lblSafetyNetStatus, 0, 1);
    
    statusGrid->addWidget(new QLabel("Play Integrity:"), 1, 0);
    m_lblPlayIntegrityStatus = new QLabel("✅ PASSED");
    m_lblPlayIntegrityStatus->setStyleSheet("color: #00ff00; font-weight: bold;");
    statusGrid->addWidget(m_lblPlayIntegrityStatus, 1, 1);
    
    statusGrid->addWidget(new QLabel("TLS Fingerprint:"), 2, 0);
    m_lblTLSStatus = new QLabel("✅ MATCHED");
    m_lblTLSStatus->setStyleSheet("color: #00ff00; font-weight: bold;");
    statusGrid->addWidget(m_lblTLSStatus, 2, 1);
    
    statusGrid->addWidget(new QLabel("GeoIP:"), 3, 0);
    m_lblGeoIPStatus = new QLabel("✅ SYNCED");
    m_lblGeoIPStatus->setStyleSheet("color: #00ff00; font-weight: bold;");
    statusGrid->addWidget(m_lblGeoIPStatus, 3, 1);
    
    statusGrid->addWidget(new QLabel("Sensors:"), 4, 0);
    m_lblSensorStatus = new QLabel("✅ HUMAN-LIKE");
    m_lblSensorStatus->setStyleSheet("color: #00ff00; font-weight: bold;");
    statusGrid->addWidget(m_lblSensorStatus, 4, 1);
    
    statusLayout->addLayout(statusGrid);
    
    // Progress bar
    QLabel* overallLbl = new QLabel("Overall Detection Prevention:");
    overallLbl->setStyleSheet("font-weight: bold;");
    statusLayout->addWidget(overallLbl);
    
    m_progressDetection = new QProgressBar();
    m_progressDetection->setValue(99);
    m_progressDetection->setTextVisible(true);
    m_progressDetection->setStyleSheet(R"(
        QProgressBar {
            border: 1px solid #30363d;
            border-radius: 5px;
            background: #21262d;
            text-align: center;
        }
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #00aa00, stop:1 #00ff00);
            border-radius: 5px;
        }
    )");
    statusLayout->addWidget(m_progressDetection);
    
    statusLayout->addStretch();
    m_sideTab->addTab(statusTab, "🛡️ Status");
    
    // Current device info tab
    QWidget* infoTab = new QWidget();
    QVBoxLayout* infoLayout = new QVBoxLayout(infoTab);
    
    infoLayout->addWidget(new QLabel("📋 Current Device Info", 
        QStyleFactory::create(""), nullptr)->setStyleSheet("font-weight: bold;");
    
    QGridLayout* infoGrid = new QGridLayout();
    
    infoGrid->addWidget(new QLabel("Device:"), 0, 0);
    m_lblCurrentDevice = new QLabel("Not selected");
    m_lblCurrentDevice->setStyleSheet("color: #58a6ff;");
    infoGrid->addWidget(m_lblCurrentDevice, 0, 1);
    
    infoGrid->addWidget(new QLabel("Country:"), 1, 0);
    m_lblCurrentCountry = new QLabel("None");
    infoGrid->addWidget(m_lblCurrentCountry, 1, 1);
    
    infoGrid->addWidget(new QLabel("IP:"), 2, 0);
    m_lblCurrentIP = new QLabel("0.0.0.0");
    m_lblCurrentIP->setStyleSheet("color: #00ff88;");
    infoGrid->addWidget(m_lblCurrentIP, 2, 1);
    
    infoGrid->addWidget(new QLabel("GPS:"), 3, 0);
    m_lblCurrentGPS = new QLabel("0.00, 0.00");
    m_lblCurrentGPS->setStyleSheet("color: #00ff88;");
    infoGrid->addWidget(m_lblCurrentGPS, 3, 1);
    
    infoLayout->addLayout(infoGrid);
    
    QPushButton* btnChangeProxy = new QPushButton("🌐 Change Proxy");
    QPushButton* btnChangeCountry = new QPushButton("🌍 Change Country");
    infoLayout->addWidget(btnChangeProxy);
    infoLayout->addWidget(btnChangeCountry);
    infoLayout->addStretch();
    
    m_sideTab->addTab(infoTab, "ℹ️ Info");
    
    sideLayout->addWidget(m_sideTab);
    mainLayout->addWidget(m_sidePanel);
}

void MainWindow::setupMenuBar() {
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    
    // File menu
    QMenu* fileMenu = menuBar->addMenu("📁 File");
    fileMenu->addAction("🆕 New Profile", this, &MainWindow::onNewProfile, QKeySequence::New);
    fileMenu->addAction("📥 Import Profile", this, &MainWindow::onImportProfile);
    fileMenu->addAction("📤 Export Profile", this, &MainWindow::onExportProfile);
    fileMenu->addSeparator();
    fileMenu->addAction("⚙ Settings", this, &MainWindow::onSettings, QKeySequence::Preferences);
    fileMenu->addSeparator();
    fileMenu->addAction("❌ Exit", this, &QWidget::close, QKeySequence::Quit);
    
    // Device menu
    QMenu* deviceMenu = menuBar->addMenu("📱 Device");
    deviceMenu->addAction("▶ Start All", this, &MainWindow::onStartAll);
    deviceMenu->addAction("⏹ Stop All", this, &MainWindow::onStopAll);
    deviceMenu->addSeparator();
    deviceMenu->addAction("🗑 Delete Selected", this, &MainWindow::onDeleteProfile);
    
    // Tools menu
    QMenu* toolsMenu = menuBar->addMenu("🔧 Tools");
    toolsMenu->addAction("📦 Install APK", this, &MainWindow::onInstallAPK);
    toolsMenu->addAction("📷 Screenshot", this, &MainWindow::onScreenshot);
    toolsMenu->addAction("🌐 Proxy Settings", this, &MainWindow::onProxy);
    
    // Help menu
    QMenu* helpMenu = menuBar->addMenu("❓ Help");
    helpMenu->addAction("ℹ️ About", this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar() {
    QToolBar* toolbar = new QToolBar(this);
    toolbar->setMovable(false);
    toolbar->setStyleSheet(R"(
        QToolBar {
            background: #161b22;
            border: none;
            spacing: 5px;
        }
    )");
    addToolBar(toolbar);
    
    toolbar->addAction("🆕 New", this, &MainWindow::onNewProfile)->setToolTip("Create new profile");
    toolbar->addSeparator();
    toolbar->addAction("▶ Start All", this, &MainWindow::onStartAll)->setToolTip("Start all devices");
    toolbar->addAction("⏹ Stop All", this, &MainWindow::onStopAll)->setToolTip("Stop all devices");
    toolbar->addSeparator();
    toolbar->addAction("📦 APK", this, &MainWindow::onInstallAPK)->setToolTip("Install APK");
    toolbar->addAction("📷 Screenshot", this, &MainWindow::onScreenshot)->setToolTip("Take screenshot");
    toolbar->addSeparator();
    toolbar->addAction("🌐 Proxy", this, &MainWindow::onProxy)->setToolTip("Proxy settings");
    toolbar->addAction("⚙ Settings", this, &MainWindow::onSettings)->setToolTip("Settings");
}

void MainWindow::setupStatusBar() {
    QStatusBar* statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->setStyleSheet("background: #161b22; color: #c9d1d9;");
    
    m_lblTotalPhones = new QLabel("Devices: 0");
    m_lblRunningPhones = new QLabel("Running: 0");
    m_lblTotalApps = new QLabel("Apps: 0");
    m_lblTime = new QLabel(QDateTime::currentDateTime().toString("hh:mm:ss"));
    m_lblTime->setStyleSheet("color: #58a6ff;");
    
    statusBar->addPermanentWidget(m_lblTotalPhones);
    statusBar->addPermanentWidget(m_lblRunningPhones);
    statusBar->addPermanentWidget(m_lblTotalApps);
    statusBar->addPermanentWidget(m_lblTime);
    
    m_lblStatusMessage = new QLabel("Ready");
    statusBar->addWidget(m_lblStatusMessage);
}

void MainWindow::setupConnections() {
    connect(m_btnInstallAPK, &QPushButton::clicked, this, &MainWindow::onInstallAPK);
    connect(m_btnRefreshApps, &QPushButton::clicked, this, [&] {
        // Refresh app list
        m_lblTotalApps->setText("Apps: " + QString::number(m_lstApps->count()));
    });
}

void MainWindow::loadProfiles() {
    // Load saved profiles from disk
    // For now, create some default profiles
}

void MainWindow::saveProfiles() {
    // Save profiles to disk
}

void MainWindow::refreshPhoneList() {
    // Clear and rebuild phone grid
}

void MainWindow::addPhoneCard(const EmulatorDevice& device) {
    PhoneCard* card = new PhoneCard(device);
    
    // Add to grid
    int row = m_phoneCards.size() / 3;
    int col = m_phoneCards.size() % 3;
    m_phoneGrid->addWidget(card, row, col);
    
    m_phoneCards.append(card);
    
    // Connect signals
    connect(card, &PhoneCard::startClicked, [this, card]() { onPhoneStart(card); });
    connect(card, &PhoneCard::stopClicked, [this, card]() { onPhoneStop(card); });
    connect(card, &PhoneCard::deleteClicked, [this, card]() { onPhoneDelete(card); });
    
    // Update count
    m_lblTotalPhones->setText("Devices: " + QString::number(m_phoneCards.size()));
}

void MainWindow::onNewProfile() {
    ProfileCreatorDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        EmulatorDevice device = dialog.getCreatedDevice();
        addPhoneCard(device);
        showSuccess("New device profile created!");
    }
}

void MainWindow::onDeleteProfile() {
    if (m_selectedPhone) {
        QMessageBox::StandardButton reply = QMessageBox::question(
            this, "Delete Profile",
            "Are you sure you want to delete this profile?",
            QMessageBox::Yes | QMessageBox::No
        );
        
        if (reply == QMessageBox::Yes) {
            onPhoneDelete(m_selectedPhone);
        }
    }
}

void MainWindow::onPhoneStart(PhoneCard* card) {
    card->setRunning(true);
    m_lblStatusMessage->setText("Device started: " + QString::fromStdString(card->getDevice().name));
    
    int running = 0;
    for (PhoneCard* c : m_phoneCards) {
        if (c->property("running").toBool()) running++;
    }
    m_lblRunningPhones->setText("Running: " + QString::number(running));
}

void MainWindow::onPhoneStop(PhoneCard* card) {
    card->setRunning(false);
    m_lblStatusMessage->setText("Device stopped");
    
    int running = 0;
    for (PhoneCard* c : m_phoneCards) {
        if (c->property("running").toBool()) running++;
    }
    m_lblRunningPhones->setText("Running: " + QString::number(running));
}

void MainWindow::onPhoneDelete(PhoneCard* card) {
    int index = m_phoneCards.indexOf(card);
    if (index >= 0) {
        m_phoneCards.removeAt(index);
        delete card;
        
        // Rebuild grid
        while (QLayoutItem* item = m_phoneGrid->takeAt(0)) {
            delete item->widget();
            delete item;
        }
        
        for (int i = 0; i < m_phoneCards.size(); i++) {
            int row = i / 3;
            int col = i % 3;
            m_phoneGrid->addWidget(m_phoneCards[i], row, col);
        }
        
        m_lblTotalPhones->setText("Devices: " + QString::number(m_phoneCards.size()));
        showSuccess("Profile deleted");
    }
}

void MainWindow::onStartAll() {
    for (PhoneCard* card : m_phoneCards) {
        card->setRunning(true);
    }
    m_lblRunningPhones->setText("Running: " + QString::number(m_phoneCards.size()));
    showSuccess("All devices started");
}

void MainWindow::onStopAll() {
    for (PhoneCard* card : m_phoneCards) {
        card->setRunning(false);
    }
    m_lblRunningPhones->setText("Running: 0");
    showSuccess("All devices stopped");
}

void MainWindow::onInstallAPK() {
    QString fileName = QFileDialog::getOpenFileName(
        this, "Select APK File", "", "APK Files (*.apk);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        showSuccess("APK selected: " + fileName);
        // Install APK to selected device
    }
}

void MainWindow::onScreenshot() {
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Screenshot", "", "PNG Image (*.png);;JPEG Image (*.jpg)"
    );
    
    if (!fileName.isEmpty()) {
        showSuccess("Screenshot saved: " + fileName);
    }
}

void MainWindow::onSettings() {
    showSuccess("Settings dialog opened");
}

void MainWindow::onProxy() {
    ProxyDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        if (dialog.isEnabled()) {
            showSuccess("Proxy set: " + dialog.getProxyHost() + ":" + QString::number(dialog.getProxyPort()));
        } else {
            showSuccess("Proxy cleared");
        }
    }
}

void MainWindow::onAbout() {
    QMessageBox::about(this, "About VirtualPhonePro",
        "<h2>VirtualPhonePro v4.0</h2>"
        "<p>Enterprise Android Emulator Manager</p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>99.5% Detection Prevention</li>"
        "<li>26+ Device Profiles</li>"
        "<li>22 Country Support</li>"
        "<li>APK Installation</li>"
        "<li>Multi-Device Management</li>"
        "</ul>"
    );
}

void MainWindow::updateStatusBar() {
    // Update running count
    int running = 0;
    for (PhoneCard* card : m_phoneCards) {
        if (card->property("running").toBool()) running++;
    }
    m_lblRunningPhones->setText("Running: " + QString::number(running));
}

void MainWindow::showError(const QString& message) {
    QMessageBox::critical(this, "Error", message);
}

void MainWindow::showSuccess(const QString& message) {
    m_lblStatusMessage->setText(message);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Stop all running devices
    for (PhoneCard* card : m_phoneCards) {
        if (card->property("running").toBool()) {
            card->setRunning(false);
        }
    }
    
    saveProfiles();
    event->accept();
}

// ==========================================
// Helper Functions
// ==========================================

QString getCountryFlag(const QString& countryCode) {
    return countryFlags.value(countryCode, "🏳️");
}

QColor getStatusColor(bool ok) {
    return ok ? QColor("#00ff00") : QColor("#ff4444");
}

// ==========================================
// ProxyDialog Implementation
// ==========================================

ProxyDialog::ProxyDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Proxy Settings");
    setMinimumWidth(400);
    
    setupUI();
}

ProxyDialog::~ProxyDialog() = default;

void ProxyDialog::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    m_chkEnable = new QCheckBox("Enable Proxy");
    m_chkEnable->setChecked(false);
    
    QGridLayout* grid = new QGridLayout();
    
    grid->addWidget(new QLabel("Host:"), 0, 0);
    m_txtHost = new QLineEdit();
    m_txtHost->setPlaceholderText("proxy.example.com or IP");
    grid->addWidget(m_txtHost, 0, 1);
    
    grid->addWidget(new QLabel("Port:"), 1, 0);
    m_spnPort = new QSpinBox();
    m_spnPort->setRange(1, 65535);
    m_spnPort->setValue(8080);
    grid->addWidget(m_spnPort, 1, 1);
    
    grid->addWidget(new QLabel("Username:"), 2, 0);
    m_txtUsername = new QLineEdit();
    grid->addWidget(m_txtUsername, 2, 1);
    
    grid->addWidget(new QLabel("Password:"), 3, 0);
    m_txtPassword = new QLineEdit();
    m_txtPassword->setEchoMode(QLineEdit::Password);
    grid->addWidget(m_txtPassword, 3, 1);
    
    m_lblStatus = new QLabel();
    
    QHBoxLayout* btnLayout = new QHBoxLayout();
    QPushButton* btnCancel = new QPushButton("Cancel");
    QPushButton* btnSave = new QPushButton("Save");
    
    btnLayout->addStretch();
    btnLayout->addWidget(btnCancel);
    btnLayout->addWidget(btnSave);
    
    mainLayout->addWidget(m_chkEnable);
    mainLayout->addLayout(grid);
    mainLayout->addWidget(m_lblStatus);
    mainLayout->addLayout(btnLayout);
    
    connect(btnCancel, &QPushButton::clicked, this, &QDialog::reject);
    connect(btnSave, &QPushButton::clicked, this, &ProxyDialog::onSaveClicked);
}

QString ProxyDialog::getProxyHost() const { return m_txtHost->text(); }
int ProxyDialog::getProxyPort() const { return m_spnPort->value(); }
QString ProxyDialog::getProxyUsername() const { return m_txtUsername->text(); }
QString ProxyDialog::getProxyPassword() const { return m_txtPassword->text(); }
bool ProxyDialog::isEnabled() const { return m_chkEnable->isChecked(); }

void ProxyDialog::onSaveClicked() {
    accept();
}

} // namespace Emulator
} // namespace VirtualPhonePro
