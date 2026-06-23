#pragma once

#include <string>
#include <map>
#include <mutex>
#include "CommonTypes.hpp"

namespace AntiDetect {

struct DeviceConfig {
    std::string manufacturer;
    std::string brand;
    std::string model;
    std::string device;
    std::string product;
    std::string hardware;
    std::string board;
    std::string deviceName;
    
    std::string androidVersion;
    std::string sdkVersion;
    std::string securityPatch;
    std::string buildId;
    std::string buildFingerprint;
    
    int screenWidth;
    int screenHeight;
    int screenDensity;
    std::string screenDensityDpi;
    
    std::string bootloader;
    std::string radioVersion;
    
    std::string gpuVendor;
    std::string gpuRenderer;
    std::string openglVersion;
};

struct NetworkConfig {
    std::string macAddress;
    std::string wifiMacAddress;
    std::string bluetoothMac;
    std::string ethernetMac;
    
    std::string carrierName;
    std::string carrierCountry;
    std::string simOperator;
    std::string networkType;
    
    double latitude;
    double longitude;
    bool locationSpoofing;
};

struct SystemConfig {
    bool disableSelinux;
    bool enableDebug;
    bool mockLocation;
    bool allowAdbAuth;
    
    std::string timezone;
    std::string locale;
    std::string language;
    
    int batteryLevel;
    std::string batteryStatus;
    std::string chargingType;
};

// Config's ConfigProfile is different from CommonTypes - use a separate struct name
struct ConfigProfile {
    std::string profileId;
    std::string profileName;
    std::string description;
    std::string createdAt;
    std::string modifiedAt;
    
    DeviceConfig device;
    NetworkConfig network;
    SystemConfig system;
};


class Config {
public:
    static Config& getInstance();
    
    bool loadFromFile(const std::string& filepath);
    bool saveToFile(const std::string& filepath);
    
    void setDeviceConfig(const DeviceConfig& config);
    DeviceConfig getDeviceConfig() const;
    
    void setNetworkConfig(const NetworkConfig& config);
    NetworkConfig getNetworkConfig() const;
    
    void setSystemConfig(const SystemConfig& config);
    SystemConfig getSystemConfig() const;
    
    std::map<std::string, ConfigProfile>& getProfiles();
    bool addProfile(const ConfigProfile& profile);
    bool removeProfile(const std::string& profileId);
    ConfigProfile* getProfile(const std::string& profileId);

private:
    Config();
    ~Config();
    
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    
    std::string configToJson();
    bool jsonToConfig(const std::string& json);
    
    DeviceConfig m_deviceConfig;
    NetworkConfig m_networkConfig;
    SystemConfig m_systemConfig;
    std::map<std::string, ConfigProfile> m_profiles;
    mutable std::mutex m_mutex;
};

}
