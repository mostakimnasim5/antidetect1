#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace AntiDetect {

enum class FingerprintCategory {
    DEVICE_INFO,
    HARDWARE_INFO,
    BUILD_PROPERTIES,
    DISPLAY_INFO,
    NETWORK_INFO,
    SENSOR_INFO,
    SYSTEM_INFO
};

struct FingerprintResult {
    bool success;
    std::string property;
    std::string originalValue;
    std::string newValue;
    std::string message;
    std::string error;
};

class DeviceFingerprint {
public:
    DeviceFingerprint();
    ~DeviceFingerprint();
    
    bool initialize();
    bool isInitialized() const;
    
    std::map<std::string, std::string> getCurrentFingerprint();
    std::vector<std::string> getFingerprintCategories();
    
    FingerprintResult spoofDeviceManufacturer(const std::string& manufacturer);
    FingerprintResult spoofDeviceModel(const std::string& model);
    FingerprintResult spoofDeviceBrand(const std::string& brand);
    
    FingerprintResult spoofBuildFingerprint(const std::string& fingerprint);
    FingerprintResult spoofBuildId(const std::string& buildId);
    FingerprintResult spoofAndroidVersion(const std::string& version);
    FingerprintResult spoofSDKVersion(const std::string& sdkVersion);
    FingerprintResult spoofSecurityPatch(const std::string& patch);
    
    FingerprintResult spoofScreenResolution(int width, int height);
    FingerprintResult spoofScreenDensity(int density);
    FingerprintResult spoofScreenDPI(const std::string& dpi);
    
    FingerprintResult spoofHardwareInfo(const std::string& hardware, const std::string& board);
    FingerprintResult spoofGPUInfo(const std::string& vendor, const std::string& renderer);
    FingerprintResult spoofOpenGLVersion(const std::string& version);
    
    FingerprintResult spoofBootloader(const std::string& bootloader);
    FingerprintResult spoofRadioVersion(const std::string& version);
    
    FingerprintResult spoofDeviceName(const std::string& name);
    
    bool resetToFactory();
    bool resetProperty(const std::string& property);
    
    std::string getOriginalValue(const std::string& property);
    bool applyFullProfile(const std::map<std::string, std::string>& profile);

private:
    bool applyPropertyChange(const std::string& property, const std::string& value);
    std::string getPropertyValue(const std::string& property);
    
    bool backupOriginalValue(const std::string& property, const std::string& value);
    bool restoreOriginalValue(const std::string& property);
    
    std::map<std::string, std::string> m_originalValues;
    std::map<std::string, std::string> m_currentValues;
    std::vector<std::string> m_appliedChanges;
    bool m_initialized;
    
    std::vector<std::string> getAllSystemProperties();
    std::vector<std::string> getRelevantDeviceProperties();
    
    static const std::vector<std::string> DEVICE_PROPERTIES;
    static const std::vector<std::string> BUILD_PROPERTIES;
    static const std::vector<std::string> DISPLAY_PROPERTIES;
    static const std::vector<std::string> HARDWARE_PROPERTIES;
};

}
