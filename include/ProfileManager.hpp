#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace AntiDetect {

struct ProfileMetadata {
    std::string id;
    std::string name;
    std::string description;
    std::string createdAt;
    std::string modifiedAt;
    std::string category;
    std::string author;
    int version;
    bool isActive;
};

struct DeviceFingerprintData {
    std::string manufacturer;
    std::string brand;
    std::string model;
    std::string device;
    std::string product;
    std::string hardware;
    std::string board;
    
    std::string androidVersion;
    std::string sdkVersion;
    std::string securityPatch;
    std::string buildId;
    std::string buildFingerprint;
    
    int screenWidth;
    int screenHeight;
    int screenDensity;
    std::string screenDPI;
    
    std::string bootloader;
    std::string radioVersion;
    std::string openglVersion;
    
    std::string gpuVendor;
    std::string gpuRenderer;
};

struct NetworkFingerprintData {
    std::string macAddress;
    std::string wifiMacAddress;
    std::string bluetoothMac;
    
    std::string carrierName;
    std::string carrierCountry;
    std::string simOperator;
    std::string networkType;
    
    double latitude;
    double longitude;
    bool locationSpoofingEnabled;
};

struct SystemFingerprintData {
    bool selinuxEnforcing;
    bool debugEnabled;
    bool mockLocationEnabled;
    
    std::string timezone;
    std::string locale;
    std::string language;
    
    int batteryLevel;
    std::string batteryStatus;
    std::string chargingType;
};

struct FingerprintProfile {
    ProfileMetadata metadata;
    DeviceFingerprintData device;
    NetworkFingerprintData network;
    SystemFingerprintData system;
};

enum class ProfileStatus {
    SUCCESS,
    NOT_FOUND,
    ALREADY_EXISTS,
    INVALID_DATA,
    FILE_ERROR
};

class ProfileManager {
public:
    ProfileManager();
    ~ProfileManager();
    
    bool initialize();
    bool isInitialized() const;
    
    std::vector<ProfileMetadata> listProfiles();
    std::vector<ProfileMetadata> searchProfiles(const std::string& query);
    
    FingerprintProfile* getProfile(const std::string& profileId);
    FingerprintProfile* getActiveProfile();
    
    bool createProfile(const FingerprintProfile& profile);
    bool updateProfile(const std::string& profileId, const FingerprintProfile& profile);
    bool deleteProfile(const std::string& profileId);
    
    bool setActiveProfile(const std::string& profileId);
    bool clearActiveProfile();
    
    bool exportProfile(const std::string& profileId, const std::string& filepath);
    bool importProfile(const std::string& filepath, std::string& newProfileId);
    
    bool exportAllProfiles(const std::string& directoryPath);
    bool importAllProfiles(const std::string& directoryPath);
    
    bool applyProfile(const std::string& profileId);
    bool resetToOriginal();
    
    std::string generateProfileId();
    FingerprintProfile createDefaultProfile(const std::string& name, const std::string& category);
    
    std::map<std::string, std::string> profileToMap(const FingerprintProfile& profile);
    FingerprintProfile mapToProfile(const std::map<std::string, std::string>& data);

private:
    bool loadProfilesFromDisk();
    bool saveProfilesToDisk();
    
    std::string getProfilesDirectory();
    std::string getProfileFilepath(const std::string& profileId);
    
    bool fileExists(const std::string& filepath);
    std::string readFile(const std::string& filepath);
    bool writeFile(const std::string& filepath, const std::string& content);
    
    std::vector<FingerprintProfile> m_profiles;
    std::string m_activeProfileId;
    std::string m_profilesDir;
    bool m_initialized;
    
    static const std::string DEFAULT_PROFILES_DIR;
};

}
