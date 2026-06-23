#include "Config.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <ctime>

namespace AntiDetect {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

Config::Config() {
    m_deviceConfig = {
        .manufacturer = "Samsung",
        .brand = "Samsung",
        .model = "SM-G998B",
        .device = "o1s",
        .product = "o1sxx",
        .hardware = "exynos2100",
        .board = "exynos2100",
        .deviceName = "Galaxy S21 Ultra",
        .androidVersion = "13",
        .sdkVersion = "33",
        .securityPatch = "2023-08-01",
        .buildId = "SP1A.210812.016.G998BXXU5EWH5",
        .buildFingerprint = "samsung/o1sxx/o1s:13/SP1A.210812.016.G998BXXU5EWH5:user/release-keys",
        .screenWidth = 1440,
        .screenHeight = 3200,
        .screenDensity = 640,
        .screenDensityDpi = "640dpi",
        .bootloader = "G998BXXU5EWH5",
        .radioVersion = "G998BXXU5EWH5",
        .gpuVendor = "ARM",
        .gpuRenderer = "Mali-G78",
        .openglVersion = "OpenGL ES 3.2 v1.r22p0-01rel0.f294e54ef2d89184852c0e67a1b1c5f7"
    };
    
    m_networkConfig = {
        .macAddress = "AA:BB:CC:DD:EE:FF",
        .wifiMacAddress = "AA:BB:CC:DD:EE:FF",
        .bluetoothMac = "AA:BB:CC:DD:EE:FF",
        .ethernetMac = "AA:BB:CC:DD:EE:FF",
        .carrierName = "Verizon",
        .carrierCountry = "US",
        .simOperator = "311480",
        .networkType = "5G",
        .latitude = 40.7128,
        .longitude = -74.0060,
        .locationSpoofing = false
    };
    
    m_systemConfig = {
        .disableSelinux = false,
        .enableDebug = false,
        .mockLocation = false,
        .allowAdbAuth = true,
        .timezone = "America/New_York",
        .locale = "en_US",
        .language = "en",
        .batteryLevel = 85,
        .batteryStatus = "Discharging",
        .chargingType = "USB"
    };
}

Config::~Config() {
}

void Config::setDeviceConfig(const DeviceConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_deviceConfig = config;
}

DeviceConfig Config::getDeviceConfig() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_deviceConfig;
}

void Config::setNetworkConfig(const NetworkConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_networkConfig = config;
}

NetworkConfig Config::getNetworkConfig() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_networkConfig;
}

void Config::setSystemConfig(const SystemConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_systemConfig = config;
}

SystemConfig Config::getSystemConfig() const {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_systemConfig;
}

std::map<std::string, FingerprintProfile>& Config::getProfiles() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_profiles;
}

bool Config::addProfile(const FingerprintProfile& profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profiles[profile.profileId] = profile;
    return true;
}

bool Config::removeProfile(const std::string& profileId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_profiles.erase(profileId) > 0;
}

FingerprintProfile* Config::getProfile(const std::string& profileId) {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_profiles.find(profileId);
    if (it != m_profiles.end()) {
        return &it->second;
    }
    return nullptr;
}

bool Config::loadFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        Logger::getInstance().error("Failed to open config file: " + filepath);
        return false;
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return jsonToConfig(buffer.str());
}

bool Config::saveToFile(const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        Logger::getInstance().error("Failed to save config file: " + filepath);
        return false;
    }
    
    file << configToJson();
    file.close();
    
    Logger::getInstance().info("Config saved to: " + filepath);
    return true;
}

std::string Config::configToJson() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    std::stringstream ss;
    ss << "{\n";
    ss << "  \"device\": {\n";
    ss << "    \"manufacturer\": \"" << m_deviceConfig.manufacturer << "\",\n";
    ss << "    \"brand\": \"" << m_deviceConfig.brand << "\",\n";
    ss << "    \"model\": \"" << m_deviceConfig.model << "\",\n";
    ss << "    \"device\": \"" << m_deviceConfig.device << "\",\n";
    ss << "    \"product\": \"" << m_deviceConfig.product << "\",\n";
    ss << "    \"hardware\": \"" << m_deviceConfig.hardware << "\",\n";
    ss << "    \"board\": \"" << m_deviceConfig.board << "\",\n";
    ss << "    \"androidVersion\": \"" << m_deviceConfig.androidVersion << "\",\n";
    ss << "    \"sdkVersion\": \"" << m_deviceConfig.sdkVersion << "\",\n";
    ss << "    \"securityPatch\": \"" << m_deviceConfig.securityPatch << "\",\n";
    ss << "    \"buildId\": \"" << m_deviceConfig.buildId << "\",\n";
    ss << "    \"buildFingerprint\": \"" << m_deviceConfig.buildFingerprint << "\",\n";
    ss << "    \"screenWidth\": " << m_deviceConfig.screenWidth << ",\n";
    ss << "    \"screenHeight\": " << m_deviceConfig.screenHeight << ",\n";
    ss << "    \"screenDensity\": " << m_deviceConfig.screenDensity << "\n";
    ss << "  },\n";
    ss << "  \"network\": {\n";
    ss << "    \"macAddress\": \"" << m_networkConfig.macAddress << "\",\n";
    ss << "    \"carrierName\": \"" << m_networkConfig.carrierName << "\",\n";
    ss << "    \"latitude\": " << m_networkConfig.latitude << ",\n";
    ss << "    \"longitude\": " << m_networkConfig.longitude << ",\n";
    ss << "    \"locationSpoofing\": " << (m_networkConfig.locationSpoofing ? "true" : "false") << "\n";
    ss << "  },\n";
    ss << "  \"system\": {\n";
    ss << "    \"timezone\": \"" << m_systemConfig.timezone << "\",\n";
    ss << "    \"locale\": \"" << m_systemConfig.locale << "\",\n";
    ss << "    \"mockLocation\": " << (m_systemConfig.mockLocation ? "true" : "false") << "\n";
    ss << "  }\n";
    ss << "}\n";
    
    return ss.str();
}

bool Config::jsonToConfig(const std::string& json) {
    if (json.empty()) {
        return false;
    }
    
    auto extractString = [&](const std::string& jsonStr, const std::string& key) -> std::string {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = jsonStr.find(searchKey);
        if (pos == std::string::npos) return "";
        
        size_t colonPos = jsonStr.find(':', pos);
        if (colonPos == std::string::npos) return "";
        
        size_t valueStart = jsonStr.find('"', colonPos);
        if (valueStart == std::string::npos) return "";
        valueStart++;
        
        size_t valueEnd = valueStart;
        while (valueEnd < jsonStr.length()) {
            if (jsonStr[valueEnd] == '"' && jsonStr[valueEnd-1] != '\\') {
                break;
            }
            valueEnd++;
        }
        
        return jsonStr.substr(valueStart, valueEnd - valueStart);
    };
    
    auto extractBool = [&](const std::string& jsonStr, const std::string& key) -> bool {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = jsonStr.find(searchKey);
        if (pos == std::string::npos) return false;
        
        size_t colonPos = jsonStr.find(':', pos);
        if (colonPos == std::string::npos) return false;
        
        size_t valueStart = jsonStr.find_first_not_of(" \t", colonPos + 1);
        if (valueStart == std::string::npos) return false;
        
        return jsonStr.substr(valueStart, 4) == "true";
    };
    
    auto extractInt = [&](const std::string& jsonStr, const std::string& key) -> int {
        std::string searchKey = "\"" + key + "\"";
        size_t pos = jsonStr.find(searchKey);
        if (pos == std::string::npos) return 0;
        
        size_t colonPos = jsonStr.find(':', pos);
        if (colonPos == std::string::npos) return 0;
        
        size_t valueStart = jsonStr.find_first_not_of(" \t", colonPos + 1);
        if (valueStart == std::string::npos) return 0;
        
        size_t valueEnd = valueStart;
        while (valueEnd < jsonStr.length() && isdigit(jsonStr[valueEnd])) {
            valueEnd++;
        }
        
        try {
            return std::stoi(jsonStr.substr(valueStart, valueEnd - valueStart));
        } catch (...) {
            return 0;
        }
    };
    
    try {
        m_deviceConfig.manufacturer = extractString(json, "manufacturer");
        m_deviceConfig.brand = extractString(json, "brand");
        m_deviceConfig.model = extractString(json, "model");
        m_deviceConfig.androidVersion = extractString(json, "androidVersion");
        m_deviceConfig.screenWidth = extractInt(json, "screenWidth");
        m_deviceConfig.screenHeight = extractInt(json, "screenHeight");
        m_deviceConfig.screenDensity = extractInt(json, "screenDensity");
        
        m_networkConfig.macAddress = extractString(json, "macAddress");
        m_networkConfig.carrierName = extractString(json, "carrierName");
        m_networkConfig.latitude = std::stod(extractString(json, "latitude"));
        m_networkConfig.longitude = std::stod(extractString(json, "longitude"));
        m_networkConfig.locationSpoofing = extractBool(json, "locationSpoofing");
        
        m_systemConfig.timezone = extractString(json, "timezone");
        m_systemConfig.locale = extractString(json, "locale");
        m_systemConfig.mockLocation = extractBool(json, "mockLocation");
        
        Logger::getInstance().info("Configuration loaded from JSON");
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("Config JSON parsing error: " + std::string(e.what()));
        return false;
    }
}

}
