/**
 * ADBSpoofer.hpp
 * 
 * ADB-based Identity Spoofing for VirtualPhonePro
 * Handles GPS, proxy, timezone, language, and hardware spoofing
 */

#pragma once

#include "../VirtualPhonePro.hpp"
#include <QObject>
#include <QString>
#include <QProcess>
#include <QMap>
#include <QVariant>
#include <mutex>

namespace VirtualPhonePro {

struct SpoofConfig {
    // GPS Location
    double latitude;
    double longitude;
    double altitude;
    double accuracy;
    
    // Network
    QString proxyHost;
    int proxyPort;
    bool useProxy;
    
    // Timezone & Locale
    QString timezone;
    QString language;
    QString country;
    QString region;
    
    // Network IP (for WebRTC/local detection)
    QString localIP;
    QString publicIP;
    
    // Hardware Identity
    QString androidId;
    QString gsfId;
    QString imei;
    QString macAddress;
    QString wifiMac;
    QString bluetoothMac;
    
    // Build Properties
    QString manufacturer;
    QString model;
    QString brand;
    QString device;
    QString product;
    QString board;
    QString hardware;
    QString buildFingerprint;
    QString bootloader;
    QString buildId;
    QString securityPatch;
    QString osVersion;
    int apiLevel;
    
    // Carrier
    QString carrierName;
    QString carrierCountry;
    QString simOperator;
    QString networkOperator;
    
    // Display
    int screenWidth;
    int screenHeight;
    int screenDPI;
    QString screenDensity;
    
    // Audio
    QString audioCodec;
    QString audioBitrate;
};

class ADBSpoofer : public QObject {
    Q_OBJECT

public:
    static ADBSpoofer& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // Core Spoofing
    bool applySpoofToDevice(const QString& deviceSerial, const SpoofConfig& config);
    bool clearSpoofFromDevice(const QString& deviceSerial);
    bool isSpoofApplied(const QString& deviceSerial);
    
    // Individual Spoofing Methods
    bool setGPSLocation(const QString& deviceSerial, double lat, double lon, double alt = 0);
    bool setTimezone(const QString& deviceSerial, const QString& timezone);
    bool setLanguage(const QString& deviceSerial, const QString& language, const QString& country);
    bool setProxy(const QString& deviceSerial, const QString& host, int port);
    bool clearProxy(const QString& deviceSerial);
    
    // Hardware Spoofing
    bool setAndroidId(const QString& deviceSerial, const QString& androidId);
    bool setGSFId(const QString& deviceSerial, const QString& gsfId);
    bool setDeviceInfo(const QString& deviceSerial, 
                       const QString& manufacturer,
                       const QString& model,
                       const QString& brand);
    bool setBuildFingerprint(const QString& deviceSerial, const QString& fingerprint);
    bool setBootloader(const QString& deviceSerial, const QString& bootloader);
    
    // Network Spoofing
    bool setMACAddress(const QString& deviceSerial, const QString& macAddress);
    bool setCarrierInfo(const QString& deviceSerial,
                        const QString& carrierName,
                        const QString& country,
                        const QString& mcc,
                        const QString& mnc);
    
    // Display Spoofing
    bool setScreenProperties(const QString& deviceSerial, 
                             int width, int height, int dpi);
    
    // Advanced Spoofing
    bool disablePlayServicesVerification(const QString& deviceSerial);
    bool enableMockLocation(const QString& deviceSerial);
    bool disableSafetyNet(const QString& deviceSerial);
    bool resetSELinux(const QString& deviceSerial);
    
    // Profile-based Spoofing
    bool applyFingerprintProfile(const QString& deviceSerial, 
                                 const FingerprintConfig& profile);
    
    // Query Methods
    QString getDeviceProperty(const QString& deviceSerial, const QString& property);
    QVariantMap getAllProperties(const QString& deviceSerial);
    bool isDeviceConnected(const QString& deviceSerial);
    QStringList getConnectedDevices();
    
    // ADB Path Configuration
    void setADBPath(const QString& path);
    QString getADBPath() const;
    
signals:
    void spoofApplied(const QString& deviceSerial);
    void spoofCleared(const QString& deviceSerial);
    void spoofError(const QString& deviceSerial, const QString& error);
    void gpsUpdated(const QString& deviceSerial, double lat, double lon);

private:
    explicit ADBSpoofer(QObject* parent = nullptr);
    ~ADBSpoofer();
    ADBSpoofer(const ADBSpoofer&) = delete;
    ADBSpoofer& operator=(const ADBSpoofer&) = delete;
    
    // ADB Execution
    QString executeADBCommand(const QStringList& args, int timeoutMs = 10000);
    bool executeADBCommandSync(const QStringList& args, 
                                QString* output = nullptr, 
                                int timeoutMs = 10000);
    
    // Setprop wrapper
    bool setSystemProperty(const QString& deviceSerial, 
                          const QString& property, 
                          const QString& value);
    bool setPersistProperty(const QString& deviceSerial,
                           const QString& property,
                           const QString& value);
    
    // Build property spoofing
    bool applyBuildSpoofing(const QString& deviceSerial, 
                           const SpoofConfig& config);
    
    // GPS spoofing methods
    bool enableGPSMockLocation(const QString& deviceSerial);
    bool setGPSProviderMockLocation(const QString& deviceSerial,
                                   double lat, double lon);
    
    // Helper methods
    QString generateRandomAndroidId();
    QString generateRandomGSFId();
    bool isValidMAC(const QString& mac);
    bool isValidAndroidId(const QString& id);
    
    // State
    QMap<QString, SpoofConfig> m_appliedConfigs;
    QMap<QString, bool> m_spoofStatus;
    QString m_adbPath;
    std::mutex m_mutex;
};

} // namespace VirtualPhonePro