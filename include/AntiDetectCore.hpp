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
class SensorSpoofer;
class PlayIntegrityBypass;
class HypervisorBypass;
class TimingAttackPrevention;
class HardwareFingerprintSpoofer;
class NetworkStackSpoofer;
class SafetyNetAdvancedBypass;

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
    
    // Device Management
    AntiDetectResult connect(const std::string& deviceAddress);
    AntiDetectResult disconnect();
    std::vector<std::string> listDevices();
    AntiDetectResult selectDevice(const std::string& serial);
    
    // Device Fingerprint
    AntiDetectResult applyDeviceProfile(const std::map<std::string, std::string>& profile);
    AntiDetectResult resetDevice();
    
    AntiDetectResult spoofManufacturer(const std::string& manufacturer);
    AntiDetectResult spoofModel(const std::string& model);
    AntiDetectResult spoofBrand(const std::string& brand);
    AntiDetectResult spoofAndroidVersion(const std::string& version);
    AntiDetectResult spoofBuildFingerprint(const std::string& fingerprint);
    
    AntiDetectResult spoofScreenResolution(int width, int height);
    AntiDetectResult spoofScreenDensity(int density);
    
    // Network
    AntiDetectResult spoofMACAddress(const std::string& macAddress);
    AntiDetectResult spoofCarrier(const std::string& carrierName);
    AntiDetectResult spoofLocation(double latitude, double longitude);
    
    AntiDetectResult enableGPSSpoofing();
    AntiDetectResult enableMockLocation();
    
    // System
    AntiDetectResult setTimezone(const std::string& timezone);
    AntiDetectResult setLocale(const std::string& locale);
    AntiDetectResult setLanguage(const std::string& language);
    
    AntiDetectResult setBatteryStatus(int level, const std::string& status);
    
    AntiDetectResult enableSELinux();
    AntiDetectResult disableSELinux();
    AntiDetectResult enableDebugMode();
    AntiDetectResult disableDebugMode();
    
    // Profile Management
    AntiDetectResult applyProfile(const std::string& profileId);
    AntiDetectResult createProfile(const std::string& name, const std::map<std::string, std::string>& data);
    AntiDetectResult deleteProfile(const std::string& profileId);
    std::vector<std::map<std::string, std::string>> listProfiles();
    
    AntiDetectResult exportConfiguration(const std::string& filepath);
    AntiDetectResult importConfiguration(const std::string& filepath);
    
    // Advanced Anti-Detection (v1.5)
    
    // Sensor Spoofing - Natural sensor noise simulation
    AntiDetectResult enableSensorSpoofing();
    AntiDetectResult enableAccelerometerSpoofing(double x, double y, double z);
    AntiDetectResult enableGyroscopeSpoofing(double x, double y, double z);
    AntiDetectResult enableMagnetometerSpoofing(double x, double y, double z);
    AntiDetectResult enableNaturalMovement(const std::string& pattern);
    
    // Play Integrity API Bypass
    AntiDetectResult enableIntegrityBypass();
    AntiDetectResult setIntegrityLevel(const std::string& level);
    AntiDetectResult bypassSafetyNet();
    AntiDetectResult emulateTrustZone();
    
    // Hypervisor/VM Detection Bypass
    AntiDetectResult enableHypervisorBypass();
    AntiDetectResult setDeviceAsRealHardware();
    AntiDetectResult enableARMSimulation();
    AntiDetectResult enableTimingNormalization();
    
    // Timing Attack Prevention
    AntiDetectResult enableTimingProtection();
    AntiDetectResult setTimingProfile(const std::string& profile);
    AntiDetectResult addExecutionNoise();
    
    // Hardware Fingerprint Spoofing (v1.6)
    AntiDetectResult enableHardwareSpoofing();
    AntiDetectResult setSamsungProfile();
    AntiDetectResult setGoogleProfile();
    AntiDetectResult setXiaomiProfile();
    AntiDetectResult spoofCPU(const std::string& cpu, int cores);
    AntiDetectResult spoofGPU(const std::string& gpu);
    
    // Network Stack Spoofing (v1.6)
    AntiDetectResult enableNetworkSpoofing();
    AntiDetectResult setDeviceTTL();
    AntiDetectResult spoofMAC(const std::string& mac);
    AntiDetectResult setGoogleDNS();
    AntiDetectResult setChromeUA();
    
    // SafetyNet Advanced Bypass (v1.6)
    AntiDetectResult performFullSafetyNetBypass();
    AntiDetectResult bypassRootDetection();
    AntiDetectResult setGreenBoot();
    AntiDetectResult enforceSELinux();
    AntiDetectResult setReleaseKeys();
    AntiDetectResult setCertifiedIntegrity();
    
    // Status
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
    
    // Advanced Anti-Detection Modules (v1.5)
    std::unique_ptr<SensorSpoofer> m_sensorSpoofer;
    std::unique_ptr<PlayIntegrityBypass> m_playIntegrity;
    std::unique_ptr<HypervisorBypass> m_hypervisorBypass;
    std::unique_ptr<TimingAttackPrevention> m_timingPrevention;
    
    // Ultra Advanced Modules (v1.6)
    std::unique_ptr<HardwareFingerprintSpoofer> m_hardwareSpoofer;
    std::unique_ptr<NetworkStackSpoofer> m_networkStackSpoofer;
    std::unique_ptr<SafetyNetAdvancedBypass> m_safetyNetBypass;
    
    static constexpr const char* VERSION = "1.6.0";
    static constexpr const char* BUILD_DATE = __DATE__;
};

}
