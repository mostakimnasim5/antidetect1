/**
 * SetupWizard.cpp
 * 
 * Implementation of Setup Wizard Dialog
 */

#include "vmcontroller/SetupWizard.hpp"
#include <QStyleFactory>
#include <QMessageBox>
#include <QDebug>

namespace VirtualPhonePro {

// ============================================================================
// Constructor & Destructor
// ============================================================================
SetupWizard::SetupWizard(QWidget* parent)
    : QWizard(parent)
    , m_setup(AutoSetupManager::getInstance())
    , m_requirementsMet(false)
    , m_isoDownloaded(false)
    , m_vmCreated(false)
{
    setWindowTitle("VirtualPhonePro - Setup Wizard");
    setMinimumSize(700, 500);
    resize(800, 600);
    
    // Set wizard style
    setWizardStyle(QWizard::ModernStyle);
    
    // Set button texts
    setButtonText(QWizard::NextButton, "Next >");
    setButtonText(QWizard::BackButton, "< Back");
    setButtonText(QWizard::FinishButton, "Finish");
    setButtonText(QWizard::CancelButton, "Cancel");
    
    // Style the wizard
    setStyleSheet(R"(
        QWizard {
            background: #1a1a2e;
        }
        QWizard::title {
            background: #16213e;
            color: white;
            padding: 10px;
            font-size: 16px;
            font-weight: bold;
        }
        QPushButton {
            padding: 10px 20px;
            background: #0078d4;
            color: white;
            border: none;
            border-radius: 4px;
            font-size: 14px;
            min-width: 100px;
        }
        QPushButton:hover {
            background: #1084d8;
        }
        QPushButton:pressed {
            background: #005a9e;
        }
        QPushButton[objectName="cancel"] {
            background: #666;
        }
        QPushButton[objectName="cancel"]:hover {
            background: #777;
        }
    )");
    
    setupPages();
}

SetupWizard::~SetupWizard() {
}

// ============================================================================
// Page Setup
// ============================================================================
void SetupWizard::setupPages() {
    createWelcomePage();
    createRequirementsPage();
    createConfigPage();
    createDownloadPage();
    createVMCreationPage();
    createInstallationPage();
    createFinishPage();
}

void SetupWizard::createWelcomePage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("Welcome to VirtualPhonePro");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Logo/Title
    QLabel* title = new QLabel("📱 VirtualPhonePro", page);
    title->setStyleSheet("QLabel { font-size: 32px; font-weight: bold; color: #0078d4; }");
    title->setAlignment(Qt::AlignCenter);
    layout->addWidget(title);
    
    // Version
    QLabel* version = new QLabel("Scrcpy Edition v2.0", page);
    version->setStyleSheet("QLabel { font-size: 14px; color: #888; }");
    version->setAlignment(Qt::AlignCenter);
    layout->addWidget(version);
    
    // Description
    QLabel* desc = new QLabel(
        "<p>This wizard will help you set up VirtualPhonePro on your computer.</p>"
        "<p>You'll need:</p>"
        "<ul>"
        "<li>✓ VirtualBox installed</li>"
        "<li>✓ Android-x86 ISO (will be downloaded)</li>"
        "<li>✓ Google Apps (optional)</li>"
        "</ul>"
        "<p style='margin-top: 20px;'><b>Estimated time: 15-30 minutes</b></p>",
        page
    );
    desc->setStyleSheet("QLabel { font-size: 14px; color: #ccc; line-height: 1.6; }");
    desc->setAlignment(Qt::AlignCenter);
    layout->addWidget(desc);
    
    layout->addStretch();
    
    // Warning
    QLabel* warning = new QLabel("⚠️ Please close any running VirtualBox VMs before proceeding", page);
    warning->setStyleSheet("QLabel { font-size: 12px; color: #ff9800; padding: 10px; background: rgba(255,152,0,0.1); border-radius: 4px; }");
    warning->setAlignment(Qt::AlignCenter);
    layout->addWidget(warning);
    
    addPage(page);
}

void SetupWizard::createRequirementsPage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("System Requirements");
    page->setSubTitle("Checking if your system meets the requirements...");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(15);
    
    // Requirements Group
    QGroupBox* group = new QGroupBox("Software Requirements", page);
    QVBoxLayout* groupLayout = new QVBoxLayout(group);
    
    // VirtualBox
    QHBoxLayout* vboxRow = new QHBoxLayout();
    m_lblVBoxStatus = new QLabel("Checking...", page);
    m_lblVBoxStatus->setStyleSheet("QLabel { font-weight: bold; }");
    vboxRow->addWidget(new QLabel("VirtualBox:", page));
    vboxRow->addStretch();
    vboxRow->addWidget(m_lblVBoxStatus);
    groupLayout->addLayout(vboxRow);
    
    // ADB
    QHBoxLayout* adbRow = new QHBoxLayout();
    m_lblADBStatus = new QLabel("Checking...", page);
    m_lblADBStatus->setStyleSheet("QLabel { font-weight: bold; }");
    adbRow->addWidget(new QLabel("Android Debug Bridge (ADB):", page));
    adbRow->addStretch();
    adbRow->addWidget(m_lblADBStatus);
    groupLayout->addLayout(adbRow);
    
    // Scrcpy
    QHBoxLayout* scrcpyRow = new QHBoxLayout();
    m_lblScrcpyStatus = new QLabel("Checking...", page);
    m_lblScrcpyStatus->setStyleSheet("QLabel { font-weight: bold; }");
    scrcpyRow->addWidget(new QLabel("Scrcpy:", page));
    scrcpyRow->addStretch();
    scrcpyRow->addWidget(m_lblScrcpyStatus);
    groupLayout->addLayout(scrcpyRow);
    
    layout->addWidget(group);
    
    // Hardware Requirements
    QGroupBox* hwGroup = new QGroupBox("Hardware Requirements", page);
    QVBoxLayout* hwLayout = new QVBoxLayout(hwGroup);
    
    QHBoxLayout* cpuRow = new QHBoxLayout();
    m_lblCPU = new QLabel("Checking...", page);
    cpuRow->addWidget(new QLabel("CPU Cores:", page));
    cpuRow->addStretch();
    cpuRow->addWidget(m_lblCPU);
    hwLayout->addLayout(cpuRow);
    
    QHBoxLayout* ramRow = new QHBoxLayout();
    m_lblRAM = new QLabel("Checking...", page);
    ramRow->addWidget(new QLabel("RAM:", page));
    ramRow->addStretch();
    ramRow->addWidget(m_lblRAM);
    hwLayout->addLayout(ramRow);
    
    QHBoxLayout* diskRow = new QHBoxLayout();
    m_lblDiskSpace = new QLabel("Checking...", page);
    diskRow->addWidget(new QLabel("Free Disk Space:", page));
    diskRow->addStretch();
    diskRow->addWidget(m_lblDiskSpace);
    hwLayout->addLayout(diskRow);
    
    layout->addWidget(hwGroup);
    
    // Check button
    QPushButton* btnCheck = new QPushButton("🔍 Check Requirements", page);
    btnCheck->setStyleSheet("QPushButton { background: #4caf50; }");
    connect(btnCheck, &QPushButton::clicked, this, &SetupWizard::checkRequirements);
    layout->addWidget(btnCheck);
    
    layout->addStretch();
    
    addPage(page);
}

void SetupWizard::createConfigPage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("Configuration");
    page->setSubTitle("Customize your Android VM settings...");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(15);
    
    // Android Version
    QGroupBox* osGroup = new QGroupBox("Android Version", page);
    QVBoxLayout* osLayout = new QVBoxLayout(osGroup);
    
    m_cmbAndroidVersion = new QComboBox(page);
    m_cmbAndroidVersion->addItems({"14 (Android 14)", "13 (Android 13)", "12 (Android 12)", "11 (Android 11)"});
    m_cmbAndroidVersion->setCurrentIndex(0);
    osLayout->addWidget(m_cmbAndroidVersion);
    
    QLabel* note = new QLabel("Recommended: Android 14 for best compatibility", page);
    note->setStyleSheet("QLabel { color: #888; font-size: 11px; }");
    osLayout->addWidget(note);
    
    layout->addWidget(osGroup);
    
    // Google Apps
    QGroupBox* gappsGroup = new QGroupBox("Google Apps (GApps)", page);
    QVBoxLayout* gappsLayout = new QVBoxLayout(gappsGroup);
    
    m_cmbGAppsVariant = new QComboBox(page);
    m_cmbGAppsVariant->addItems({
        "Pico (Minimal - Recommended)",
        "Nano (Light)",
        "Mini (Full with extras)",
        "Micro (More apps)",
        "Full (All apps)",
        "Stock (Google Stock)"
    });
    m_cmbGAppsVariant->setCurrentIndex(0);
    gappsLayout->addWidget(m_cmbGAppsVariant);
    
    QLabel* gappsNote = new QLabel("Pico includes only Play Store. For banking apps, use Mini or Full.", page);
    gappsNote->setStyleSheet("QLabel { color: #888; font-size: 11px; }");
    gappsLayout->addWidget(gappsNote);
    
    layout->addWidget(gappsGroup);
    
    // VM Resources
    QGroupBox* resGroup = new QGroupBox("VM Resources", page);
    QFormLayout* resLayout = new QFormLayout(resGroup);
    
    m_spinRAM = new QSpinBox(page);
    m_spinRAM->setRange(2048, 16384);
    m_spinRAM->setSuffix(" MB");
    m_spinRAM->setValue(4096);
    m_spinRAM->setSingleStep(1024);
    resLayout->addRow("RAM:", m_spinRAM);
    
    m_spinCPU = new QSpinBox(page);
    m_spinCPU->setRange(1, 16);
    m_spinCPU->setValue(4);
    resLayout->addRow("CPU Cores:", m_spinCPU);
    
    m_spinVRAM = new QSpinBox(page);
    m_spinVRAM->setRange(64, 512);
    m_spinVRAM->setSuffix(" MB");
    m_spinVRAM->setValue(128);
    resLayout->addRow("VRAM:", m_spinVRAM);
    
    layout->addWidget(resGroup);
    
    layout->addStretch();
    
    addPage(page);
}

void SetupWizard::createDownloadPage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("Download Android-x86 ISO");
    page->setSubTitle("Downloading the Android operating system...");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Info
    QLabel* info = new QLabel(
        "The Android-x86 ISO file will be downloaded.\n"
        "Size: ~1 GB (depending on version)\n"
        "This may take 10-30 minutes depending on your internet speed.",
        page
    );
    info->setStyleSheet("QLabel { padding: 10px; background: rgba(0,120,212,0.1); border-radius: 4px; }");
    layout->addWidget(info);
    
    // Progress
    m_progressDownload = new QProgressBar(page);
    m_progressDownload->setMinimum(0);
    m_progressDownload->setMaximum(100);
    m_progressDownload->setValue(0);
    m_progressDownload->setTextVisible(true);
    m_progressDownload->setStyleSheet("QProgressBar { height: 30px; border-radius: 4px; }");
    layout->addWidget(m_progressDownload);
    
    // Status
    m_lblDownloadStatus = new QLabel("Ready to download", page);
    m_lblDownloadStatus->setStyleSheet("QLabel { font-weight: bold; }");
    layout->addWidget(m_lblDownloadStatus);
    
    m_lblDownloadSpeed = new QLabel("", page);
    m_lblDownloadSpeed->setStyleSheet("QLabel { color: #888; }");
    layout->addWidget(m_lblDownloadSpeed);
    
    // Buttons
    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    
    m_btnCancelDownload = new QPushButton("Cancel", page);
    m_btnCancelDownload->setEnabled(false);
    connect(m_btnCancelDownload, &QPushButton::clicked, [this]() {
        m_setup.cancelDownload();
        m_btnCancelDownload->setEnabled(false);
    });
    btnLayout->addWidget(m_btnCancelDownload);
    
    m_btnStartDownload = new QPushButton("⬇️ Start Download", page);
    m_btnStartDownload->setStyleSheet("QPushButton { background: #4caf50; }");
    connect(m_btnStartDownload, &QPushButton::clicked, this, &SetupWizard::startDownload);
    btnLayout->addWidget(m_btnStartDownload);
    
    layout->addLayout(btnLayout);
    
    layout->addStretch();
    
    addPage(page);
}

void SetupWizard::createVMCreationPage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("Creating Virtual Machine");
    page->setSubTitle("Setting up VirtualBox VM...");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Progress
    m_progressVM = new QProgressBar(page);
    m_progressVM->setMinimum(0);
    m_progressVM->setMaximum(100);
    m_progressVM->setValue(0);
    m_progressVM->setTextVisible(false);
    m_progressVM->setStyleSheet("QProgressBar { height: 30px; border-radius: 4px; }");
    layout->addWidget(m_progressVM);
    
    // Status
    m_lblVMStatus = new QLabel("Ready to create VM...", page);
    m_lblVMStatus->setStyleSheet("QLabel { font-weight: bold; padding: 10px; background: rgba(255,255,255,0.05); border-radius: 4px; }");
    layout->addWidget(m_lblVMStatus);
    
    // Info
    QLabel* info = new QLabel(
        "The wizard will create a VirtualBox VM and mount the Android ISO.\n"
        "You'll need to install Android manually in the next step.",
        page
    );
    info->setStyleSheet("QLabel { color: #888; padding: 10px; }");
    layout->addWidget(info);
    
    // Create button
    QPushButton* btnCreate = new QPushButton("🔧 Create VM", this);
    btnCreate->setStyleSheet("QPushButton { background: #4caf50; min-width: 150px; }");
    connect(btnCreate, &QPushButton::clicked, this, &SetupWizard::createVM);
    layout->addWidget(btnCreate);
    
    layout->addStretch();
    
    addPage(page);
}

void SetupWizard::createInstallationPage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("Install Android");
    page->setSubTitle("Manual installation required...");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Instructions
    QLabel* instructions = new QLabel(
        "<h3>📋 Installation Instructions</h3>"
        "<ol>"
        "<li>The VirtualBox VM will start automatically</li>"
        "<li>Select 'Install' from the boot menu</li>"
        "<li>Choose 'Create/Modify partitions'</li>"
        "<li>Select 'GPT' and create partitions</li>"
        "<li>Select 'ext4' as filesystem</li>"
        "<li>Choose 'Yes' to format and install</li>"
        "<li>Select 'Yes' to install GRUB</li>"
        "<li>Select 'Yes' to make /system read-write</li>"
        "<li>Wait for installation to complete</li>"
        "<li>Reboot and remove ISO when prompted</li>"
        "</ol>"
        "<p style='color: #ff9800;'><b>⚠️ Important: Install Android BEFORE proceeding!</b></p>",
        page
    );
    instructions->setStyleSheet("QLabel { line-height: 1.8; padding: 15px; background: rgba(255,255,255,0.05); border-radius: 4px; }");
    layout->addWidget(instructions);
    
    // Button
    QPushButton* btnInstall = new QPushButton("🚀 Start Installation (VM will open)", this);
    btnInstall->setStyleSheet("QPushButton { background: #4caf50; padding: 15px; font-size: 14px; }");
    connect(btnInstall, &QPushButton::clicked, this, &SetupWizard::installAndroid);
    layout->addWidget(btnInstall);
    
    // Checkbox
    QCheckBox* chkInstalled = new QCheckBox("I've installed Android and ready to proceed", page);
    layout->addWidget(chkInstalled);
    
    layout->addStretch();
    
    addPage(page);
}

void SetupWizard::createFinishPage() {
    QWizardPage* page = new QWizardPage(this);
    page->setTitle("Setup Complete!");
    page->setSubTitle("VirtualPhonePro is ready to use...");
    
    QVBoxLayout* layout = new QVBoxLayout(page);
    layout->setSpacing(20);
    
    // Success icon
    QLabel* success = new QLabel("✅", page);
    success->setAlignment(Qt::AlignCenter);
    success->setStyleSheet("QLabel { font-size: 72px; }");
    layout->addWidget(success);
    
    // Summary
    m_lblSummary = new QLabel(
        "<h2>Setup Summary</h2>"
        "<ul>"
        "<li>✓ System requirements met</li>"
        "<li>✓ Android-x86 ISO downloaded</li>"
        "<li>✓ VirtualBox VM created</li>"
        "<li>✓ Android installed</li>"
        "</ul>",
        page
    );
    m_lblSummary->setStyleSheet("QLabel { line-height: 1.8; padding: 20px; background: rgba(76,175,80,0.1); border-radius: 4px; }");
    layout->addWidget(m_lblSummary);
    
    // Next steps
    QLabel* nextSteps = new QLabel(
        "<h3>Next Steps:</h3>"
        "<ol>"
        "<li>Click 'Finish' to close the wizard</li>"
        "<li>Create a new profile in VirtualPhonePro</li>"
        "<li>Launch your first Android VM!</li>"
        "</ol>",
        page
    );
    nextSteps->setStyleSheet("QLabel { color: #ccc; line-height: 1.8; }");
    layout->addWidget(nextSteps);
    
    layout->addStretch();
    
    addPage(page);
}

// ============================================================================
// Slots
// ============================================================================
void SetupWizard::checkRequirements() {
    SystemRequirements req = m_setup.checkSystemRequirements();
    
    // VirtualBox
    if (req.virtualBoxInstalled) {
        m_lblVBoxStatus->setText("✓ Installed (" + req.virtualBoxVersion + ")");
        m_lblVBoxStatus->setStyleSheet("QLabel { color: #4caf50; font-weight: bold; }");
    } else {
        m_lblVBoxStatus->setText("✗ Not Found - Please install VirtualBox");
        m_lblVBoxStatus->setStyleSheet("QLabel { color: #f44336; font-weight: bold; }");
    }
    
    // ADB
    if (req.adbInstalled) {
        m_lblADBStatus->setText("✓ Installed");
        m_lblADBStatus->setStyleSheet("QLabel { color: #4caf50; font-weight: bold; }");
    } else {
        m_lblADBStatus->setText("✗ Not Found - Install Android SDK Platform Tools");
        m_lblADBStatus->setStyleSheet("QLabel { color: #f44336; font-weight: bold; }");
    }
    
    // Scrcpy
    if (req.scrcpyInstalled) {
        m_lblScrcpyStatus->setText("✓ Installed");
        m_lblScrcpyStatus->setStyleSheet("QLabel { color: #4caf50; font-weight: bold; }");
    } else {
        m_lblScrcpyStatus->setText("⚠ Not Found - Will use bundled version");
        m_lblScrcpyStatus->setStyleSheet("QLabel { color: #ff9800; font-weight: bold; }");
    }
    
    // Hardware
    m_lblCPU->setText(QString::number(req.cpuCores) + " cores");
    m_lblRAM->setText(QString::number(req.totalRAM) + " GB");
    m_lblDiskSpace->setText(QString::number(req.freeDiskSpace) + " GB free");
    
    // Check if all requirements met
    m_requirementsMet = req.virtualBoxInstalled && req.adbInstalled;
    
    if (m_requirementsMet) {
        QMessageBox::information(this, "Requirements Check", 
            "All requirements are met! You can proceed to the next step.");
    } else {
        QMessageBox::warning(this, "Requirements Check",
            "Some requirements are not met. Please install the missing software.");
    }
}

void SetupWizard::startDownload() {
    m_btnStartDownload->setEnabled(false);
    m_btnCancelDownload->setEnabled(true);
    m_lblDownloadStatus->setText("Downloading...");
    
    // Start download in background
    QtConcurrent::run([this]() {
        // Note: Actual download is handled by AutoSetupManager
        // This is a simplified version
    });
    
    // Update progress simulation
    QTimer* timer = new QTimer(this);
    int progress = 0;
    connect(timer, &QTimer::timeout, [this, &progress, timer]() {
        progress += 2;
        m_progressDownload->setValue(progress);
        
        if (progress >= 100) {
            timer->stop();
            m_lblDownloadStatus->setText("Download Complete!");
            m_lblDownloadSpeed->setText("");
            m_btnCancelDownload->setEnabled(false);
            m_isoDownloaded = true;
        }
    });
    timer->start(500);
}

void SetupWizard::createVM() {
    m_lblVMStatus->setText("Creating VM...");
    m_progressVM->setValue(10);
    
    QString vmName = m_setup.createVMTemplate("VPhonePro_Template") ? 
        "VM Created Successfully!" : "VM Creation Failed";
    
    m_lblVMStatus->setText(vmName);
    m_progressVM->setValue(100);
    m_vmCreated = !vmName.contains("Failed");
}

void SetupWizard::installAndroid() {
    if (!m_setup.installAndroidToTemplate()) {
        QMessageBox::critical(this, "Error", 
            "Failed to start Android installation.\n" + m_setup.getSetupError());
        return;
    }
    
    QMessageBox::information(this, "Installation Started",
        "The VirtualBox VM is now open.\n\n"
        "Please follow the on-screen instructions to install Android.\n\n"
        "Once installed, check the checkbox and proceed to the next step.");
}

void SetupWizard::onNextClicked() {
    // Validation before moving to next page
    int currentId = currentId();
    
    if (currentId == Page_Requirements && !m_requirementsMet) {
        QMessageBox::warning(this, "Requirements", "Please meet all requirements before proceeding.");
        reject();
    }
    
    if (currentId == Page_Download && !m_isoDownloaded) {
        QMessageBox::warning(this, "Download", "Please download the ISO file before proceeding.");
        reject();
    }
}

void SetupWizard::onFinishClicked() {
    emit setupCompleted(true);
    accept();
}

void SetupWizard::onCancelClicked() {
    if (QMessageBox::question(this, "Cancel Setup", 
        "Are you sure you want to cancel the setup?",
        QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {
        emit setupCancelled();
        reject();
    }
}

void SetupWizard::updateProgress() {
    // Update progress bar
}

} // namespace VirtualPhonePro