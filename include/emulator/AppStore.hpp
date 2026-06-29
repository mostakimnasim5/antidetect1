#pragma once

/**
 * AppStore - APK Installation and Management Module
 * 
 * Handles:
 * - APK installation via ADB
 * - App listing and management
 * - App data management
 * - Package info extraction
 */

#include "AndroidEmulator.hpp"
#include <QtCore>
#include <QObject>

#if defined(_WIN32) || defined(_WIN64)
#include <QtCore/QProcess>
#endif

namespace VirtualPhonePro {
namespace Emulator {

class AppStore : public QObject {
    Q_OBJECT

public:
    explicit AppStore(QObject* parent = nullptr);
    ~AppStore();

    // Initialize with ADB path
    bool initialize(const QString& adbPath);
    
    // APK management
    bool installAPK(const QString& deviceId, const QString& apkPath);
    bool uninstallApp(const QString& deviceId, const QString& packageName);
    bool launchApp(const QString& deviceId, const QString& packageName);
    
    // Get installed apps
    QList<InstalledApp> getInstalledApps(const QString& deviceId);
    
    // App info
    QString getAppName(const QString& apkPath);
    QString getPackageName(const QString& apkPath);
    QIcon getAppIcon(const QString& apkPath);
    
    // Data management
    bool clearAppData(const QString& deviceId, const QString& packageName);
    bool backupAppData(const QString& deviceId, const QString& packageName, const QString& backupPath);
    bool restoreAppData(const QString& deviceId, const QString& packageName, const QString& backupPath);
    
    // Permissions
    bool grantPermission(const QString& deviceId, const QString& packageName, const QString& permission);
    bool revokePermission(const QString& deviceId, const QString& packageName, const QString& permission);
    
signals:
    void installProgress(int percent);
    void installCompleted(const QString& packageName, bool success);
    void uninstallCompleted(const QString& packageName, bool success);

private slots:
    void onADBOutput();
    void onADBError();

private:
    QString executeADBCommand(const QStringList& args);
    bool waitForDevice(const QString& deviceId, int timeoutMs = 30000);
    
    QString m_adbPath;
    QString m_currentDeviceId;
    QProcess* m_adbProcess;
};

} // namespace Emulator
} // namespace VirtualPhonePro
