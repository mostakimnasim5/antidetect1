#pragma once

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTableWidget>
#include <QLabel>
#include <QMenuBar>
#include <QStatusBar>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onCreateVM();
    void onStartVM();
    void onStopVM();
    void onDeleteVM();
    void onRefreshVMs();
    void onGenerateProfile();
    void onApplyProfile();

private:
    void setupUI();
    void setupMenuBar();
    void setupConnections();
    void refreshVMList();
    void showStatus(const QString& message);

    QTabWidget *tabWidget;
    QTableWidget *vmTable;
    QLabel *statusLabel;
};
