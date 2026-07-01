/**
 * QEMUSpoofer.cpp
 * 
 * Implementation of QEMU-based Device Spoofing
 * No ADB required - uses QEMU device parameters and kernel command line
 */

#include "vmcontroller/QEMUSpoofer.hpp"
#include <QDebug>
#include <QRandomGenerator>

namespace VirtualPhonePro {

// ============================================================================
// Singleton Implementation
// ============================================================================
QEMUSpoofer& QEMUSpoofer::getInstance() {
    static QEMUSpoofer instance;
    return instance;
}

// ============================================================================
// Constructor & Destructor
// ============================================================================
QEMUSpoofer::QEMUSpoofer(QObject* parent)
    : QObject(parent)
{
}

QEMUSpoofer::~QEMUSpoofer() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================
bool QEMUSpoofer::initialize() {
    qInfo() << "[QEMUSpoofer] Initialized successfully";
    return true;
}

void QEMUSpoofer::shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_spoofingStates.clear();
    qInfo() << "[QEMUSpoofer] Shutdown complete";
}

// ============================================================================
// Complete Spoofing
// ============================================================================
bool QEMUSpoofer::applySpoofing(const QString& instanceId,
                               const FingerprintConfig& profile,
                               const SpoofConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);

    qInfo() << "[QEMUSpoofer] Applying complete spoofing to instance:" << instanceId;

    // Create spoofing state
    SpoofingState state;
    state.fingerprint = profile;
    state.config = config;
    state.active = true;

    // Generate unique identifiers if not provided
    if (config.androidId.isEmpty()) {
        state.androidId = generateAndroidId();
    } else {
        state.androidId = config.androidId;
    }

    if (config.gsfId.isEmpty()) {
        state.gsfId = generateGSFId();
    } else {
        state.gsfId = config.gsfId;
    }

    if (config.wifiMac.isEmpty()) {
        state.macAddress = generateMAC();
    } else {
        state.macAddress = config.wifiMac;
    }

    if (config.imei.isEmpty()) {
        generateIMEI(&state.imei);
    } else {
        state.imei = config.imei;
    }

    if (config.latitude != 0.0 || config.longitude != 0.0) {
        state.latitude = config.latitude;
        state.longitude = config.longitude;
    } else {
        state.latitude = profile.latitude;
        state.longitude = profile.longitude;
    }

    state.timezone = config.timezone.isEmpty() ? profile.timezone : config.timezone;

    // Store state
    m_spoofingStates[instanceId] = state;

    // Apply via QEMU controller
    QEMUController& qemu = QEMUController::getInstance();

    // Set device info
    qemu.setDeviceInfo(instanceId, profile.manufacturer, profile.model, profile.brand);

    // Set serial number
    qemu.setSerialNumber(instanceId, state.serialNumber);

    // Set Android ID
    qemu.setAndroidId(instanceId, state.androidId);

    // Set MAC address
    qemu.setMACAddress(instanceId, state.macAddress);

    // Set GPS coordinates
    if (state.latitude != 0.0 || state.longitude != 0.0) {
        qemu.setGPSCoordinates(instanceId, state.latitude, state.longitude);
    }

    // Set timezone
    if (!state.timezone.isEmpty()) {
        qemu.setTimezone(instanceId, state.timezone);
    }

    emit spoofApplied(instanceId);
    qInfo() << "[QEMUSpoofer] Spoofing applied successfully";

    return true;
}

bool QEMUSpoofer::clearSpoofing(const QString& instanceId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        return false;
    }

    m_spoofingStates.remove(instanceId);

    emit spoofCleared(instanceId);
    qInfo() << "[QEMUSpoofer] Spoofing cleared for instance:" << instanceId;

    return true;
}

bool QEMUSpoofer::isSpoofActive(const QString& instanceId) const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_spoofingStates.contains(instanceId) && m_spoofingStates[instanceId].active;
}

// ============================================================================
// Individual Spoofing Methods
// ============================================================================

// Device Identity
bool QEMUSpoofer::setDeviceModel(const QString& instanceId, const QString& manufacturer,
                                   const QString& model, const QString& brand) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.manufacturer = manufacturer;
    m_spoofingStates[instanceId].fingerprint.model = model;
    m_spoofingStates[instanceId].fingerprint.brand = brand;

    return QEMUController::getInstance().setDeviceInfo(instanceId, manufacturer, model, brand);
}

bool QEMUSpoofer::setSerialNumber(const QString& instanceId, const QString& serial) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].serialNumber = serial;
    return QEMUController::getInstance().setSerialNumber(instanceId, serial);
}

bool QEMUSpoofer::setAndroidId(const QString& instanceId, const QString& androidId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].androidId = androidId;
    return QEMUController::getInstance().setAndroidId(instanceId, androidId);
}

bool QEMUSpoofer::setGSFId(const QString& instanceId, const QString& gsfId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].gsfId = gsfId;
    return true;  // GSF ID is stored locally, applied at boot
}

bool QEMUSpoofer::setIMEI(const QString& instanceId, const QString& imei) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].imei = imei;
    return QEMUController::getInstance().setIMEI(instanceId, imei);
}

// Build Properties
bool QEMUSpoofer::setBuildFingerprint(const QString& instanceId, const QString& fingerprint) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.fingerprint = fingerprint;
    return true;  // Applied via kernel command line
}

bool QEMUSpoofer::setBootloader(const QString& instanceId, const QString& bootloader) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.bootloader = bootloader;
    return true;
}

bool QEMUSpoofer::setSecurityPatch(const QString& instanceId, const QString& patch) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.securityPatch = patch;
    return true;
}

bool QEMUSpoofer::setBuildId(const QString& instanceId, const QString& buildId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.buildId = buildId;
    return true;
}

// Network
bool QEMUSpoofer::setMACAddress(const QString& instanceId, const QString& mac) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].macAddress = mac;
    return QEMUController::getInstance().setMACAddress(instanceId, mac);
}

bool QEMUSpoofer::setCarrierInfo(const QString& instanceId, const QString& carrier,
                                 const QString& country, const QString& mcc, const QString& mnc) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].config.carrierName = carrier;
    m_spoofingStates[instanceId].config.carrierCountry = country;

    return true;
}

// Location
bool QEMUSpoofer::setGPSCoordinates(const QString& instanceId, double lat, double lon) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].latitude = lat;
    m_spoofingStates[instanceId].longitude = lon;

    return QEMUController::getInstance().setGPSCoordinates(instanceId, lat, lon);
}

bool QEMUSpoofer::setTimezone(const QString& instanceId, const QString& timezone) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].timezone = timezone;
    return QEMUController::getInstance().setTimezone(instanceId, timezone);
}

bool QEMUSpoofer::setLanguage(const QString& instanceId, const QString& lang, const QString& country) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.locale = QString("%1_%2").arg(lang).arg(country);
    return true;
}

// Display
bool QEMUSpoofer::setScreenProperties(const QString& instanceId, int width, int height, int dpi) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.screenWidth = width;
    m_spoofingStates[instanceId].fingerprint.screenHeight = height;
    m_spoofingStates[instanceId].fingerprint.screenDPI = dpi;

    return QEMUController::getInstance().setScreenSize(instanceId, width, height);
}

// Hardware
bool QEMUSpoofer::setCPUInfo(const QString& instanceId, const QString& cpuModel) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.cpuModel = cpuModel;
    return true;
}

bool QEMUSpoofer::setGPUInfo(const QString& instanceId, const QString& gpuVendor, const QString& gpuRenderer) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.gpuVendor = gpuVendor;
    m_spoofingStates[instanceId].fingerprint.gpuRenderer = gpuRenderer;
    return true;
}

bool QEMUSpoofer::setSensorInfo(const QString& instanceId, const QString& sensorModel) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        m_spoofingStates[instanceId] = SpoofingState();
    }

    m_spoofingStates[instanceId].fingerprint.sensorModel = sensorModel;
    return true;
}

// Battery
bool QEMUSpoofer::setBatteryProperties(const QString& instanceId, int level, bool charging) {
    Q_UNUSED(instanceId);
    Q_UNUSED(level);
    Q_UNUSED(charging);
    // Battery properties are typically set via guest agent or HAL
    return true;
}

// ============================================================================
// Kernel Command Line Builder
// ============================================================================
QString QEMUSpoofer::buildKernelCmdLine(const QString& instanceId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        return QString();
    }

    const SpoofingState& state = m_spoofingStates[instanceId];
    QStringList params;

    // Device identification
    params << QString("androidboot.hardware=%1").arg(state.fingerprint.model.toLower());
    params << QString("androidboot.serialno=%1").arg(state.serialNumber);
    params << QString("ro.serialno=%1").arg(state.serialNumber);

    // Product info
    params << QString("ro.product.model=%1").arg(state.fingerprint.model);
    params << QString("ro.product.brand=%1").arg(state.fingerprint.brand);
    params << QString("ro.product.manufacturer=%1").arg(state.fingerprint.manufacturer);
    params << QString("ro.product.name=%1").arg(state.fingerprint.model);

    // Android ID
    params << QString("androidboot.android_id=%1").arg(state.androidId);

    // Build info
    params << QString("ro.build.id=%1").arg(state.fingerprint.buildId);
    params << QString("ro.build.product=%1").arg(state.fingerprint.model);
    params << QString("ro.build.version.release=%1").arg(state.fingerprint.androidVersion);
    params << QString("ro.build.version.sdk=%1").arg(state.fingerprint.androidVersion);
    params << QString("ro.build.version.security_patch=%1").arg(state.fingerprint.securityPatch);

    // Bootloader
    params << QString("ro.bootloader=%1").arg(state.fingerprint.bootloader);

    // Hardware
    params << QString("ro.hardware=%1").arg(state.fingerprint.cpuABI);
    params << QString("ro.arch=%1").arg(state.fingerprint.cpuABI);

    // Location
    if (state.latitude != 0.0 || state.longitude != 0.0) {
        params << QString("gps.latitude=%1").arg(state.latitude, 0, 'f', 6);
        params << QString("gps.longitude=%1").arg(state.longitude, 0, 'f', 6);
    }

    // Timezone
    if (!state.timezone.isEmpty()) {
        params << QString("androidboot.timezone=%1").arg(state.timezone);
    }

    // GSF ID (for Google Play Services)
    if (!state.gsfId.isEmpty()) {
        params << QString("ro.config.gsfid=%1").arg(state.gsfId);
    }

    return params.join(" ");
}

QStringList QEMUSpoofer::buildQEMUDevices(const QString& instanceId) {
    std::lock_guard<std::mutex> lock(m_mutex);

    if (!m_spoofingStates.contains(instanceId)) {
        return QStringList();
    }

    const SpoofingState& state = m_spoofingStates[instanceId];
    QStringList devices;

    // Network device with MAC
    devices << "-device" << QString("virtio-net-pci,mac=%1,netdev=net0").arg(state.macAddress);
    devices << "-netdev" << QString("user,id=net0");

    return devices;
}

// ============================================================================
// Profile-based Spoofing
// ============================================================================
bool QEMUSpoofer::applyFingerprintProfile(const QString& instanceId, const FingerprintConfig& profile) {
    SpoofConfig config;
    config.latitude = profile.latitude;
    config.longitude = profile.longitude;
    config.timezone = profile.timezone;
    config.language = profile.locale.split('_').first();
    config.country = profile.locale.split('_').last();
    config.manufacturer = profile.manufacturer;
    config.model = profile.model;
    config.brand = profile.brand;
    config.screenWidth = profile.screenWidth;
    config.screenHeight = profile.screenHeight;
    config.screenDPI = profile.screenDPI;

    return applySpoofing(instanceId, profile, config);
}

QVariantMap QEMUSpoofer::getSpoofingStatus(const QString& instanceId) const {
    std::lock_guard<std::mutex> lock(m_mutex);

    QVariantMap status;

    if (!m_spoofingStates.contains(instanceId)) {
        status["active"] = false;
        return status;
    }

    const SpoofingState& state = m_spoofingStates[instanceId];

    status["active"] = state.active;
    status["manufacturer"] = state.fingerprint.manufacturer;
    status["model"] = state.fingerprint.model;
    status["brand"] = state.fingerprint.brand;
    status["serialNumber"] = state.serialNumber;
    status["androidId"] = state.androidId;
    status["macAddress"] = state.macAddress;
    status["latitude"] = state.latitude;
    status["longitude"] = state.longitude;
    status["timezone"] = state.timezone;

    return status;
}

// ============================================================================
// Private Helpers
// ============================================================================
QString QEMUSpoofer::generateSerialNumber() {
    QString chars = "0123456789ABCDEF";
    QString serial;
    for (int i = 0; i < 16; ++i) {
        serial.append(chars[QRandomGenerator::global()->bounded(16)]);
    }
    return serial;
}

QString QEMUSpoofer::generateAndroidId() {
    QString chars = "0123456789abcdef";
    QString id;
    for (int i = 0; i < 16; ++i) {
        id.append(chars[QRandomGenerator::global()->bounded(16)]);
    }
    return id;
}

QString QEMUSpoofer::generateGSFId() {
    quint64 id = QRandomGenerator::global()->bounded(100000000000ULL);
    return QString::number(id);
}

QString QEMUSpoofer::generateMAC() {
    QStringList oui = {"02:", "06:", "0a:", "12:", "1e:", "3a:"};
    QString mac = oui[QRandomGenerator::global()->bounded(oui.size())];

    for (int i = 0; i < 3; ++i) {
        mac += QString("%1:").arg(QRandomGenerator::global()->bounded(256), 2, 16, QChar('0')).toUpper();
    }
    return mac.chopped(1);
}

bool QEMUSpoofer::generateIMEI(QString* outIMEI) {
    QString imei = "35";
    for (int i = 0; i < 13; ++i) {
        imei.append(QString::number(QRandomGenerator::global()->bounded(10)));
    }

    int sum = 0;
    for (int i = 0; i < 14; ++i) {
        int digit = imei[i].digitValue();
        if (i % 2 == 0) {
            digit *= 2;
            if (digit > 9) digit -= 9;
        }
        sum += digit;
    }
    int checkDigit = (10 - (sum % 10)) % 10;
    imei.append(QString::number(checkDigit));

    *outIMEI = imei;
    return true;
}

QString QEMUSpoofer::buildDeviceCmdLine(const QString& instanceId, const FingerprintConfig& profile) {
    Q_UNUSED(instanceId);

    return QString(
        "androidboot.hardware=%1 "
        "ro.product.model=%2 "
        "ro.product.brand=%3 "
        "ro.product.manufacturer=%4 "
        "ro.build.id=%5 "
        "ro.build.version.release=%6"
    ).arg(profile.model.toLower())
     .arg(profile.model)
     .arg(profile.brand)
     .arg(profile.manufacturer)
     .arg(profile.buildId)
     .arg(profile.androidVersion);
}

QString QEMUSpoofer::buildNetworkCmdLine(const QString& instanceId, const SpoofConfig& config) {
    Q_UNUSED(instanceId);

    if (config.wifiMac.isEmpty()) {
        return QString();
    }

    return QString("android.wifi.mac=%1").arg(config.wifiMac);
}

QString QEMUSpoofer::buildGPSCmdLine(const SpoofConfig& config) {
    if (config.latitude == 0.0 && config.longitude == 0.0) {
        return QString();
    }

    return QString("gps.latitude=%1 gps.longitude=%2")
        .arg(config.latitude, 0, 'f', 6)
        .arg(config.longitude, 0, 'f', 6);
}

QString QEMUSpoofer::buildBuildCmdLine(const FingerprintConfig& profile) {
    return QString(
        "ro.build.id=%1 "
        "ro.build.version.security_patch=%2 "
        "ro.bootloader=%3"
    ).arg(profile.buildId)
     .arg(profile.securityPatch)
     .arg(profile.bootloader);
}

} // namespace VirtualPhonePro