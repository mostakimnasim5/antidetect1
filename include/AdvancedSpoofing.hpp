#pragma once

#include <string>
#include <vector>
#include <map>
#include <random>
#include <chrono>

namespace AntiDetect {

struct SpoofingResult {
    bool success;
    std::string category;
    std::string property;
    std::string originalValue;
    std::string newValue;
    std::string error;
};

struct DeviceIDConfig {
    std::string androidId;
    std::string deviceId;
    std::string serialNumber;
    std::string buildId;
};

struct HardwareConfig {
    std::string cpuModel;
    std::string cpuAbi;
    std::string cpuHardware;
    int processorCount;
    long totalMemory;
};

struct SensorConfig {
    bool accelerometer;
    bool gyroscope;
    bool magnetometer;
    bool proximity;
    bool light;
    
    float accelerometerX;
    float accelerometerY;
    float accelerometerZ;
    
    float gyroscopeX;
    float gyroscopeY;
    float gyroscopeZ;
    
    float magnetometerX;
    float magnetometerY;
    float magnetometerZ;
};

struct WebRTCConfig {
    std::string localIp;
    std::string publicIp;
    std::vector<std::string> iceServers;
    bool enableProxy;
};

class AdvancedSpoofing {
public:
    AdvancedSpoofing();
    ~AdvancedSpoofing();
    
    bool initialize();
    bool isInitialized() const;
    
    // Device ID Spoofing
    SpoofingResult spoofAndroidId(const std::string& androidId);
    SpoofingResult spoofDeviceId(const std::string& deviceId);
    SpoofingResult spoofSerialNumber(const std::string& serial);
    SpoofingResult spoofBuildId(const std::string& buildId);
    
    // Hardware Spoofing
    SpoofingResult spoofCPUModel(const std::string& cpuModel);
    SpoofingResult spoofCPUAbi(const std::string& abi);
    SpoofingResult spoofProcessorCount(int count);
    SpoofingResult spoofTotalMemory(long memoryMB);
    
    // GPU Spoofing
    SpoofingResult spoofGPURenderer(const std::string& renderer);
    SpoofingResult spoofGPUVendor(const std::string& vendor);
    SpoofingResult spoofOpenGLVersion(const std::string& version);
    SpoofingResult spoofVulkanVersion(const std::string& version);
    
    // Sensor Spoofing
    SpoofingResult spoofAccelerometer(float x, float y, float z);
    SpoofingResult spoofGyroscope(float x, float y, float z);
    SpoofingResult spoofMagnetometer(float x, float y, float z);
    SpoofingResult spoofProximity(bool present);
    SpoofingResult spoofLightSensor(float lux);
    
    SpoofingResult enableSensorSpoofing();
    SpoofingResult disableSensorSpoofing();
    
    // User-Agent Spoofing
    SpoofingResult spoofUserAgent(const std::string& userAgent);
    std::string generateRandomUserAgent(const std::string& browser = "Chrome", const std::string& os = "Android");
    
    // WebRTC Spoofing
    SpoofingResult spoofWebRTCLocalIP(const std::string& ip);
    SpoofingResult spoofWebRTCPublicIP(const std::string& ip);
    SpoofingResult enableWebRTCProxy();
    SpoofingResult disableWebRTCProxy();
    
    // Widevine & DRM
    SpoofingResult spoofWidevineLevel(int level);
    SpoofingResult spoofHDCPLevel(const std::string& level);
    SpoofingResult enableDRMEmulation();
    SpoofingResult disableDRMEmulation();
    
    // SafetyNet & Play Integrity
    SpoofingResult spoofSafetyNetResponse(const std::map<std::string, std::string>& response);
    SpoofingResult spoofPlayIntegrityResult(const std::string& nonce, const std::string& result);
    SpoofingResult enableBasicIntegrity();
    SpoofingResult enableDeviceIntegrity();
    SpoofingResult enableNoCtsMismatch();
    
    // Tracker Blocking
    SpoofingResult blockTracker(const std::string& trackerDomain);
    SpoofingResult unblockTracker(const std::string& trackerDomain);
    std::vector<std::string> getBlockedTrackers();
    SpoofingResult loadBlocklist(const std::string& filepath);
    
    // Utility
    std::string generateRandomAndroidId();
    std::string generateRandomSerial();
    std::string generateRandomDeviceId();
    std::string generateRandomBuildFingerprint();
    
    bool resetAll();
    bool resetCategory(const std::string& category);
    std::map<std::string, std::string> getCurrentSpoofState();

private:
    bool applySpoof(const std::string& property, const std::string& value);
    std::string getCurrentValue(const std::string& property);
    void backupOriginalValue(const std::string& property, const std::string& value);
    
    std::string generateRandomHex(int length);
    std::string generateRandomAlphanumeric(int length);
    std::string formatSerialNumber();
    std::string formatAndroidId();
    
    bool m_initialized;
    bool m_sensorSpoofingEnabled;
    bool m_webRTCPProxyEnabled;
    bool m_drmEmulationEnabled;
    
    std::map<std::string, std::string> m_originalValues;
    std::map<std::string, std::string> m_currentValues;
    std::map<std::string, std::string> m_blockedTrackers;
    
    std::vector<std::string> m_appliedSpoofs;
    std::vector<std::string> m_trustedTrackers;
    
    static const std::vector<std::string> DEVICE_ID_PROPERTIES;
    static const std::vector<std::string> HARDWARE_PROPERTIES;
    static const std::vector<std::string> GPU_PROPERTIES;
    static const std::vector<std::string> WEBRTC_PROPERTIES;
    static const std::vector<std::string> DRM_PROPERTIES;
    static const std::vector<std::string> TRACKER_DOMAINS;
};

}
