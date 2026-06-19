#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

namespace AntiDetect {

struct DeviceInfo {
    std::string serial;
    std::string model;
    std::string state;
    std::string product;
    std::string device;
    std::string transportId;
};

struct ProfileInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    std::string createdAt;
    std::string modifiedAt;
};

struct OperationResult {
    bool success;
    std::string message;
    std::string error;
    std::map<std::string, std::string> data;
};

class AntiDetectEngine {
public:
    AntiDetectEngine();
    ~AntiDetectEngine();
    
    bool initialize();
    bool shutdown();
    
    // Device Management
    std::vector<DeviceInfo> listDevices();
    OperationResult connect(const std::string& address);
    OperationResult disconnect();
    OperationResult selectDevice(const std::string& serial);
    
    // Device Spoofing
    OperationResult spoofManufacturer(const std::string& manufacturer);
    OperationResult spoofModel(const std::string& model);
    OperationResult spoofBrand(const std::string& brand);
    OperationResult spoofAndroidVersion(const std::string& version);
    OperationResult spoofBuildFingerprint(const std::string& fingerprint);
    OperationResult spoofDeviceId(const std::string& deviceId);
    OperationResult spoofAndroidId(const std::string& androidId);
    OperationResult spoofSerialNumber(const std::string& serial);
    
    // Screen Spoofing
    OperationResult spoofScreenResolution(int width, int height);
    OperationResult spoofScreenDensity(int density);
    
    // Network Spoofing
    OperationResult spoofMACAddress(const std::string& mac);
    OperationResult spoofCarrier(const std::string& carrier);
    OperationResult spoofCountry(const std::string& country);
    OperationResult spoofLocation(double latitude, double longitude);
    OperationResult enableMockLocation();
    OperationResult spoofUserAgent(const std::string& userAgent);
    OperationResult spoofWebRTCIP(const std::string& ip);
    
    // Hardware Spoofing
    OperationResult spoofCPUModel(const std::string& cpu);
    OperationResult spoofGPU(const std::string& gpu);
    OperationResult spoofTotalMemory(int memoryMB);
    
    // Sensor Spoofing
    OperationResult spoofAccelerometer(float x, float y, float z);
    OperationResult spoofGyroscope(float x, float y, float z);
    OperationResult spoofMagnetometer(float x, float y, float z);
    OperationResult spoofLightSensor(float lux);
    
    // System Control
    OperationResult setTimezone(const std::string& timezone);
    OperationResult setLocale(const std::string& locale);
    OperationResult setLanguage(const std::string& language);
    OperationResult setBatteryStatus(int level, const std::string& status);
    OperationResult enableSELinux();
    OperationResult disableSELinux();
    OperationResult enableDebugMode();
    OperationResult disableDebugMode();
    
    // DRM & Integrity
    OperationResult spoofWidevineLevel(int level);
    OperationResult spoofHDCPLevel(const std::string& level);
    OperationResult enableBasicIntegrity();
    OperationResult enableDeviceIntegrity();
    OperationResult enableNoCtsMismatch();
    OperationResult spoofSafetyNet(const std::map<std::string, std::string>& response);
    
    // Tracker Blocking
    OperationResult blockTracker(const std::string& domain);
    OperationResult unblockTracker(const std::string& domain);
    OperationResult loadBlocklist(const std::string& filepath);
    
    // Profile Management
    std::vector<ProfileInfo> listProfiles();
    OperationResult applyProfile(const std::string& profileId);
    OperationResult createProfile(const std::string& name, const std::map<std::string, std::string>& data);
    OperationResult deleteProfile(const std::string& profileId);
    OperationResult exportProfiles(const std::string& directory);
    OperationResult importProfiles(const std::string& directory);
    
    // State Management
    OperationResult resetAll();
    OperationResult backupState();
    OperationResult restoreState();
    
    // Information
    std::map<std::string, std::string> getCurrentFingerprint();
    std::map<std::string, std::string> getSystemStatus();
    std::map<std::string, std::string> getDeviceInfo();

private:
    OperationResult createResult(bool success, const std::string& message, const std::string& error = "");
    
    class Impl;
    std::unique_ptr<Impl> m_impl;
};

}
