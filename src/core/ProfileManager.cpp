/**
 * ProfileManager - Device Profile Generation & Management
 */

#include "core/ProfileManager.hpp"
#include "core/CryptoUtils.hpp"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <random>
#include <openssl/rand.h>

namespace VirtualPhonePro {

ProfileManager::ProfileManager() {}

ProfileManager::~ProfileManager() {}

ProfileManager& ProfileManager::getInstance() {
    static ProfileManager instance;
    return instance;
}

bool ProfileManager::initialize() {
    m_database.initializeDatabase();
    return true;
}

void ProfileManager::shutdown() {
    // Cleanup
}

FingerprintConfig ProfileManager::generateRandomProfile(const std::string& region) {
    std::lock_guard<std::mutex> lock(m_profileMutex);
    
    FingerprintConfig profile;
    
    // Get random device from database
    auto device = m_database.getRandom();
    if (device) {
        profile.manufacturer = device->manufacturer;
        profile.model = device->model;
        profile.brand = device->manufacturer;
        profile.cpuModel = device->cpuModel;
        profile.cpuABI = "arm64-v8a";
        profile.cpuCores = 8;
        profile.gpuVendor = "Qualcomm";
        profile.gpuRenderer = device->gpuModel;
        profile.screenWidth = device->screenWidth;
        profile.screenHeight = device->screenHeight;
        profile.screenDPI = device->screenDPI;
    } else {
        // Fallback to Samsung Galaxy S21
        profile.manufacturer = "Samsung";
        profile.model = "SM-G991B";
        profile.brand = "Samsung";
        profile.cpuModel = "Exynos 2100";
        profile.cpuABI = "arm64-v8a";
        profile.cpuCores = 8;
        profile.gpuVendor = "ARM";
        profile.gpuRenderer = "Mali-G78 MP14";
        profile.screenWidth = 1080;
        profile.screenHeight = 2400;
        profile.screenDPI = 421;
    }
    
    // Generate random but realistic values
    profile.androidVersion = "13";
    profile.securityPatch = "2024-01-01";
    profile.buildId = generateUniqueBuildId(profile.manufacturer);
    profile.bootloader = generateBootloader(profile.model);
    profile.hardware = profile.manufacturer == "Samsung" ? "exynos2100" : "qcom";
    profile.board = "O1";
    profile.device = profile.model.substr(0, 4).tolower();
    profile.product = profile.model.substr(0, 3).tolower();
    
    // Generate fingerprint
    profile.fingerprint = generateUniqueFingerprint(profile);
    
    // Network
    profile.macAddress = generateMACAddress(profile.manufacturer);
    profile.carrier = "Carrier";
    profile.countryCode = region.empty() ? "US" : region;
    
    // Location
    if (region == "BD") {
        profile.latitude = 23.8103;
        profile.longitude = 90.4125;
        profile.timezone = "Asia/Dhaka";
    } else if (region == "IN") {
        profile.latitude = 28.6139;
        profile.longitude = 77.2090;
        profile.timezone = "Asia/Kolkata";
    } else {
        profile.latitude = 37.7749;
        profile.longitude = -122.4194;
        profile.timezone = "America/Los_Angeles";
    }
    
    profile.locale = "en_" + profile.countryCode;
    
    // Sensor using secure random
    profile.sensorModel = "BMI" + std::to_string(Crypto::SecureRandomGenerator().generateUint32() % 200 + 100);
    
    return profile;
}

FingerprintConfig ProfileManager::generateProfile(const std::string& manufacturer,
                                                const std::string& model,
                                                const std::string& region) {
    FingerprintConfig profile;
    
    // Find device in database
    auto devices = m_database.getByManufacturer(manufacturer);
    DeviceProfile targetDevice;
    bool found = false;
    
    for (const auto& device : devices) {
        if (device.model == model || device.codename == model) {
            targetDevice = device;
            found = true;
            break;
        }
    }
    
    if (!found) {
        // Use first match or default
        if (!devices.empty()) {
            targetDevice = devices[0];
        }
    }
    
    profile.manufacturer = targetDevice.manufacturer;
    profile.model = targetDevice.model;
    profile.brand = targetDevice.manufacturer;
    profile.androidVersion = "13";
    profile.securityPatch = "2024-01-01";
    profile.buildId = generateUniqueBuildId(profile.manufacturer);
    profile.bootloader = generateBootloader(profile.model);
    profile.hardware = "qcom";
    profile.board = "O1";
    profile.device = targetDevice.codename;
    profile.product = targetDevice.codename;
    profile.fingerprint = generateUniqueFingerprint(profile);
    profile.macAddress = generateMACAddress(profile.manufacturer);
    profile.cpuModel = targetDevice.cpuModel;
    profile.cpuABI = "arm64-v8a";
    profile.cpuCores = 8;
    profile.gpuVendor = "Qualcomm";
    profile.gpuRenderer = targetDevice.gpuModel;
    profile.screenWidth = targetDevice.screenWidth;
    profile.screenHeight = targetDevice.screenHeight;
    profile.screenDPI = targetDevice.screenDPI;
    
    return profile;
}

FingerprintConfig ProfileManager::generateFromTemplate(const std::string& templateId) {
    auto profile = generateRandomProfile();
    profile.model = templateId;
    profile.fingerprint = generateUniqueFingerprint(profile);
    return profile;
}

FingerprintConfig ProfileManager::randomizeFingerprint(const FingerprintConfig& base) {
    FingerprintConfig result = base;
    Crypto::SecureRandomGenerator rng;
    
    // Randomize build ID
    result.buildId = generateUniqueBuildId(result.manufacturer);
    
    // Randomize serial number
    result.bootloader = generateBootloader(result.model);
    
    // Randomize MAC
    result.macAddress = generateMACAddress();
    
    // Randomize some numeric values slightly using secure random
    result.screenWidth += (rng.generateUint32() % 100) - 50;
    result.screenHeight += (rng.generateUint32() % 100) - 50;
    result.screenDPI += (rng.generateUint32() % 20) - 10;
    
    // Regenerate fingerprint
    result.fingerprint = generateUniqueFingerprint(result);
    
    return result;
}

FingerprintConfig ProfileManager::applyRegionVariations(const FingerprintConfig& base,
                                                       const std::string& region) {
    FingerprintConfig result = base;
    result.countryCode = region;
    result.locale = "en_" + region;
    
    // Apply region-specific carrier using secure random
    auto carriers = m_database.getCarrierProfiles(region);
    if (!carriers.empty()) {
        auto carrier = carriers[Crypto::SecureRandomGenerator().generateUint32() % carriers.size()];
        result.carrier = carrier.name;
    }
    
    return result;
}

std::vector<DeviceProfile> ProfileManager::getAllDeviceProfiles() {
    return m_database.getAll();
}

std::vector<DeviceProfile> ProfileManager::getDeviceProfilesByManufacturer(const std::string& manufacturer) {
    return m_database.getByManufacturer(manufacturer);
}

std::vector<CarrierProfile> ProfileManager::getCarrierProfiles(const std::string& country) {
    return {}; // Placeholder
}

std::vector<RegionProfile> ProfileManager::getAllRegionProfiles() {
    return {}; // Placeholder
}

std::optional<DeviceProfile> ProfileManager::getDeviceProfile(const std::string& id) {
    return m_database.getById(id);
}

bool ProfileManager::saveProfile(const FingerprintConfig& profile, const std::string& id) {
    std::lock_guard<std::mutex> lock(m_profileMutex);
    
    std::string path = "./profiles/" + id + ".json";
    
    // Create directory if not exists
    mkdir("./profiles", 0755);
    
    // Save to file
    std::ofstream file(path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{";
    file << "\"id\":\"" << id << "\",";
    file << "\"manufacturer\":\"" << profile.manufacturer << "\",";
    file << "\"model\":\"" << profile.model << "\",";
    file << "\"androidVersion\":\"" << profile.androidVersion << "\",";
    file << "\"fingerprint\":\"" << profile.fingerprint << "\"";
    file << "}";
    
    file.close();
    return true;
}

std::optional<FingerprintConfig> ProfileManager::loadProfile(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_profileMutex);
    
    std::string path = "./profiles/" + id + ".json";
    std::ifstream file(path);
    
    if (!file.is_open()) {
        return std::nullopt;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    
    FingerprintConfig profile;
    profile.model = id;
    
    file.close();
    return profile;
}

std::vector<FingerprintConfig> ProfileManager::listSavedProfiles() {
    return {}; // Placeholder
}

bool ProfileManager::deleteProfile(const std::string& id) {
    std::lock_guard<std::mutex> lock(m_profileMutex);
    
    std::string path = "./profiles/" + id + ".json";
    return remove(path.c_str()) == 0;
}

bool ProfileManager::validateFingerprint(const FingerprintConfig& profile) {
    return !profile.fingerprint.empty() && !profile.model.empty();
}

std::string ProfileManager::calculateFingerprintHash(const FingerprintConfig& profile) {
    std::string data = profile.fingerprint + profile.model + profile.manufacturer;
    return hashString(data);
}

std::string ProfileManager::generateUniqueBuildId(const std::string& manufacturer) {
    Crypto::SecureRandomGenerator rng;
    std::string prefix;
    if (manufacturer == "Samsung") prefix = "SP1A";
    else if (manufacturer == "Google") prefix = "TQ1A";
    else if (manufacturer == "Xiaomi") prefix = "V14";
    else prefix = "RP1A";
    
    char dateBuf[5];
    snprintf(dateBuf, sizeof(dateBuf), "%02d", (rng.generateUint32() % 12) + 1);
    std::string date = std::to_string(2024) + dateBuf;
    std::string suffix = std::to_string(rng.generateUint32() % 9000 + 1000);
    
    return prefix + "." + date + suffix;
}

std::string ProfileManager::generateUniqueFingerprint(const FingerprintConfig& config) {
    std::stringstream ss;
    ss << config.manufacturer << "/" << config.product << "/" << config.device << ":"
       << config.androidVersion << "/" << config.buildId << "/" << config.bootloader << ":"
       << config.hardware << "/" << config.board;
    
    return ss.str();
}

std::string ProfileManager::generateBootloader(const std::string& model) {
    Crypto::SecureRandomGenerator rng;
    std::string prefix = model.substr(0, 4);
    std::string version = std::to_string(rng.generateUint32() % 10 + 1);
    std::string letter = std::string(1, 'A' + (rng.generateUint32() % 26));
    
    return prefix + "U" + version + letter;
}

std::string ProfileManager::generateSerialNumber() {
    Crypto::SecureRandomGenerator rng;
    std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string serial;
    
    for (int i = 0; i < 12; i++) {
        serial += chars[rng.generateUint32() % chars.length()];
    }
    
    return serial;
}

std::string ProfileManager::generateMACAddress(const std::string& prefix) {
    Crypto::SecureRandomGenerator rng;
    std::string mac;
    
    if (prefix == "Samsung") {
        mac = "A0:CE:C8";
    } else if (prefix == "Google") {
        mac = "4C:8D:79";
    } else if (prefix == "Xiaomi") {
        mac = "64:09:80";
    } else {
        mac = "00:1A:2B";
    }
    
    // Add random bytes using secure random
    for (int i = 0; i < 3; i++) {
        mac += ":" + std::format("{:02X}", rng.generateUint32() % 256);
    }
    
    return mac;
}

std::map<std::string, std::string> ProfileManager::generateSensorData() {
    Crypto::SecureRandomGenerator rng;
    std::map<std::string, std::string> sensors;
    
    // Accelerometer using secure random
    sensors["accelerometer"] = std::to_string((rng.generateUint32() % 200 - 100) / 100.0) + "," +
                               std::to_string((rng.generateUint32() % 200 - 100) / 100.0) + "," +
                               std::to_string(9.8 + (rng.generateUint32() % 20 - 10) / 100.0);
    
    // Gyroscope
    sensors["gyroscope"] = std::to_string((rng.generateUint32() % 100 - 50) / 100.0) + "," +
                           std::to_string((rng.generateUint32() % 100 - 50) / 100.0) + "," +
                           std::to_string((rng.generateUint32() % 100 - 50) / 100.0);
    
    // Magnetometer
    sensors["magnetometer"] = std::to_string((rng.generateUint32() % 600 - 300) / 10.0) + "," +
                               std::to_string((rng.generateUint32() % 600 - 300) / 10.0) + "," +
                               std::to_string((rng.generateUint32() % 600 - 300) / 10.0);
    
    return sensors;
}

} // namespace VirtualPhonePro
