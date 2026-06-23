#include "AntiDetectCore.hpp"
#include "ADBManager.hpp"
#include "DeviceFingerprint.hpp"
#include "NetworkSpoofer.hpp"
#include "SystemManager.hpp"
#include "ProfileManager.hpp"
#include "Logger.hpp"
#include "Config.hpp"
#include <iostream>

namespace AntiDetect {

AntiDetectCore& AntiDetectCore::getInstance() {
    static AntiDetectCore instance;
    return instance;
}

AntiDetectCore::AntiDetectCore()
    : m_status(AntiDetectStatus::UNINITIALIZED)
{
}

AntiDetectCore::~AntiDetectCore() {
    shutdown();
}

bool AntiDetectCore::initialize() {
    return initialize("");
}

bool AntiDetectCore::initialize(const std::string& configPath) {
    if (m_status != AntiDetectStatus::UNINITIALIZED) {
        Logger::getInstance().warning("AntiDetectCore already initialized");
        return true;
    }
    
    m_status = AntiDetectStatus::INITIALIZING;
    
    Logger::getInstance().info("==============================================");
    Logger::getInstance().info("   AntiDetectPro v" + std::string(VERSION));
    Logger::getInstance().info("   Enterprise Anti-Detect System");
    Logger::getInstance().info("==============================================");
    
    initializeComponents();
    
    m_status = AntiDetectStatus::READY;
    
    Logger::getInstance().info("AntiDetectPro initialized successfully");
    
    return true;
}

void AntiDetectCore::initializeComponents() {
    m_adbManager = std::make_unique<ADBManager>();
    m_deviceFingerprint = std::make_unique<DeviceFingerprint>();
    m_networkSpoofer = std::make_unique<NetworkSpoofer>();
    m_systemManager = std::make_unique<SystemManager>();
    m_profileManager = std::make_unique<ProfileManager>();
    
    Logger::getInstance().info("Initializing ADB Manager...");
    if (!m_adbManager->initialize()) {
        Logger::getInstance().warning("ADB initialization failed - some features may be unavailable");
    }
    
    if (m_adbManager->isConnected()) {
        Logger::getInstance().info("Initializing Device Fingerprint Engine...");
        m_deviceFingerprint->initialize();
        
        Logger::getInstance().info("Initializing Network Spoofer...");
        m_networkSpoofer->initialize();
        
        Logger::getInstance().info("Initializing System Manager...");
        m_systemManager->initialize();
    }
    
    Logger::getInstance().info("Initializing Profile Manager...");
    m_profileManager->initialize();
}

void AntiDetectCore::cleanupComponents() {
    m_profileManager.reset();
    m_systemManager.reset();
    m_networkSpoofer.reset();
    m_deviceFingerprint.reset();
    m_adbManager.reset();
}

bool AntiDetectCore::shutdown() {
    if (m_status == AntiDetectStatus::UNINITIALIZED) {
        return true;
    }
    
    Logger::getInstance().info("Shutting down AntiDetectPro...");
    
    cleanupComponents();
    
    m_status = AntiDetectStatus::UNINITIALIZED;
    
    Logger::getInstance().info("AntiDetectPro shutdown complete");
    
    return true;
}

AntiDetectStatus AntiDetectCore::getStatus() const {
    return m_status;
}

std::string AntiDetectCore::getVersion() const {
    return VERSION;
}

AntiDetectResult AntiDetectCore::createResult(bool success, const std::string& message, const std::string& error) {
    AntiDetectResult result;
    result.success = success;
    result.message = message;
    result.error = error;
    return result;
}

AntiDetectResult AntiDetectCore::connect(const std::string& deviceAddress) {
    if (!m_adbManager) {
        return createResult(false, "", "ADB Manager not initialized");
    }
    
    bool success = m_adbManager->connect(deviceAddress);
    
    if (success) {
        m_deviceFingerprint->initialize();
        m_networkSpoofer->initialize();
        m_systemManager->initialize();
        
        m_status = AntiDetectStatus::RUNNING;
    }
    
    return createResult(success, 
                        success ? "Connected to device: " + deviceAddress : "",
                        success ? "" : m_adbManager->getLastError());
}

AntiDetectResult AntiDetectCore::disconnect() {
    if (!m_adbManager) {
        return createResult(false, "", "ADB Manager not initialized");
    }
    
    bool success = m_adbManager->disconnectAll();
    m_status = AntiDetectStatus::READY;
    
    return createResult(success, success ? "Disconnected from device" : "");
}

std::vector<std::string> AntiDetectCore::listDevices() {
    std::vector<std::string> devices;
    
    if (!m_adbManager) {
        return devices;
    }
    
    auto deviceList = m_adbManager->getDevices();
    for (const auto& device : deviceList) {
        devices.push_back(device.serial + " (" + device.state + ")");
    }
    
    return devices;
}

AntiDetectResult AntiDetectCore::selectDevice(const std::string& serial) {
    if (!m_adbManager) {
        return createResult(false, "", "ADB Manager not initialized");
    }
    
    bool success = m_adbManager->selectDevice(serial);
    
    return createResult(success,
                        success ? "Selected device: " + serial : "",
                        success ? "" : "Device not found");
}

AntiDetectResult AntiDetectCore::applyDeviceProfile(const std::map<std::string, std::string>& profile) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    bool success = m_deviceFingerprint->applyFullProfile(profile);
    
    return createResult(success, 
                        success ? "Device profile applied" : "",
                        success ? "" : "Failed to apply profile");
}

AntiDetectResult AntiDetectCore::resetDevice() {
    if (!m_deviceFingerprint || !m_networkSpoofer) {
        return createResult(false, "", "Components not initialized");
    }
    
    m_deviceFingerprint->resetToFactory();
    m_networkSpoofer->resetAll();
    m_systemManager->resetAllChanges();
    
    return createResult(true, "Device reset to original state");
}

AntiDetectResult AntiDetectCore::spoofManufacturer(const std::string& manufacturer) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofDeviceManufacturer(manufacturer);
    
    return createResult(result.success,
                        result.success ? "Manufacturer spoofed to: " + manufacturer : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofModel(const std::string& model) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofDeviceModel(model);
    
    return createResult(result.success,
                        result.success ? "Model spoofed to: " + model : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofBrand(const std::string& brand) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofDeviceBrand(brand);
    
    return createResult(result.success,
                        result.success ? "Brand spoofed to: " + brand : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofAndroidVersion(const std::string& version) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofAndroidVersion(version);
    
    return createResult(result.success,
                        result.success ? "Android version spoofed to: " + version : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofBuildFingerprint(const std::string& fingerprint) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofBuildFingerprint(fingerprint);
    
    return createResult(result.success,
                        result.success ? "Build fingerprint spoofed" : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofScreenResolution(int width, int height) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofScreenResolution(width, height);
    
    std::stringstream ss;
    ss << width << "x" << height;
    
    return createResult(result.success,
                        result.success ? "Screen resolution spoofed to: " + ss.str() : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofScreenDensity(int density) {
    if (!m_deviceFingerprint) {
        return createResult(false, "", "Device Fingerprint not initialized");
    }
    
    auto result = m_deviceFingerprint->spoofScreenDensity(density);
    
    return createResult(result.success,
                        result.success ? "Screen density spoofed to: " + std::to_string(density) : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofMACAddress(const std::string& macAddress) {
    if (!m_networkSpoofer) {
        return createResult(false, "", "Network Spoofer not initialized");
    }
    
    auto result = m_networkSpoofer->spoofMACAddress(macAddress);
    
    return createResult(result.success,
                        result.success ? "MAC address spoofed to: " + macAddress : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofCarrier(const std::string& carrierName) {
    if (!m_networkSpoofer) {
        return createResult(false, "", "Network Spoofer not initialized");
    }
    
    auto result = m_networkSpoofer->spoofCarrierName(carrierName);
    
    return createResult(result.success,
                        result.success ? "Carrier spoofed to: " + carrierName : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::spoofLocation(double latitude, double longitude) {
    if (!m_networkSpoofer) {
        return createResult(false, "", "Network Spoofer not initialized");
    }
    
    auto result = m_networkSpoofer->spoofLocation(latitude, longitude);
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << latitude << ", " << longitude;
    
    return createResult(result.success,
                        result.success ? "Location spoofed to: " + ss.str() : "",
                        result.success ? "" : result.error);
}

AntiDetectResult AntiDetectCore::enableGPSSpoofing() {
    if (!m_networkSpoofer) {
        return createResult(false, "", "Network Spoofer not initialized");
    }
    
    auto result = m_networkSpoofer->enableGPSSpoofing();
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::enableMockLocation() {
    if (!m_networkSpoofer) {
        return createResult(false, "", "Network Spoofer not initialized");
    }
    
    auto result = m_networkSpoofer->enableMockLocation();
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::setTimezone(const std::string& timezone) {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->setTimezone(timezone);
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::setLocale(const std::string& locale) {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->setLocale(locale);
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::setLanguage(const std::string& language) {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->setLanguage(language);
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::setBatteryStatus(int level, const std::string& status) {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->setBatteryStatus(level, status, "USB");
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::enableSELinux() {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->enableSELinux();
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::disableSELinux() {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->disableSELinux();
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::enableDebugMode() {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->enableDebugMode();
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::disableDebugMode() {
    if (!m_systemManager) {
        return createResult(false, "", "System Manager not initialized");
    }
    
    auto result = m_systemManager->disableDebugMode();
    
    return createResult(result.success, result.message, result.error);
}

AntiDetectResult AntiDetectCore::applyProfile(const std::string& profileId) {
    if (!m_profileManager) {
        return createResult(false, "", "Profile Manager not initialized");
    }
    
    bool success = m_profileManager->applyProfile(profileId);
    
    return createResult(success,
                        success ? "Profile applied: " + profileId : "",
                        success ? "" : "Profile not found");
}

AntiDetectResult AntiDetectCore::createProfile(const std::string& name, const std::map<std::string, std::string>& data) {
    if (!m_profileManager) {
        return createResult(false, "", "Profile Manager not initialized");
    }
    
    auto profile = m_profileManager->createDefaultProfile(name, "Custom");
    
    bool success = m_profileManager->createProfile(profile);
    
    return createResult(success,
                        success ? "Profile created: " + name : "",
                        success ? "" : "Failed to create profile");
}

AntiDetectResult AntiDetectCore::deleteProfile(const std::string& profileId) {
    if (!m_profileManager) {
        return createResult(false, "", "Profile Manager not initialized");
    }
    
    bool success = m_profileManager->deleteProfile(profileId);
    
    return createResult(success,
                        success ? "Profile deleted: " + profileId : "",
                        success ? "" : "Profile not found");
}

std::vector<std::map<std::string, std::string>> AntiDetectCore::listProfiles() {
    std::vector<std::map<std::string, std::string>> profileList;
    
    if (!m_profileManager) {
        return profileList;
    }
    
    auto profiles = m_profileManager->listProfiles();
    for (const auto& profile : profiles) {
        std::map<std::string, std::string> profileData;
        profileData["id"] = profile.id;
        profileData["name"] = profile.name;
        profileData["description"] = profile.description;
        profileData["category"] = profile.category;
        profileList.push_back(profileData);
    }
    
    return profileList;
}

AntiDetectResult AntiDetectCore::exportConfiguration(const std::string& filepath) {
    if (!m_profileManager) {
        return createResult(false, "", "Profile Manager not initialized");
    }
    
    bool success = m_profileManager->exportAllProfiles(filepath);
    
    return createResult(success,
                        success ? "Configuration exported to: " + filepath : "",
                        success ? "" : "Failed to export configuration");
}

AntiDetectResult AntiDetectCore::importConfiguration(const std::string& filepath) {
    if (!m_profileManager) {
        return createResult(false, "", "Profile Manager not initialized");
    }
    
    std::string newProfileId;
    bool success = m_profileManager->importProfile(filepath, newProfileId);
    
    return createResult(success,
                        success ? "Configuration imported: " + newProfileId : "",
                        success ? "" : "Failed to import configuration");
}

std::map<std::string, std::string> AntiDetectCore::getDeviceInfo() {
    std::map<std::string, std::string> info;
    
    if (!m_adbManager) {
        return info;
    }
    
    auto devices = m_adbManager->getDevices();
    if (!devices.empty()) {
        info["serial"] = devices[0].serial;
        info["model"] = devices[0].model;
        info["device"] = devices[0].device;
        info["product"] = devices[0].product;
    }
    
    return info;
}

std::map<std::string, std::string> AntiDetectCore::getCurrentFingerprint() {
    std::map<std::string, std::string> fingerprint;
    
    if (m_deviceFingerprint) {
        fingerprint = m_deviceFingerprint->getCurrentFingerprint();
    }
    
    return fingerprint;
}

std::map<std::string, std::string> AntiDetectCore::getSystemStatus() {
    std::map<std::string, std::string> status;
    
    if (m_systemManager) {
        status = m_systemManager->getSystemInfo();
    }
    
    switch (m_status) {
        case AntiDetectStatus::UNINITIALIZED: status["core_status"] = "Uninitialized"; break;
        case AntiDetectStatus::INITIALIZING: status["core_status"] = "Initializing"; break;
        case AntiDetectStatus::READY: status["core_status"] = "Ready"; break;
        case AntiDetectStatus::RUNNING: status["core_status"] = "Running"; break;
        case AntiDetectStatus::ERROR: status["core_status"] = "Error"; break;
        default: status["core_status"] = "Unknown"; break;
    }
    
    return status;
}

}
