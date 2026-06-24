#pragma once

/**
 * ProfileManager - Device Profile Generation & Management
 * 
 * Generates realistic, unique device profiles for each VM instance.
 */

#include "../VirtualPhonePro.hpp"
#include "../profiles/DeviceProfileDatabase.hpp"

namespace VirtualPhonePro {

class ProfileManager {
public:
    static ProfileManager& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // Profile Generation
    FingerprintConfig generateRandomProfile(const std::string& region = "");
    FingerprintConfig generateProfile(const std::string& manufacturer, 
                                     const std::string& model,
                                     const std::string& region = "");
    FingerprintConfig generateFromTemplate(const std::string& templateId);
    
    // Profile Modification
    FingerprintConfig randomizeFingerprint(const FingerprintConfig& base);
    FingerprintConfig applyRegionVariations(const FingerprintConfig& base, 
                                           const std::string& region);
    
    // Database Access
    std::vector<DeviceProfile> getAllDeviceProfiles();
    std::vector<DeviceProfile> getDeviceProfilesByManufacturer(const std::string& manufacturer);
    std::vector<CarrierProfile> getCarrierProfiles(const std::string& country);
    std::vector<RegionProfile> getAllRegionProfiles();
    std::optional<DeviceProfile> getDeviceProfile(const std::string& id);
    
    // Profile Storage
    bool saveProfile(const FingerprintConfig& profile, const std::string& id);
    std::optional<FingerprintConfig> loadProfile(const std::string& id);
    std::vector<FingerprintConfig> listSavedProfiles();
    bool deleteProfile(const std::string& id);
    
    // Validation
    bool validateFingerprint(const FingerprintConfig& profile);
    std::string calculateFingerprintHash(const FingerprintConfig& profile);
    
private:
    ProfileManager();
    ~ProfileManager();
    ProfileManager(const ProfileManager&) = delete;
    ProfileManager& operator=(const ProfileManager&) = delete;
    
    // Internal generation helpers
    std::string generateUniqueBuildId(const std::string& manufacturer);
    std::string generateUniqueFingerprint(const FingerprintConfig& config);
    std::string generateBootloader(const std::string& model);
    std::string generateSerialNumber();
    std::string generateMACAddress(const std::string& prefix = "");
    
    // Sensor data generation
    std::map<std::string, std::string> generateSensorData();
    
    // Database
    DeviceProfileDatabase m_database;
    std::mutex m_profileMutex;
};

} // namespace VirtualPhonePro
