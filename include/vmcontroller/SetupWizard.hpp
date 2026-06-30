/**
 * SetupWizard.hpp
 * 
 * Setup Wizard Dialog for VirtualPhonePro
 * Guides users through system requirements check, ISO download, and VM setup
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include "AutoSetupManager.hpp"

#include <QDialog>
#include <QWizard>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>
#include <QComboBox>
#include <QSpinBox>
#include <QLineEdit>
#include <QGroupBox>
#include <QTableWidget>
#include <QMovie>
#include <QTimer>

namespace VirtualPhonePro {

class SetupWizard : public QWizard {
    Q_OBJECT

public:
    explicit SetupWizard(QWidget* parent = nullptr);
    ~SetupWizard();

signals:
    void setupStarted();
    void setupCompleted(bool success);
    void setupCancelled();

private slots:
    void onNextClicked();
    void onBackClicked();
    void onCancelClicked();
    void onFinishClicked();
    void updateProgress();
    void checkRequirements();
    void startDownload();
    void createVM();
    void installAndroid();

private:
    void setupPages();
    void createWelcomePage();
    void createRequirementsPage();
    void createConfigPage();
    void createDownloadPage();
    void createVMCreationPage();
    void createInstallationPage();
    void createFinishPage();

    // Page IDs
    enum {
        Page_Welcome = 0,
        Page_Requirements = 1,
        Page_Config = 2,
        Page_Download = 3,
        Page_VMCreation = 4,
        Page_Installation = 5,
        Page_Finish = 6
    };

    AutoSetupManager& m_setup;
    
    // UI Elements
    QLabel* m_lblVBoxStatus;
    QLabel* m_lblADBStatus;
    QLabel* m_lblScrcpyStatus;
    QLabel* m_lblDiskSpace;
    QLabel* m_lblRAM;
    QLabel* m_lblCPU;
    
    QComboBox* m_cmbAndroidVersion;
    QComboBox* m_cmbGAppsVariant;
    QSpinBox* m_spinRAM;
    QSpinBox* m_spinCPU;
    QSpinBox* m_spinVRAM;
    
    QProgressBar* m_progressDownload;
    QLabel* m_lblDownloadStatus;
    QLabel* m_lblDownloadSpeed;
    QPushButton* m_btnStartDownload;
    QPushButton* m_btnCancelDownload;
    
    QProgressBar* m_progressVM;
    QLabel* m_lblVMStatus;
    
    QLabel* m_lblSummary;
    
    // State
    bool m_requirementsMet;
    bool m_isoDownloaded;
    bool m_vmCreated;
};

} // namespace VirtualPhonePro