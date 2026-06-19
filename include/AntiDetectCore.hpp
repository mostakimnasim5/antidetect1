#pragma once

#include <string>
#include <memory>
#include <vector>
#include <map>

namespace AntiDetect {

class ADBManager;
class DeviceFingerprint;
class NetworkSpoofer;
class SystemManager;
class ProfileManager;
class Logger;
class Config;

enum class AntiDetectStatus {
    UNINITIALIZED,
    INITIALIZING,
    READY,
    RUNNING,
    ERROR
};

struct AntiDetectResult {
    bool success;
    std::string message;
    std::string error;
    std::map<std::string, std::string> data;
};

class AntiDetectCore {
public:
    static AntiDetectCore& getInstance();
    
    bool initialize();
    bool initialize(const std::string& configPath);
    bool shutdown();
    
    AntiDetectStatus getStatus() const;
    std::string getVersion() const;
    
    AntiDetectResult connect(const std::string& deviceAddress);
    AntiDetectResult disconnect();
    std::vector<std::string> listDevices();
    AntiDetectResult selectDevice(const std::string& serial);
    
    AntiDetectResult applyDeviceProfile(const std::map<std::string, std::string>& profile);
    AntiDetectResult resetDevice();
    
    AntiDetectResult spoofManufacturer(const std::string& manufacturer);
    AntiDetectResult spoofModel(const std::string& model);
    AntiDetectResult spoofBrand(const std::string& brand);
    AntiDetectResult spoofAndroidVersion(const std::string& version);
    AntiDetectResult spoofBuildFingerprint(const std::string& fingerprint);
    
    AntiDetectResult spoofScreenResolution(int width, int height);
    AntiDetectResult spoofScreenDensity(int density);
    
    AntiDetectResult spoofMACAddress(const std::string& macAddress);
    AntiDetectResult spoofCarrier(const std::string& carrierName);
    AntiDetectResult spoofLocation(double latitude, double longitude);
    
    AntiDetectResult enableGPSSpoofing();
    AntiDetectResult enableMockLocation();
    
    AntiDetectResult setTimezone(const std::string& timezone);
    AntiDetectResult setLocale(const std::string& locale);
    AntiDetectResult setLanguage(const std::string& language);
    
    AntiDetectResult setBatteryStatus(int level, const std::string& status);
    
    AntiDetectResult enableSELinux();
    AntiDetectResult disableSELinux();
    AntiDetectResult enableDebugMode();
    AntiDetectResult disableDebugMode();
    
    AntiDetectResult applyProfile(const std::string& profileId);
    AntiDetectResult createProfile(const std::string& name, const std::map<std::string, std::string>& data);
    AntiDetectResult deleteProfile(const std::string& profileId);
    std::vector<std::map<std::string, std::string>> listProfiles();
    
    AntiDetectResult exportConfiguration(const std::string& filepath);
    AntiDetectResult importConfiguration(const std::string& filepath);
    
    std::map<std::string, std::string> getDeviceInfo();
    std::map<std::string, std::string> getCurrentFingerprint();
    std::map<std::string, std::string> getSystemStatus();
    
    AntiDetectCore(const AntiDetectCore&) = delete;
    AntiDetectCore& operator=(const AntiDetectCore&) = delete;

private:
    AntiDetectCore();
    ~AntiDetectCore();
    
    void initializeComponents();
    void cleanupComponents();
    
    AntiDetectResult createResult(bool success, const std::string& message, const std::string& error = "");
    
    AntiDetectStatus m_status;
    std::unique_ptr<ADBManager> m_adbManager;
    std::unique_ptr<DeviceFingerprint> m_deviceFingerprint;
    std::unique_ptr<NetworkSpoofer> m_networkSpoofer;
    std::unique_ptr<SystemManager> m_systemManager;
    std::unique_ptr<ProfileManager> m_profileManager;
    
    static constexpr const char* VERSION = "1.0.0";
    static constexpr const char* BUILD_DATE = __DATE__;
};

}
