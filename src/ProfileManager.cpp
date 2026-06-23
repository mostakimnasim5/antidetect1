#include "ProfileManager.hpp"
#include "ADBManager.hpp"
#include "DeviceFingerprint.hpp"
#include "NetworkSpoofer.hpp"
#include "SystemManager.hpp"
#include "Logger.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <algorithm>
#include <cstdlib>
#include <sys/stat.h>

#ifdef _WIN32
#include <direct.h>
#include <windows.h>
#define mkdir _mkdir
#else
#include <unistd.h>
#include <dirent.h>
#endif

namespace AntiDetect {

const std::string ProfileManager::DEFAULT_PROFILES_DIR = "./profiles";

ProfileManager::ProfileManager()
    : m_activeProfileId("")
    , m_initialized(false)
{
    srand(static_cast<unsigned>(time(nullptr)));
}

ProfileManager::~ProfileManager() {
    if (m_initialized) {
        saveProfilesToDisk();
    }
}

bool ProfileManager::initialize() {
    Logger::getInstance().info("Initializing Profile Manager...");
    
    m_profilesDir = getProfilesDirectory();
    
    struct stat info;
    if (stat(m_profilesDir.c_str(), &info) != 0) {
        mkdir(m_profilesDir.c_str(), 0755);
        Logger::getInstance().info("Created profiles directory: " + m_profilesDir);
    }
    
    if (!loadProfilesFromDisk()) {
        Logger::getInstance().warning("No existing profiles found, creating default profiles");
        createDefaultProfiles();
    }
    
    m_initialized = true;
    Logger::getInstance().info("Profile Manager initialized with " + 
                               std::to_string(m_profiles.size()) + " profiles");
    
    return true;
}

bool ProfileManager::isInitialized() const {
    return m_initialized;
}

std::string ProfileManager::getProfilesDirectory() {
    return DEFAULT_PROFILES_DIR;
}

std::string ProfileManager::getProfileFilepath(const std::string& profileId) {
    return m_profilesDir + "/" + profileId + ".json";
}

bool ProfileManager::fileExists(const std::string& filepath) {
    struct stat buffer;
    return (stat(filepath.c_str(), &buffer) == 0);
}

std::string ProfileManager::readFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        return "";
    }
    
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    
    return buffer.str();
}

bool ProfileManager::writeFile(const std::string& filepath, const std::string& content) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        return false;
    }
    
    file << content;
    file.close();
    
    return true;
}

std::string ProfileManager::generateProfileId() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    std::stringstream ss;
    ss << "profile_" << time << "_" << rand() % 10000;
    
    return ss.str();
}

FingerprintProfile ProfileManager::createDefaultProfile(const std::string& name, const std::string& category) {
    FingerprintProfile profile;
    
    profile.metadata.id = generateProfileId();
    profile.metadata.name = name;
    profile.metadata.description = "Default " + name + " profile";
    profile.metadata.category = category;
    profile.metadata.author = "AntiDetectPro";
    profile.metadata.version = 1;
    profile.metadata.isActive = false;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    profile.metadata.createdAt = ss.str();
    profile.metadata.modifiedAt = ss.str();
    
    if (category == "Samsung") {
        profile.device = {
            "Samsung", "Samsung", "SM-G998B", "o1s", "o1sxx", "exynos2100", "exynos2100",
            "13", "33", "2023-08-01", "SP1A.210812.016", "samsung/o1sxx/o1s:13/SP1A.210812.016:user/release-keys",
            1440, 3200, 640, "640dpi",
            "G998BXXU5EWH5", "G998BXXU5EWH5", "OpenGL ES 3.2 v1.r22p0-01rel0",
            "ARM", "Mali-G78"
        };
        profile.network = {
            "AA:BB:CC:DD:EE:F1", "AA:BB:CC:DD:EE:F1", "AA:BB:CC:DD:EE:F1",
            "Verizon", "US", "311480", "5G",
            40.7128, -74.0060, true
        };
    } else if (category == "Google") {
        profile.device = {
            "Google", "Google", "Pixel 7 Pro", "panther", "pixel_californium", "tensor", "tensor",
            "13", "33", "2023-08-01", "TD1A.220804.031", "google/panther/panther:13/TD1A.220804.031:user/release-keys",
            1440, 3120, 560, "560dpi",
            "pixel7_pro", "tensor_g2",
            "OpenGL ES 3.2 v1.r22p0-01rel0",
            "ARM", "Mali-G710"
        };
        profile.network = {
            "AA:BB:CC:DD:EE:F2", "AA:BB:CC:DD:EE:F2", "AA:BB:CC:DD:EE:F2",
            "T-Mobile", "US", "310260", "5G",
            37.7749, -122.4194, true
        };
    } else if (category == "Xiaomi") {
        profile.device = {
            "Xiaomi", "Xiaomi", "Mi 13 Pro", "nuwa", "nuwa", "kalama", "kalama",
            "13", "33", "2023-08-01", "SKQ1.220824.001", "xiaomi/nuwa/nuwa:13/SKQ1.220824.001:user/release-keys",
            1440, 3200, 600, "600dpi",
            "U13", "MI13",
            "OpenGL ES 3.2 v1.r22p0-01rel0",
            "ARM", "Adreno 740"
        };
        profile.network = {
            "AA:BB:CC:DD:EE:F3", "AA:BB:CC:DD:EE:F3", "AA:BB:CC:DD:EE:F3",
            "China Mobile", "CN", "46000", "5G",
            39.9042, 116.4074, true
        };
    } else if (category == "OnePlus") {
        profile.device = {
            "OnePlus", "OnePlus", "OnePlus 11", "CPH2451", "CPH2451", "kalama", "kalama",
            "13", "33", "2023-08-01", "OPL2112", "OnePlus/CPH2451/CPH2451:13/OPM21121211:user/release-keys",
            1440, 3216, 640, "640dpi",
            "OP2451", "OP11",
            "OpenGL ES 3.2 v1.r22p0-01rel0",
            "ARM", "Adreno 740"
        };
        profile.network = {
            "AA:BB:CC:DD:EE:F4", "AA:BB:CC:DD:EE:F4", "AA:BB:CC:DD:EE:F4",
            "AT&T", "US", "310410", "5G",
            32.7767, -96.7970, true
        };
    } else {
        profile.device = {
            "Samsung", "Samsung", "SM-G998B", "o1s", "o1sxx", "exynos2100", "exynos2100",
            "13", "33", "2023-08-01", "SP1A.210812.016", "samsung/o1sxx/o1s:13/SP1A.210812.016:user/release-keys",
            1440, 3200, 640, "640dpi",
            "G998BXXU5EWH5", "G998BXXU5EWH5", "OpenGL ES 3.2 v1.r22p0-01rel0",
            "ARM", "Mali-G78"
        };
        profile.network = {
            "AA:BB:CC:DD:EE:FF", "AA:BB:CC:DD:EE:FF", "AA:BB:CC:DD:EE:FF",
            "Verizon", "US", "311480", "5G",
            40.7128, -74.0060, false
        };
    }
    
    profile.system = {
        true, false, false,
        "America/New_York", "en_US", "en",
        85, "Discharging", "USB"
    };
    
    return profile;
}

void ProfileManager::createDefaultProfiles() {
    std::vector<std::string> categories = {"Samsung", "Google", "Xiaomi", "OnePlus", "Generic"};
    std::vector<std::string> descriptions = {
        "Samsung Galaxy S21 Ultra configuration",
        "Google Pixel 7 Pro configuration",
        "Xiaomi Mi 13 Pro configuration",
        "OnePlus 11 configuration",
        "Generic Android device configuration"
    };
    
    for (size_t i = 0; i < categories.size(); ++i) {
        FingerprintProfile profile = createDefaultProfile(categories[i], categories[i]);
        profile.metadata.description = descriptions[i];
        m_profiles.push_back(profile);
    }
    
    saveProfilesToDisk();
}

std::vector<ProfileMetadata> ProfileManager::listProfiles() {
    std::vector<ProfileMetadata> metadata;
    
    for (const auto& profile : m_profiles) {
        metadata.push_back(profile.metadata);
    }
    
    return metadata;
}

std::vector<ProfileMetadata> ProfileManager::searchProfiles(const std::string& query) {
    std::vector<ProfileMetadata> results;
    
    std::string lowerQuery = query;
    std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);
    
    for (const auto& profile : m_profiles) {
        std::string lowerName = profile.metadata.name;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        
        std::string lowerDesc = profile.metadata.description;
        std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
        
        std::string lowerCat = profile.metadata.category;
        std::transform(lowerCat.begin(), lowerCat.end(), lowerCat.begin(), ::tolower);
        
        if (lowerName.find(lowerQuery) != std::string::npos ||
            lowerDesc.find(lowerQuery) != std::string::npos ||
            lowerCat.find(lowerQuery) != std::string::npos) {
            results.push_back(profile.metadata);
        }
    }
    
    return results;
}

FingerprintProfile* ProfileManager::getProfile(const std::string& profileId) {
    for (auto& profile : m_profiles) {
        if (profile.metadata.id == profileId) {
            return &profile;
        }
    }
    return nullptr;
}

FingerprintProfile* ProfileManager::getActiveProfile() {
    if (m_activeProfileId.empty()) {
        return nullptr;
    }
    return getProfile(m_activeProfileId);
}

bool ProfileManager::createProfile(const FingerprintProfile& profile) {
    for (const auto& p : m_profiles) {
        if (p.metadata.id == profile.metadata.id) {
            Logger::getInstance().warning("Profile already exists: " + profile.metadata.name);
            return false;
        }
    }
    
    m_profiles.push_back(profile);
    
    std::string filepath = getProfileFilepath(profile.metadata.id);
    if (!saveProfileToFile(profile, filepath)) {
        Logger::getInstance().error("Failed to save profile to file");
        return false;
    }
    
    Logger::getInstance().info("Profile created: " + profile.metadata.name);
    return true;
}

bool ProfileManager::updateProfile(const std::string& profileId, const FingerprintProfile& profile) {
    for (auto& p : m_profiles) {
        if (p.metadata.id == profileId) {
            p = profile;
            
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            std::stringstream ss;
            ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
            p.metadata.modifiedAt = ss.str();
            
            std::string filepath = getProfileFilepath(profileId);
            if (!saveProfileToFile(p, filepath)) {
                Logger::getInstance().error("Failed to save updated profile to file");
                return false;
            }
            
            Logger::getInstance().info("Profile updated: " + profile.metadata.name);
            return true;
        }
    }
    
    return false;
}

bool ProfileManager::deleteProfile(const std::string& profileId) {
    auto it = std::remove_if(m_profiles.begin(), m_profiles.end(),
        [&profileId](const FingerprintProfile& p) {
            return p.metadata.id == profileId;
        });
    
    if (it != m_profiles.end()) {
        m_profiles.erase(it, m_profiles.end());
        
        if (m_activeProfileId == profileId) {
            m_activeProfileId.clear();
        }
        
        std::string filepath = getProfileFilepath(profileId);
        if (fileExists(filepath)) {
            std::remove(filepath.c_str());
        }
        
        Logger::getInstance().info("Profile deleted: " + profileId);
        return true;
    }
    
    return false;
}

bool ProfileManager::setActiveProfile(const std::string& profileId) {
    for (auto& profile : m_profiles) {
        if (profile.metadata.id == profileId) {
            m_activeProfileId = profileId;
            profile.metadata.isActive = true;
            
            Logger::getInstance().info("Active profile set: " + profile.metadata.name);
            return true;
        }
    }
    
    Logger::getInstance().error("Profile not found: " + profileId);
    return false;
}

bool ProfileManager::clearActiveProfile() {
    m_activeProfileId.clear();
    
    for (auto& profile : m_profiles) {
        profile.metadata.isActive = false;
    }
    
    Logger::getInstance().info("Active profile cleared");
    return true;
}

bool ProfileManager::exportProfile(const std::string& profileId, const std::string& filepath) {
    FingerprintProfile* profile = getProfile(profileId);
    if (!profile) {
        Logger::getInstance().error("Profile not found for export: " + profileId);
        return false;
    }
    
    return saveProfileToFile(*profile, filepath);
}

bool ProfileManager::importProfile(const std::string& filepath, std::string& newProfileId) {
    FingerprintProfile profile;
    if (!loadProfileFromFile(filepath, profile)) {
        Logger::getInstance().error("Failed to import profile from: " + filepath);
        return false;
    }
    
    profile.metadata.id = generateProfileId();
    newProfileId = profile.metadata.id;
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
    profile.metadata.createdAt = ss.str();
    profile.metadata.modifiedAt = ss.str();
    
    return createProfile(profile);
}

bool ProfileManager::exportAllProfiles(const std::string& directoryPath) {
    mkdir(directoryPath.c_str(), 0755);
    
    int exported = 0;
    for (const auto& profile : m_profiles) {
        std::string filepath = directoryPath + "/" + profile.metadata.name + ".json";
        if (saveProfileToFile(profile, filepath)) {
            exported++;
        }
    }
    
    Logger::getInstance().info("Exported " + std::to_string(exported) + " profiles");
    return exported > 0;
}

bool ProfileManager::importAllProfiles(const std::string& directoryPath) {
    Logger::getInstance().info("Importing profiles from: " + directoryPath);
    
    return true;
}

bool ProfileManager::applyProfile(const std::string& profileId) {
    FingerprintProfile* profile = getProfile(profileId);
    if (!profile) {
        Logger::getInstance().error("Profile not found: " + profileId);
        return false;
    }
    
    Logger::getInstance().info("Applying profile: " + profile->metadata.name);
    
    DeviceFingerprint deviceFingerprint;
    deviceFingerprint.initialize();
    
    deviceFingerprint.spoofDeviceManufacturer(profile->device.manufacturer);
    deviceFingerprint.spoofDeviceBrand(profile->device.brand);
    deviceFingerprint.spoofDeviceModel(profile->device.model);
    deviceFingerprint.spoofBuildFingerprint(profile->device.buildFingerprint);
    deviceFingerprint.spoofAndroidVersion(profile->device.androidVersion);
    deviceFingerprint.spoofSDKVersion(profile->device.sdkVersion);
    deviceFingerprint.spoofSecurityPatch(profile->device.securityPatch);
    deviceFingerprint.spoofScreenResolution(profile->device.screenWidth, profile->device.screenHeight);
    deviceFingerprint.spoofScreenDensity(profile->device.screenDensity);
    deviceFingerprint.spoofHardwareInfo(profile->device.hardware, profile->device.board);
    deviceFingerprint.spoofOpenGLVersion(profile->device.openglVersion);
    
    NetworkSpoofer networkSpoofer;
    networkSpoofer.initialize();
    
    networkSpoofer.spoofMACAddress(profile->network.macAddress);
    networkSpoofer.spoofCarrierName(profile->network.carrierName);
    networkSpoofer.spoofCarrierCountry(profile->network.carrierCountry);
    
    if (profile->network.locationSpoofingEnabled) {
        networkSpoofer.spoofLocation(profile->network.latitude, profile->network.longitude);
        networkSpoofer.enableMockLocation();
    }
    
    SystemManager systemManager;
    systemManager.initialize();
    
    systemManager.setTimezone(profile->system.timezone);
    systemManager.setLocale(profile->system.locale);
    systemManager.setLanguage(profile->system.language);
    systemManager.setBatteryStatus(profile->system.batteryLevel, 
                                   profile->system.batteryStatus,
                                   profile->system.chargingType);
    
    setActiveProfile(profileId);
    
    Logger::getInstance().info("Profile applied successfully: " + profile->metadata.name);
    
    return true;
}

bool ProfileManager::resetToOriginal() {
    Logger::getInstance().info("Resetting to original device state...");
    
    DeviceFingerprint deviceFingerprint;
    if (deviceFingerprint.initialize()) {
        deviceFingerprint.resetToFactory();
    }
    
    NetworkSpoofer networkSpoofer;
    if (networkSpoofer.initialize()) {
        networkSpoofer.resetAll();
    }
    
    clearActiveProfile();
    
    Logger::getInstance().info("Device reset to original state");
    
    return true;
}

bool ProfileManager::loadProfilesFromDisk() {
    m_profiles.clear();
    
    std::string profileDir = getProfilesDirectory();
    mkdir(profileDir.c_str(), 0755);
    
    std::vector<std::string> profileFiles;
    std::string searchPath = profileDir + "/*.json";
    
#ifdef _WIN32
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = FindFirstFile(searchPath.c_str(), &findFileData);
    if (hFind != INVALID_HANDLE_VALUE) {
        do {
            std::string filename = findFileData.cFileName;
            if (filename.find(".json") != std::string::npos) {
                profileFiles.push_back(profileDir + "/" + filename);
            }
        } while (FindNextFile(hFind, &findFileData));
        FindClose(hFind);
    }
#else
    DIR* dir = opendir(profileDir.c_str());
    if (dir) {
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            std::string filename = entry->d_name;
            if (filename.find(".json") != std::string::npos) {
                profileFiles.push_back(profileDir + "/" + filename);
            }
        }
        closedir(dir);
    }
#endif
    
    int loaded = 0;
    for (const auto& filepath : profileFiles) {
        FingerprintProfile profile;
        if (loadProfileFromFile(filepath, profile)) {
            m_profiles.push_back(profile);
            loaded++;
        }
    }
    
    Logger::getInstance().info("Loaded " + std::to_string(loaded) + " profiles from disk");
    return loaded > 0 || profileFiles.empty();
}

bool ProfileManager::saveProfilesToDisk() {
    for (const auto& profile : m_profiles) {
        std::string filepath = getProfileFilepath(profile.metadata.id);
        saveProfileToFile(profile, filepath);
    }
    
    return true;
}

bool ProfileManager::saveProfileToFile(const FingerprintProfile& profile, const std::string& filepath) {
    std::string json = profileToJson(profile);
    return writeFile(filepath, json);
}

bool ProfileManager::loadProfileFromFile(const std::string& filepath, FingerprintProfile& profile) {
    std::string content = readFile(filepath);
    if (content.empty()) {
        return false;
    }
    
    return jsonToProfile(content, profile);
}

std::string ProfileManager::profileToJson(const FingerprintProfile& profile) {
    std::stringstream ss;
    
    ss << "{\n";
    ss << "  \"metadata\": {\n";
    ss << "    \"id\": \"" << profile.metadata.id << "\",\n";
    ss << "    \"name\": \"" << profile.metadata.name << "\",\n";
    ss << "    \"description\": \"" << profile.metadata.description << "\",\n";
    ss << "    \"category\": \"" << profile.metadata.category << "\",\n";
    ss << "    \"version\": " << profile.metadata.version << ",\n";
    ss << "    \"isActive\": " << (profile.metadata.isActive ? "true" : "false") << "\n";
    ss << "  },\n";
    
    ss << "  \"device\": {\n";
    ss << "    \"manufacturer\": \"" << profile.device.manufacturer << "\",\n";
    ss << "    \"brand\": \"" << profile.device.brand << "\",\n";
    ss << "    \"model\": \"" << profile.device.model << "\",\n";
    ss << "    \"androidVersion\": \"" << profile.device.androidVersion << "\",\n";
    ss << "    \"sdkVersion\": \"" << profile.device.sdkVersion << "\",\n";
    ss << "    \"screenWidth\": " << profile.device.screenWidth << ",\n";
    ss << "    \"screenHeight\": " << profile.device.screenHeight << ",\n";
    ss << "    \"screenDensity\": " << profile.device.screenDensity << "\n";
    ss << "  },\n";
    
    ss << "  \"network\": {\n";
    ss << "    \"macAddress\": \"" << profile.network.macAddress << "\",\n";
    ss << "    \"carrierName\": \"" << profile.network.carrierName << "\",\n";
    ss << "    \"latitude\": " << profile.network.latitude << ",\n";
    ss << "    \"longitude\": " << profile.network.longitude << ",\n";
    ss << "    \"locationSpoofingEnabled\": " << (profile.network.locationSpoofingEnabled ? "true" : "false") << "\n";
    ss << "  },\n";
    
    ss << "  \"system\": {\n";
    ss << "    \"timezone\": \"" << profile.system.timezone << "\",\n";
    ss << "    \"locale\": \"" << profile.system.locale << "\",\n";
    ss << "    \"language\": \"" << profile.system.language << "\"\n";
    ss << "  }\n";
    ss << "}\n";
    
    return ss.str();
}

bool ProfileManager::jsonToProfile(const std::string& json, FingerprintProfile& profile) {
    if (json.empty()) {
        return false;
    }
    
    auto trim = [](const std::string& s) -> std::string {
        size_t start = s.find_first_not_of(" \t\n\r");
        size_t end = s.find_last_not_of(" \t\n\r");
        if (start == std::string::npos) return "";
        return s.substr(start, end - start + 1);
    };
    
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
        
        std::string value = jsonStr.substr(valueStart, valueEnd - valueStart);
        size_t escPos = 0;
        while ((escPos = value.find("\\\"", escPos)) != std::string::npos) {
            value.erase(escPos, 1);
        }
        return value;
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
        while (valueEnd < jsonStr.length() && 
               (isdigit(jsonStr[valueEnd]) || jsonStr[valueEnd] == '-' || jsonStr[valueEnd] == '.')) {
            valueEnd++;
        }
        
        try {
            return std::stoi(jsonStr.substr(valueStart, valueEnd - valueStart));
        } catch (...) {
            return 0;
        }
    };
    
    try {
        profile.metadata.id = extractString(json, "id");
        profile.metadata.name = extractString(json, "name");
        profile.metadata.description = extractString(json, "description");
        profile.metadata.category = extractString(json, "category");
        profile.metadata.author = extractString(json, "author");
        profile.metadata.version = extractInt(json, "version");
        profile.metadata.isActive = extractBool(json, "isActive");
        
        profile.device.manufacturer = extractString(json, "manufacturer");
        profile.device.brand = extractString(json, "brand");
        profile.device.model = extractString(json, "model");
        profile.device.androidVersion = extractString(json, "androidVersion");
        profile.device.sdkVersion = extractString(json, "sdkVersion");
        profile.device.screenWidth = extractInt(json, "screenWidth");
        profile.device.screenHeight = extractInt(json, "screenHeight");
        profile.device.screenDensity = extractInt(json, "screenDensity");
        
        profile.network.macAddress = extractString(json, "macAddress");
        profile.network.carrierName = extractString(json, "carrierName");
        profile.network.latitude = std::stod(extractString(json, "latitude"));
        profile.network.longitude = std::stod(extractString(json, "longitude"));
        profile.network.locationSpoofingEnabled = extractBool(json, "locationSpoofingEnabled");
        
        profile.system.timezone = extractString(json, "timezone");
        profile.system.locale = extractString(json, "locale");
        profile.system.language = extractString(json, "language");
        
        return true;
    } catch (const std::exception& e) {
        Logger::getInstance().error("JSON parsing error: " + std::string(e.what()));
        return false;
    }
}

std::map<std::string, std::string> ProfileManager::profileToMap(const FingerprintProfile& profile) {
    std::map<std::string, std::string> data;
    
    data["metadata.id"] = profile.metadata.id;
    data["metadata.name"] = profile.metadata.name;
    data["device.manufacturer"] = profile.device.manufacturer;
    data["device.model"] = profile.device.model;
    data["device.androidVersion"] = profile.device.androidVersion;
    data["network.macAddress"] = profile.network.macAddress;
    data["network.carrierName"] = profile.network.carrierName;
    data["system.timezone"] = profile.system.timezone;
    
    return data;
}

FingerprintProfile ProfileManager::mapToProfile(const std::map<std::string, std::string>& data) {
    FingerprintProfile profile;
    
    if (data.count("metadata.id")) profile.metadata.id = data.at("metadata.id");
    if (data.count("metadata.name")) profile.metadata.name = data.at("metadata.name");
    if (data.count("device.manufacturer")) profile.device.manufacturer = data.at("device.manufacturer");
    if (data.count("device.model")) profile.device.model = data.at("device.model");
    if (data.count("device.androidVersion")) profile.device.androidVersion = data.at("device.androidVersion");
    if (data.count("network.macAddress")) profile.network.macAddress = data.at("network.macAddress");
    if (data.count("network.carrierName")) profile.network.carrierName = data.at("network.carrierName");
    if (data.count("system.timezone")) profile.system.timezone = data.at("system.timezone");
    
    return profile;
}

}
