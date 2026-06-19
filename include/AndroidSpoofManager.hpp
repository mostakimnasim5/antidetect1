#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

namespace AntiDetect {

// Forward declarations
class ADBManager;
class Logger;

struct DeviceIdentifiers {
    std::string androidId;        // 16 hex characters
    std::string imei;             // 15 digits with Luhn check
    std::string gsfId;            // Google Services Framework ID (16 digits)
    std::string gaid;             // Google Advertising ID (UUID format)
    std::string androidBootId;     // UUID format
    std::string imsi;             // IMSI (15 digits: MCC+MNC+MSIN)
    std::string serialNumber;      // Device serial
};

struct DeviceInfoSpoof {
    std::string manufacturer;
    std::string brand;
    std::string model;
    std::string device;
    std::string product;
    std::string hardware;
    std::string board;
    std::string tacCode;          // Type Allocation Code (first 8 digits of IMEI)
};

struct BuildSpoof {
    std::string androidVersion;
    std::string sdkVersion;
    std::string securityPatch;
    std::string buildId;
    std::string buildFingerprint;
    std::string buildTags;
    std::string buildType;
};

struct NetworkSpoof {
    std::string wifiMac;
    std::string bluetoothMac;
    std::string carrierName;
    std::string carrierCountry;
    std::string simOperator;       // MCC+MNC
    std::string networkType;       // 5G, LTE, 3G, etc.
};

struct LocationSpoof {
    double latitude;
    double longitude;
    double altitude;
    double accuracy;
    std::string country;
    std::string city;
    std::string timezone;
};

struct HardwareSpoof {
    std::string cpuModel;
    std::string cpuAbi;
    int processorCount;
    long totalMemoryMB;
    std::string gpuRenderer;
    std::string gpuVendor;
    std::string openGlVersion;
    std::string vulkanVersion;
};

struct SensorSpoof {
    bool enabled;
    float accelerometer[3];  // x, y, z
    float gyroscope[3];
    float magnetometer[3];
    float light;
    bool proximityNear;
};

struct BrowserSpoof {
    std::string userAgent;
    std::string webrtcLocalIP;
    std::string webrtcPublicIP;
    std::string acceptLanguage;
    std::string acceptEncoding;
};

struct DRMSpoof {
    int widevineLevel;         // 1, 2, or 3
    std::string hdcpLevel;     // 1.0 to 2.3
    bool basicIntegrity;
    bool deviceIntegrity;
    bool ctsMismatch;
    bool playservicesValid;
    bool basicDeviceIntegrity;
};

struct CompleteProfile {
    std::string profileId;
    std::string profileName;
    std::string brand;             // e.g., Samsung, Google, Xiaomi
    std::string description;
    
    DeviceIdentifiers identifiers;
    DeviceInfoSpoof device;
    BuildSpoof build;
    NetworkSpoof network;
    LocationSpoof location;
    HardwareSpoof hardware;
    SensorSpoof sensors;
    BrowserSpoof browser;
    DRMSpoof drm;
    
    std::string createdAt;
    std::string modifiedAt;
};

struct OperationResult {
    bool success;
    std::string message;
    std::string error;
};

class AndroidSpoofManager {
public:
    static AndroidSpoofManager& getInstance();
    
    // Initialization
    bool initialize();
    bool shutdown();
    
    // Single ID Generation
    std::string generateAndroidId();
    std::string generateIMEI(const std::string& tacCode = "");
    std::string generateGSFId();
    std::string generateGAID();
    std::string generateAndroidBootId();
    std::string generateIMSI(const std::string& mccMnc = "");
    std::string generateSerialNumber();
    
    // Luhn Algorithm
    bool validateIMEI(const std::string& imei);
    bool validateIMSI(const std::string& imsi);
    int calculateLuhnCheckDigit(const std::string& baseNumber);
    
    // TAC Database
    std::vector<std::string> getTACCodesByBrand(const std::string& brand);
    std::string getRandomTACForBrand(const std::string& brand);
    std::vector<std::string> getAllBrands();
    std::string getTACInfo(const std::string& tacCode);
    
    // Profile Generation
    CompleteProfile generateProfile(const std::string& brand, const std::string& model = "");
    std::vector<CompleteProfile> generateBatchProfiles(int count, const std::string& brand = "");
    CompleteProfile generateRandomProfile();
    
    // Country & Network Data
    std::vector<std::map<std::string, std::string>> getCountryData();
    std::map<std::string, std::string> getRandomLocationData(const std::string& country = "");
    std::string getRandomMCCMNC(const std::string& country = "");
    
    // Spoofing Operations
    OperationResult applyIdentifiers(const DeviceIdentifiers& ids);
    OperationResult applyDeviceInfo(const DeviceInfoSpoof& device);
    OperationResult applyBuildInfo(const BuildSpoof& build);
    OperationResult applyNetworkInfo(const NetworkSpoof& network);
    OperationResult applyLocationInfo(const LocationSpoof& location);
    OperationResult applyHardwareInfo(const HardwareSpoof& hardware);
    OperationResult applySensorSpoofing(const SensorSpoof& sensors);
    OperationResult applyBrowserSpoofing(const BrowserSpoof& browser);
    OperationResult applyDRMSpoofing(const DRMSpoof& drm);
    
    OperationResult applyCompleteProfile(const CompleteProfile& profile);
    OperationResult applyProfileById(const std::string& profileId);
    
    // Tracker Blocking (Hosts File)
    OperationResult blockTracker(const std::string& domain);
    OperationResult unblockTracker(const std::string& domain);
    OperationResult loadTrackerBlocklist(const std::string& filepath);
    std::vector<std::string> getBlockedTrackers();
    OperationResult applyHostsBlocking();
    OperationResult backupHostsFile();
    OperationResult restoreHostsFile();
    
    // Profile Management
    bool saveProfile(const CompleteProfile& profile);
    bool deleteProfile(const std::string& profileId);
    CompleteProfile* getProfile(const std::string& profileId);
    std::vector<CompleteProfile> getAllProfiles();
    std::vector<CompleteProfile> searchProfiles(const std::string& query);
    
    // Import/Export (JSON)
    OperationResult exportToJSON(const std::string& filepath);
    OperationResult exportToJSON(const std::string& filepath, const std::vector<std::string>& profileIds);
    OperationResult importFromJSON(const std::string& filepath);
    OperationResult exportBatchToJSON(const std::string& directory, int profilesPerFile = 1000);
    
    // Reset
    OperationResult resetAll();
    OperationResult resetIdentifiers();
    OperationResult resetNetwork();
    OperationResult resetLocation();
    OperationResult resetSensors();
    OperationResult restoreOriginalState();
    
    // Status
    std::map<std::string, std::string> getCurrentSpoofState();
    std::map<std::string, std::string> getSystemStatus();
    
private:
    AndroidSpoofManager();
    ~AndroidSpoofManager();
    
    AndroidSpoofManager(const AndroidSpoofManager&) = delete;
    AndroidSpoofManager& operator=(const AndroidSpoofManager&) = delete;
    
    void initializeTACDatabase();
    void initializeCountryData();
    void initializeBrandProfiles();
    
    std::string generateRandomHex(int length);
    std::string generateRandomDigits(int length);
    std::string formatMAC();
    std::string formatUUID();
    
    bool applyToDevice(const std::string& property, const std::string& value);
    std::string getDeviceProperty(const std::string& property);
    
    void backupCurrentState();
    
    std::map<std::string, std::string> m_originalState;
    std::map<std::string, std::string> m_currentState;
    std::map<std::string, CompleteProfile> m_profiles;
    std::vector<std::string> m_blockedTrackers;
    
    std::map<std::string, std::vector<std::string>> m_tacDatabase;
    std::map<std::string, std::map<std::string, std::string>> m_countryData;
    std::map<std::string, CompleteProfile> m_brandProfiles;
    
    std::map<std::string, std::map<std::string, std::string>> m_mccMncDatabase;
    
    bool m_initialized;
    bool m_hostsBackupCreated;
    std::string m_hostsBackupPath;
    
    // Brand-specific profile templates
    void createSamsungProfiles();
    void createGoogleProfiles();
    void createXiaomiProfiles();
    void createOnePlusProfiles();
    void createAppleProfiles();
    void createOPPOProfiles();
    void createVivoProfiles();
    void createRealmeProfiles();
    void createHuaweiProfiles();
    void createSonyProfiles();
    void createLGProfiles();
    void createMotorolaProfiles();
    void createAsusProfiles();
    void createNokiaProfiles();
};

}
