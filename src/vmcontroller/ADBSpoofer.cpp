/**
 * ADBSpoofer.cpp
 * 
 * Implementation of ADB-based Identity Spoofing
 */

#include "vmcontroller/ADBSpoofer.hpp"
#include <QDebug>
#include <QProcess>
#include <QFile>
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#endif

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
ADBSpoofer& ADBSpoofer::getInstance() {
    static ADBSpoofer instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
ADBSpoofer::ADBSpoofer(QObject* parent)
    : QObject(parent)
{
#ifdef _WIN32
    m_adbPath = "adb.exe";
#else
    m_adbPath = "adb";
#endif
}

ADBSpoofer::~ADBSpoofer() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool ADBSpoofer::initialize() {
    // Verify ADB is available
    QProcess proc;
    proc.start(m_adbPath, { "version" });
    if (!proc.waitForFinished(5000) || proc.exitCode() != 0) {
        qWarning() << "[ADBSpoofer] ADB not found or not working";
        return false;
    }
    
    // Start ADB server if not running
    proc.start(m_adbPath, { "start-server" });
    proc.waitForFinished(5000);
    
    qInfo() << "[ADBSpoofer] Initialized successfully";
    return true;
}

void ADBSpoofer::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Clear all applied spoofs
    for (auto it = m_appliedConfigs.begin(); it != m_appliedConfigs.end(); ++it) {
        clearSpoofFromDevice(it.key());
    }
    m_appliedConfigs.clear();
    
    qInfo() << "[ADBSpoofer] Shutdown complete";
}

// ============================================================================
// Core Spoofing
// ============================================================================
bool ADBSpoofer::applySpoofToDevice(const QString& deviceSerial, const SpoofConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (!isDeviceConnected(deviceSerial)) {
        qWarning() << "[ADBSpoofer] Device not connected:" << deviceSerial;
        emit spoofError(deviceSerial, "Device not connected");
        return false;
    }
    
    qInfo() << "[ADBSpoofer] Applying spoof to device:" << deviceSerial;
    
    bool success = true;
    
    // Apply GPS spoofing
    if (config.latitude != 0.0 || config.longitude != 0.0) {
        success &= setGPSLocation(deviceSerial, config.latitude, config.longitude, config.altitude);
    }
    
    // Apply timezone
    if (!config.timezone.isEmpty()) {
        success &= setTimezone(deviceSerial, config.timezone);
    }
    
    // Apply language
    if (!config.language.isEmpty()) {
        success &= setLanguage(deviceSerial, config.language, config.country);
    }
    
    // Apply proxy
    if (config.useProxy && !config.proxyHost.isEmpty()) {
        success &= setProxy(deviceSerial, config.proxyHost, config.proxyPort);
    }
    
    // Apply hardware spoofing
    success &= applyBuildSpoofing(deviceSerial, config);
    
    // Apply hardware IDs
    if (!config.androidId.isEmpty()) {
        success &= setAndroidId(deviceSerial, config.androidId);
    }
    if (!config.gsfId.isEmpty()) {
        success &= setGSFId(deviceSerial, config.gsfId);
    }
    
    // Apply MAC addresses
    if (!config.wifiMac.isEmpty()) {
        success &= setMACAddress(deviceSerial, config.wifiMac);
    }
    
    // Apply carrier info
    if (!config.carrierName.isEmpty()) {
        success &= setCarrierInfo(deviceSerial, config.carrierName, 
                                 config.carrierCountry, "", "");
    }
    
    // Apply screen properties
    if (config.screenWidth > 0 && config.screenHeight > 0) {
        success &= setScreenProperties(deviceSerial, 
                                      config.screenWidth, 
                                      config.screenHeight, 
                                      config.screenDPI);
    }
    
    // Enable mock locations
    success &= enableMockLocation(deviceSerial);
    
    // Disable Play Services verification
    success &= disablePlayServicesVerification(deviceSerial);
    
    if (success) {
        m_appliedConfigs[deviceSerial] = config;
        m_spoofStatus[deviceSerial] = true;
        emit spoofApplied(deviceSerial);
    } else {
        emit spoofError(deviceSerial, "Some spoof operations failed");
    }
    
    return success;
}

bool ADBSpoofer::clearSpoofFromDevice(const QString& deviceSerial) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    qInfo() << "[ADBSpoofer] Clearing spoof from device:" << deviceSerial;
    
    // Reset GPS
    setGPSLocation(deviceSerial, 0, 0);
    
    // Clear proxy
    clearProxy(deviceSerial);
    
    // Clear mock location setting
    setSystemProperty(deviceSerial, "persist.mock.location._enabled", "0");
    
    // Reboot to reset some properties
    executeADBCommand({ "-s", deviceSerial, "reboot" });
    QThread::sleep(10); // Wait for reboot
    
    m_appliedConfigs.remove(deviceSerial);
    m_spoofStatus[deviceSerial] = false;
    
    emit spoofCleared(deviceSerial);
    return true;
}

bool ADBSpoofer::isSpoofApplied(const QString& deviceSerial) {
    return m_spoofStatus.value(deviceSerial, false);
}

// ============================================================================
// Individual Spoofing Methods
// ============================================================================
bool ADBSpoofer::setGPSLocation(const QString& deviceSerial, double lat, double lon, double alt) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting GPS:" << lat << "," << lon;
    
    // Enable mock location app permission
    executeADBCommand({
        "-s", deviceSerial, "shell", "appops", "set",
        "com.android.providers.settings", "WRITE_SECURE_SETTINGS", "allow"
    });
    
    // Enable mock locations in developer options
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "secure", "mock_location", "1"
    });
    
    // Set GPS provider location
    QString cmd = QString(
        "am", "startservice", "-n", "com.android.location.fusedprovider/FusedProvider"
    );
    executeADBCommand({ "-s", deviceSerial, "shell", cmd });
    
    // Method 1: Using settings database
    QString locationStr = QString("%1,%2").arg(lat, 0, 'f', 6).arg(lon, 0, 'f', 6);
    bool res1 = executeADBCommandSync({
        "-s", deviceSerial, "shell", "settings", "put",
        "secure", "location_providers_allowed", "+gps,network"
    });
    
    // Method 2: Using geocoder mock
    executeADBCommand({
        "-s", deviceSerial, "shell", "setprop",
        "persist.mock.location.latitude", QString::number(lat)
    });
    executeADBCommand({
        "-s", deviceSerial, "shell", "setprop",
        "persist.mock.location.longitude", QString::number(lon)
    });
    
    // Method 3: Using dumpsys location
    QString locationCmd = QString(
        "killall"
    );
    
    // Set via service
    executeADBCommand({
        "-s", deviceSerial, "shell", "dumpsys", "location", "--location", 
        QString::number(lat), QString::number(lon)
    });
    
    emit gpsUpdated(deviceSerial, lat, lon);
    return true;
}

bool ADBSpoofer::setTimezone(const QString& deviceSerial, const QString& timezone) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting timezone:" << timezone;
    
    // Set system timezone
    bool success = setSystemProperty(deviceSerial, "persist.sys.timezone", timezone);
    
    // Also set via settings for immediate effect
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "global", "time_zone_24_format", "0"
    });
    
    // Set auto timezone off
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "global", "auto_time_zone", "0"
    });
    
    return success;
}

bool ADBSpoofer::setLanguage(const QString& deviceSerial, const QString& language, const QString& country) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting language:" << language << country;
    
    QString locale = country.isEmpty() ? language : QString("%1-%2").arg(language).arg(country);
    
    // Set system locale
    setSystemProperty(deviceSerial, "persist.sys.language", language);
    setSystemProperty(deviceSerial, "persist.sys.country", country);
    setSystemProperty(deviceSerial, "persist.sys.locale", locale);
    setSystemProperty(deviceSerial, "ro.product.locale", locale);
    setSystemProperty(deviceSerial, "ro.product.locale.language", language);
    setSystemProperty(deviceSerial, "ro.product.locale.region", country);
    
    return true;
}

bool ADBSpoofer::setProxy(const QString& deviceSerial, const QString& host, int port) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting proxy:" << host << ":" << port;
    
    // Global proxy
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "global", "http_proxy", QString("%1:%2").arg(host).arg(port)
    });
    
    // WiFi proxy (for all WiFi networks)
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "global", "wifi_static_ip", "true"
    });
    
    return true;
}

bool ADBSpoofer::clearProxy(const QString& deviceSerial) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Clearing proxy";
    
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "global", "http_proxy", ":0"
    });
    
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "delete",
        "global", "http_proxy"
    });
    
    return true;
}

// ============================================================================
// Hardware Spoofing
// ============================================================================
bool ADBSpoofer::setAndroidId(const QString& deviceSerial, const QString& androidId) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    if (!isValidAndroidId(androidId)) {
        qWarning() << "[ADBSpoofer] Invalid Android ID:" << androidId;
        return false;
    }
    
    qDebug() << "[ADBSpoofer] Setting Android ID:" << androidId;
    
    // Set in settings database
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "secure", "android_id", androidId
    });
    
    // Also set as persist property for apps that read it directly
    setPersistProperty(deviceSerial, "persist.sys.android_id", androidId);
    
    return true;
}

bool ADBSpoofer::setGSFId(const QString& deviceSerial, const QString& gsfId) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting GSF ID:" << gsfId;
    
    // GSF ID is stored in Google Play Services
    setPersistProperty(deviceSerial, "persist.gsfid", gsfId);
    
    // Write to GSF SQLite database if possible
    executeADBCommand({
        "-s", deviceSerial, "shell", "content", "insert",
        "-d", "content://com.google.gservices.android.provider.gsf",
        "--eu", "android.intent.extra.NAME", "android_id",
        "--ei", "value", gsfId
    });
    
    return true;
}

bool ADBSpoofer::setDeviceInfo(const QString& deviceSerial,
                              const QString& manufacturer,
                              const QString& model,
                              const QString& brand) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting device info:" << manufacturer << model << brand;
    
    setPersistProperty(deviceSerial, "ro.product.manufacturer", manufacturer);
    setPersistProperty(deviceSerial, "ro.product.model", model);
    setPersistProperty(deviceSerial, "ro.product.brand", brand);
    setPersistProperty(deviceSerial, "ro.product.name", model);
    
    // Also set system properties
    setSystemProperty(deviceSerial, "ro.product.manufacturer", manufacturer);
    setSystemProperty(deviceSerial, "ro.product.model", model);
    setSystemProperty(deviceSerial, "ro.product.brand", brand);
    
    return true;
}

bool ADBSpoofer::setBuildFingerprint(const QString& deviceSerial, const QString& fingerprint) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting build fingerprint:" << fingerprint;
    
    setPersistProperty(deviceSerial, "ro.build.fingerprint", fingerprint);
    setSystemProperty(deviceSerial, "ro.build.fingerprint", fingerprint);
    
    return true;
}

bool ADBSpoofer::setBootloader(const QString& deviceSerial, const QString& bootloader) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting bootloader:" << bootloader;
    
    setPersistProperty(deviceSerial, "ro.bootloader", bootloader);
    setSystemProperty(deviceSerial, "ro.bootloader", bootloader);
    
    return true;
}

// ============================================================================
// Network Spoofing
// ============================================================================
bool ADBSpoofer::setMACAddress(const QString& deviceSerial, const QString& macAddress) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    if (!isValidMAC(macAddress)) {
        qWarning() << "[ADBSpoofer] Invalid MAC address:" << macAddress;
        return false;
    }
    
    qDebug() << "[ADBSpoofer] Setting MAC address:" << macAddress;
    
    // WiFi MAC
    setPersistProperty(deviceSerial, "persist.wifi.mac", macAddress);
    setSystemProperty(deviceSerial, "wifi.interface", "wlan0");
    
    // Write to nvram
    executeADBCommand({
        "-s", deviceSerial, "shell", "ip", "link", "set", "wlan0", "down"
    });
    executeADBCommand({
        "-s", deviceSerial, "shell", "ip", "link", "set", "wlan0", "address", macAddress
    });
    executeADBCommand({
        "-s", deviceSerial, "shell", "ip", "link", "set", "wlan0", "up"
    });
    
    return true;
}

bool ADBSpoofer::setCarrierInfo(const QString& deviceSerial,
                               const QString& carrierName,
                               const QString& country,
                               const QString& mcc,
                               const QString& mnc) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting carrier info:" << carrierName;
    
    setPersistProperty(deviceSerial, "persist.carrier.name", carrierName);
    setSystemProperty(deviceSerial, "ro.carrier.name", carrierName);
    
    if (!country.isEmpty()) {
        setPersistProperty(deviceSerial, "persist.sys.country", country);
    }
    if (!mcc.isEmpty()) {
        setSystemProperty(deviceSerial, "ro.sim.mcc", mcc);
    }
    if (!mnc.isEmpty()) {
        setSystemProperty(deviceSerial, "ro.sim.mnc", mnc);
    }
    
    // Set as default carrier
    executeADBCommand({
        "-s", deviceSerial, "shell", "service", "call", "iphonesubinfo", "15"
    });
    
    return true;
}

// ============================================================================
// Display Spoofing
// ============================================================================
bool ADBSpoofer::setScreenProperties(const QString& deviceSerial, 
                                     int width, int height, int dpi) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Setting screen:" << width << "x" << height << "@" << dpi;
    
    // Set display properties
    setPersistProperty(deviceSerial, "persist.sys.screen.width", QString::number(width));
    setPersistProperty(deviceSerial, "persist.sys.screen.height", QString::number(height));
    setPersistProperty(deviceSerial, "persist.sys.screen.dpi", QString::number(dpi));
    
    // WM commands for immediate effect
    executeADBCommand({
        "-s", deviceSerial, "shell", "wm", "size", 
        QString("%1x%2").arg(width).arg(height)
    });
    executeADBCommand({
        "-s", deviceSerial, "shell", "wm", "density", QString::number(dpi)
    });
    
    return true;
}

// ============================================================================
// Advanced Spoofing
// ============================================================================
bool ADBSpoofer::disablePlayServicesVerification(const QString& deviceSerial) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Disabling Play Services verification";
    
    // Disable SafetyNet
    executeADBCommand({
        "-s", deviceSerial, "shell", "pm", "disable-user", "--user", "0",
        "com.google.android.gms"
    });
    
    // Set SafetyNet flags
    setPersistProperty(deviceSerial, "persist.sys.safetynet.enabled", "false");
    setPersistProperty(deviceSerial, "ro.build.selinux", "permissive");
    
    return true;
}

bool ADBSpoofer::enableMockLocation(const QString& deviceSerial) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Enabling mock location";
    
    // Enable developer options
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "global", "developer_options_enabled", "1"
    });
    
    // Enable mock locations
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "secure", "mock_location", "1"
    });
    
    setPersistProperty(deviceSerial, "persist.mock.location.always", "true");
    
    return true;
}

bool ADBSpoofer::disableSafetyNet(const QString& deviceSerial) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Disabling SafetyNet";
    
    // Set system properties
    setPersistProperty(deviceSerial, "ro.secure", "0");
    setPersistProperty(deviceSerial, "ro.debuggable", "1");
    setPersistProperty(deviceSerial, "ro.build.selinux", "permissive");
    setPersistProperty(deviceSerial, "persist.sys.safetynet", "disabled");
    
    // Set flags
    setSystemProperty(deviceSerial, "ro.build.type", "user");
    setSystemProperty(deviceSerial, "ro.build.tags", "release-keys");
    
    return true;
}

bool ADBSpoofer::resetSELinux(const QString& deviceSerial) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qDebug() << "[ADBSpoofer] Resetting SELinux to permissive";
    
    executeADBCommand({
        "-s", deviceSerial, "shell", "setenforce", "0"
    });
    
    setPersistProperty(deviceSerial, "persist.sys.selinux", "permissive");
    
    return true;
}

// ============================================================================
// Profile-based Spoofing
// ============================================================================
bool ADBSpoofer::applyFingerprintProfile(const QString& deviceSerial, 
                                         const FingerprintConfig& profile) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    qInfo() << "[ADBSpoofer] Applying fingerprint profile to device:" << deviceSerial;
    
    SpoofConfig config;
    
    // Map FingerprintConfig to SpoofConfig
    config.latitude = profile.latitude;
    config.longitude = profile.longitude;
    config.timezone = profile.timezone;
    config.language = profile.locale.split('_').first();
    config.country = profile.locale.split('_').last();
    config.manufacturer = profile.manufacturer;
    config.model = profile.model;
    config.brand = profile.brand;
    config.androidId = profile.fingerprint.isEmpty() ? generateRandomAndroidId() : profile.fingerprint;
    config.wifiMac = profile.macAddress;
    config.screenWidth = profile.screenWidth;
    config.screenHeight = profile.screenHeight;
    config.screenDPI = profile.screenDPI;
    
    return applySpoofToDevice(deviceSerial, config);
}

// ============================================================================
// Query Methods
// ============================================================================
QString ADBSpoofer::getDeviceProperty(const QString& deviceSerial, const QString& property) {
    if (!isDeviceConnected(deviceSerial)) return QString();
    
    QString output;
    executeADBCommandSync({
        "-s", deviceSerial, "shell", "getprop", property
    }, &output);
    
    return output.trimmed();
}

QVariantMap ADBSpoofer::getAllProperties(const QString& deviceSerial) {
    QVariantMap properties;
    
    if (!isDeviceConnected(deviceSerial)) return properties;
    
    QString output;
    executeADBCommandSync({
        "-s", deviceSerial, "shell", "getprop"
    }, &output);
    
    // Parse getprop output
    // Format: [property.name]: [value]
    QRegularExpression re(R"(\[([^\]]+)\]: \[([^\]]*)\])");
    QRegularExpressionMatchIterator it = re.globalMatch(output);
    
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        properties[match.captured(1)] = match.captured(2);
    }
    
    return properties;
}

bool ADBSpoofer::isDeviceConnected(const QString& deviceSerial) {
    QStringList devices = getConnectedDevices();
    return devices.contains(deviceSerial);
}

QStringList ADBSpoofer::getConnectedDevices() {
    QString output;
    executeADBCommandSync({ "devices" }, &output);
    
    QStringList devices;
    for (const QString& line : output.split('\n')) {
        if (line.contains("\tdevice") && !line.contains("List")) {
            QString serial = line.split('\t').first().trimmed();
            if (!serial.isEmpty()) {
                devices.append(serial);
            }
        }
    }
    
    return devices;
}

// ============================================================================
// ADB Path Configuration
// ============================================================================
void ADBSpoofer::setADBPath(const QString& path) {
    m_adbPath = path;
}

QString ADBSpoofer::getADBPath() const {
    return m_adbPath;
}

// ============================================================================
// Private Helpers
// ============================================================================
QString ADBSpoofer::executeADBCommand(const QStringList& args, int timeoutMs) {
    QProcess proc;
    proc.start(m_adbPath, args);
    proc.waitForFinished(timeoutMs);
    return proc.readAllStandardOutput();
}

bool ADBSpoofer::executeADBCommandSync(const QStringList& args, 
                                       QString* output, 
                                       int timeoutMs) {
    QProcess proc;
    proc.start(m_adbPath, args);
    bool success = proc.waitForFinished(timeoutMs);
    
    if (output) {
        *output = proc.readAllStandardOutput();
    }
    
    return success && proc.exitCode() == 0;
}

bool ADBSpoofer::setSystemProperty(const QString& deviceSerial, 
                                   const QString& property, 
                                   const QString& value) {
    return executeADBCommandSync({
        "-s", deviceSerial, "shell", "setprop", property, value
    });
}

bool ADBSpoofer::setPersistProperty(const QString& deviceSerial,
                                   const QString& property,
                                   const QString& value) {
    bool success = executeADBCommandSync({
        "-s", deviceSerial, "shell", "setprop", property, value
    });
    
    // Also try setting persist version
    QString persistProp = property;
    if (!persistProp.startsWith("persist.")) {
        persistProp = "persist." + property;
        success &= executeADBCommandSync({
            "-s", deviceSerial, "shell", "setprop", persistProp, value
        });
    }
    
    return success;
}

bool ADBSpoofer::applyBuildSpoofing(const QString& deviceSerial, 
                                    const SpoofConfig& config) {
    if (!isDeviceConnected(deviceSerial)) return false;
    
    // Build fingerprint template
    QString fingerprint = QString("%1/%2/%3:%4/%5:%2/%2")
        .arg(config.brand.isEmpty() ? config.manufacturer : config.brand)
        .arg(config.model)
        .arg(config.device)
        .arg(config.osVersion)
        .arg(config.buildId);
    
    // Apply all build properties
    if (!config.manufacturer.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.product.manufacturer", config.manufacturer);
        setSystemProperty(deviceSerial, "ro.product.manufacturer", config.manufacturer);
    }
    
    if (!config.model.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.product.model", config.model);
        setSystemProperty(deviceSerial, "ro.product.model", config.model);
    }
    
    if (!config.brand.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.product.brand", config.brand);
        setSystemProperty(deviceSerial, "ro.product.brand", config.brand);
    }
    
    if (!config.device.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.product.device", config.device);
        setSystemProperty(deviceSerial, "ro.product.device", config.device);
    }
    
    if (!config.product.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.product.name", config.product);
        setSystemProperty(deviceSerial, "ro.product.name", config.product);
    }
    
    if (!config.board.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.product.board", config.board);
        setSystemProperty(deviceSerial, "ro.product.board", config.board);
    }
    
    if (!config.hardware.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.hardware", config.hardware);
        setSystemProperty(deviceSerial, "ro.hardware", config.hardware);
    }
    
    // Build fingerprint
    if (!fingerprint.isEmpty() && fingerprint != "//:") {
        setPersistProperty(deviceSerial, "ro.build.fingerprint", fingerprint);
        setSystemProperty(deviceSerial, "ro.build.fingerprint", fingerprint);
    }
    
    if (!config.bootloader.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.bootloader", config.bootloader);
        setSystemProperty(deviceSerial, "ro.bootloader", config.bootloader);
    }
    
    if (!config.buildId.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.build.id", config.buildId);
        setSystemProperty(deviceSerial, "ro.build.id", config.buildId);
    }
    
    if (!config.securityPatch.isEmpty()) {
        setPersistProperty(deviceSerial, "ro.build.version.security_patch", config.securityPatch);
        setSystemProperty(deviceSerial, "ro.build.version.security_patch", config.securityPatch);
    }
    
    // API Level
    if (config.apiLevel > 0) {
        setPersistProperty(deviceSerial, "ro.build.version.sdk", QString::number(config.apiLevel));
        setSystemProperty(deviceSerial, "ro.build.version.sdk", QString::number(config.apiLevel));
    }
    
    return true;
}

bool ADBSpoofer::enableGPSMockLocation(const QString& deviceSerial) {
    // Grant mock location permission
    executeADBCommand({
        "-s", deviceSerial, "shell", "appops", "set",
        "com.android.providers.settings", "WRITE_SECURE_SETTINGS", "allow"
    });
    
    // Enable mock location
    return setSystemProperty(deviceSerial, "persist.mock.location.always", "1") &&
           setSystemProperty(deviceSerial, "mock.location.enabled", "1");
}

bool ADBSpoofer::setGPSProviderMockLocation(const QString& deviceSerial,
                                            double lat, double lon) {
    // Use settings command to set mock location
    executeADBCommand({
        "-s", deviceSerial, "shell", "settings", "put",
        "secure", "location_providers_allowed", "+gps,network"
    });
    
    // For more accurate GPS spoofing, the app would need a mock location provider app
    // This sets the location that a mock provider would report
    return true;
}

QString ADBSpoofer::generateRandomAndroidId() {
    // Generate 16 character hex Android ID
    QString chars = "0123456789abcdef";
    QString id;
    for (int i = 0; i < 16; ++i) {
        id.append(chars[QRandomGenerator::global()->bounded(16)]);
    }
    return id;
}

QString ADBSpoofer::generateRandomGSFId() {
    // Generate 12 digit GSF ID
    quint64 id = QRandomGenerator::global()->bounded(100000000000ULL);
    return QString::number(id);
}

bool ADBSpoofer::isValidMAC(const QString& mac) {
    QRegularExpression re("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
    return re.match(mac).hasMatch();
}

bool ADBSpoofer::isValidAndroidId(const QString& id) {
    // Android ID is 16 hex characters
    QRegularExpression re("^[0-9a-fA-F]{16}$");
    return re.match(id).hasMatch();
}

} // namespace VirtualPhonePro