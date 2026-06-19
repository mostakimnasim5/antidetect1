#include "AntiDetectEngine.hpp"
#include "ADBManager.hpp"
#include "DeviceFingerprint.hpp"
#include "NetworkSpoofer.hpp"
#include "SystemManager.hpp"
#include "ProfileManager.hpp"
#include "AdvancedSpoofing.hpp"
#include "Logger.hpp"

namespace AntiDetect {

class AntiDetectEngine::Impl {
public:
    Impl() : m_adbManager(ADBManager::getInstance()),
              m_deviceFingerprint(std::make_unique<DeviceFingerprint>()),
              m_networkSpoofer(std::make_unique<NetworkSpoofer>()),
              m_systemManager(std::make_unique<SystemManager>()),
              m_profileManager(std::make_unique<ProfileManager>()),
              m_advancedSpoofing(std::make_unique<AdvancedSpoofing>())
    {
    }
    
    ADBManager& m_adbManager;
    std::unique_ptr<DeviceFingerprint> m_deviceFingerprint;
    std::unique_ptr<NetworkSpoofer> m_networkSpoofer;
    std::unique_ptr<SystemManager> m_systemManager;
    std::unique_ptr<ProfileManager> m_profileManager;
    std::unique_ptr<AdvancedSpoofing> m_advancedSpoofing;
};

AntiDetectEngine::AntiDetectEngine() : m_impl(std::make_unique<Impl>()) {}

AntiDetectEngine::~AntiDetectEngine() = default;

bool AntiDetectEngine::initialize() {
    Logger::getInstance().info("Initializing AntiDetect Engine...");
    
    if (!m_impl->m_adbManager.initialize()) {
        Logger::getInstance().warning("ADB initialization failed - limited functionality");
    }
    
    m_impl->m_deviceFingerprint->initialize();
    m_impl->m_networkSpoofer->initialize();
    m_impl->m_systemManager->initialize();
    m_impl->m_profileManager->initialize();
    m_impl->m_advancedSpoofing->initialize();
    
    Logger::getInstance().info("AntiDetect Engine initialized");
    return true;
}

bool AntiDetectEngine::shutdown() {
    Logger::getInstance().info("Shutting down AntiDetect Engine...");
    m_impl.reset();
    return true;
}

OperationResult AntiDetectEngine::createResult(bool success, const std::string& message, const std::string& error) {
    return {success, message, error, {}};
}

std::vector<DeviceInfo> AntiDetectEngine::listDevices() {
    std::vector<DeviceInfo> devices;
    auto deviceList = m_impl->m_adbManager.getDevices();
    
    for (const auto& d : deviceList) {
        DeviceInfo info;
        info.serial = d.serial;
        info.model = d.model;
        info.state = d.state;
        info.product = d.product;
        info.device = d.device;
        info.transportId = d.transportId;
        devices.push_back(info);
    }
    
    return devices;
}

OperationResult AntiDetectEngine::connect(const std::string& address) {
    bool success = m_impl->m_adbManager.connect(address);
    return createResult(success,
                       success ? "Connected to " + address : "",
                       success ? "" : "Failed to connect");
}

OperationResult AntiDetectEngine::disconnect() {
    bool success = m_impl->m_adbManager.disconnectAll();
    return createResult(success, success ? "Disconnected" : "", "");
}

OperationResult AntiDetectEngine::selectDevice(const std::string& serial) {
    bool success = m_impl->m_adbManager.selectDevice(serial);
    return createResult(success,
                       success ? "Selected " + serial : "",
                       success ? "" : "Device not found");
}

OperationResult AntiDetectEngine::spoofManufacturer(const std::string& manufacturer) {
    auto result = m_impl->m_deviceFingerprint->spoofDeviceManufacturer(manufacturer);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofModel(const std::string& model) {
    auto result = m_impl->m_deviceFingerprint->spoofDeviceModel(model);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofBrand(const std::string& brand) {
    auto result = m_impl->m_deviceFingerprint->spoofDeviceBrand(brand);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofAndroidVersion(const std::string& version) {
    auto result = m_impl->m_deviceFingerprint->spoofAndroidVersion(version);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofBuildFingerprint(const std::string& fingerprint) {
    auto result = m_impl->m_deviceFingerprint->spoofBuildFingerprint(fingerprint);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofDeviceId(const std::string& deviceId) {
    auto result = m_impl->m_advancedSpoofing->spoofDeviceId(deviceId);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofAndroidId(const std::string& androidId) {
    auto result = m_impl->m_advancedSpoofing->spoofAndroidId(androidId);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofSerialNumber(const std::string& serial) {
    auto result = m_impl->m_advancedSpoofing->spoofSerialNumber(serial);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofScreenResolution(int width, int height) {
    auto result = m_impl->m_deviceFingerprint->spoofScreenResolution(width, height);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofScreenDensity(int density) {
    auto result = m_impl->m_deviceFingerprint->spoofScreenDensity(density);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofMACAddress(const std::string& mac) {
    auto result = m_impl->m_networkSpoofer->spoofMACAddress(mac);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofCarrier(const std::string& carrier) {
    auto result = m_impl->m_networkSpoofer->spoofCarrierName(carrier);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofCountry(const std::string& country) {
    auto result = m_impl->m_networkSpoofer->spoofCarrierCountry(country);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofLocation(double latitude, double longitude) {
    auto result = m_impl->m_networkSpoofer->spoofLocation(latitude, longitude);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::enableMockLocation() {
    auto result = m_impl->m_networkSpoofer->enableMockLocation();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofUserAgent(const std::string& userAgent) {
    auto result = m_impl->m_advancedSpoofing->spoofUserAgent(userAgent);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofWebRTCIP(const std::string& ip) {
    auto result = m_impl->m_advancedSpoofing->spoofWebRTCLocalIP(ip);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofCPUModel(const std::string& cpu) {
    auto result = m_impl->m_advancedSpoofing->spoofCPUModel(cpu);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofGPU(const std::string& gpu) {
    auto result = m_impl->m_advancedSpoofing->spoofGPURenderer(gpu);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofTotalMemory(int memoryMB) {
    auto result = m_impl->m_advancedSpoofing->spoofTotalMemory(memoryMB);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofAccelerometer(float x, float y, float z) {
    auto result = m_impl->m_advancedSpoofing->spoofAccelerometer(x, y, z);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofGyroscope(float x, float y, float z) {
    auto result = m_impl->m_advancedSpoofing->spoofGyroscope(x, y, z);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofMagnetometer(float x, float y, float z) {
    auto result = m_impl->m_advancedSpoofing->spoofMagnetometer(x, y, z);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofLightSensor(float lux) {
    auto result = m_impl->m_advancedSpoofing->spoofLightSensor(lux);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::setTimezone(const std::string& timezone) {
    auto result = m_impl->m_systemManager->setTimezone(timezone);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::setLocale(const std::string& locale) {
    auto result = m_impl->m_systemManager->setLocale(locale);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::setLanguage(const std::string& language) {
    auto result = m_impl->m_systemManager->setLanguage(language);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::setBatteryStatus(int level, const std::string& status) {
    auto result = m_impl->m_systemManager->setBatteryStatus(level, status, "USB");
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::enableSELinux() {
    auto result = m_impl->m_systemManager->enableSELinux();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::disableSELinux() {
    auto result = m_impl->m_systemManager->disableSELinux();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::enableDebugMode() {
    auto result = m_impl->m_systemManager->enableDebugMode();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::disableDebugMode() {
    auto result = m_impl->m_systemManager->disableDebugMode();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofWidevineLevel(int level) {
    auto result = m_impl->m_advancedSpoofing->spoofWidevineLevel(level);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofHDCPLevel(const std::string& level) {
    auto result = m_impl->m_advancedSpoofing->spoofHDCPLevel(level);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::enableBasicIntegrity() {
    auto result = m_impl->m_advancedSpoofing->enableBasicIntegrity();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::enableDeviceIntegrity() {
    auto result = m_impl->m_advancedSpoofing->enableDeviceIntegrity();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::enableNoCtsMismatch() {
    auto result = m_impl->m_advancedSpoofing->enableNoCtsMismatch();
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::spoofSafetyNet(const std::map<std::string, std::string>& response) {
    auto result = m_impl->m_advancedSpoofing->spoofSafetyNetResponse(response);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::blockTracker(const std::string& domain) {
    auto result = m_impl->m_advancedSpoofing->blockTracker(domain);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::unblockTracker(const std::string& domain) {
    auto result = m_impl->m_advancedSpoofing->unblockTracker(domain);
    return createResult(result.success, result.message, result.error);
}

OperationResult AntiDetectEngine::loadBlocklist(const std::string& filepath) {
    auto result = m_impl->m_advancedSpoofing->loadBlocklist(filepath);
    return createResult(result.success, result.message, result.error);
}

std::vector<ProfileInfo> AntiDetectEngine::listProfiles() {
    std::vector<ProfileInfo> profiles;
    auto profileList = m_impl->m_profileManager->listProfiles();
    
    for (const auto& p : profileList) {
        ProfileInfo info;
        info.id = p.id;
        info.name = p.name;
        info.description = p.description;
        info.category = p.category;
        info.createdAt = p.createdAt;
        info.modifiedAt = p.modifiedAt;
        profiles.push_back(info);
    }
    
    return profiles;
}

OperationResult AntiDetectEngine::applyProfile(const std::string& profileId) {
    bool success = m_impl->m_profileManager->applyProfile(profileId);
    return createResult(success,
                       success ? "Profile applied" : "",
                       success ? "" : "Profile not found");
}

OperationResult AntiDetectEngine::createProfile(const std::string& name, const std::map<std::string, std::string>& data) {
    auto profile = m_impl->m_profileManager->createDefaultProfile(name, "Custom");
    bool success = m_impl->m_profileManager->createProfile(profile);
    return createResult(success, success ? "Profile created" : "", "");
}

OperationResult AntiDetectEngine::deleteProfile(const std::string& profileId) {
    bool success = m_impl->m_profileManager->deleteProfile(profileId);
    return createResult(success, success ? "Profile deleted" : "", "");
}

OperationResult AntiDetectEngine::exportProfiles(const std::string& directory) {
    bool success = m_impl->m_profileManager->exportAllProfiles(directory);
    return createResult(success, success ? "Profiles exported" : "", "");
}

OperationResult AntiDetectEngine::importProfiles(const std::string& directory) {
    m_impl->m_profileManager->importAllProfiles(directory);
    return createResult(true, "Profiles imported", "");
}

OperationResult AntiDetectEngine::resetAll() {
    m_impl->m_deviceFingerprint->resetToFactory();
    m_impl->m_networkSpoofer->resetAll();
    m_impl->m_systemManager->resetAllChanges();
    m_impl->m_advancedSpoofing->resetAll();
    m_impl->m_profileManager->resetToOriginal();
    return createResult(true, "All spoofing reset", "");
}

OperationResult AntiDetectEngine::backupState() {
    m_impl->m_systemManager->backupSystemState();
    return createResult(true, "State backed up", "");
}

OperationResult AntiDetectEngine::restoreState() {
    m_impl->m_systemManager->restoreSystemState();
    return createResult(true, "State restored", "");
}

std::map<std::string, std::string> AntiDetectEngine::getCurrentFingerprint() {
    return m_impl->m_deviceFingerprint->getCurrentFingerprint();
}

std::map<std::string, std::string> AntiDetectEngine::getSystemStatus() {
    return m_impl->m_systemManager->getSystemInfo();
}

std::map<std::string, std::string> AntiDetectEngine::getDeviceInfo() {
    std::map<std::string, std::string> info;
    auto devices = listDevices();
    if (!devices.empty()) {
        info["serial"] = devices[0].serial;
        info["model"] = devices[0].model;
        info["state"] = devices[0].state;
    }
    return info;
}

}
