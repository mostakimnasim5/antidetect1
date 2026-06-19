#pragma once

#include <string>
#include <vector>
#include <map>

namespace AntiDetect {

struct NetworkSpoofResult {
    bool success;
    std::string category;
    std::string originalValue;
    std::string newValue;
    std::string error;
};

class NetworkSpoofer {
public:
    NetworkSpoofer();
    ~NetworkSpoofer();
    
    bool initialize();
    bool isInitialized() const;
    
    std::map<std::string, std::string> getCurrentNetworkInfo();
    
    NetworkSpoofResult spoofMACAddress(const std::string& macAddress);
    NetworkSpoofResult spoofWiFiMACAddress(const std::string& macAddress);
    NetworkSpoofResult spoofBluetoothMAC(const std::string& macAddress);
    NetworkSpoofResult spoofEthernetMAC(const std::string& macAddress);
    
    NetworkSpoofResult spoofCarrierName(const std::string& carrierName);
    NetworkSpoofResult spoofCarrierCountry(const std::string& country);
    NetworkSpoofResult spoofSIMOperator(const std::string& operatorCode);
    NetworkSpoofResult spoofNetworkType(const std::string& networkType);
    
    NetworkSpoofResult spoofLocation(double latitude, double longitude);
    NetworkSpoofResult enableGPSSpoofing();
    NetworkSpoofResult disableGPSSpoofing();
    
    NetworkSpoofResult enableMockLocation();
    NetworkSpoofResult disableMockLocation();
    
    std::string generateRandomMAC(const std::string& prefix = "");
    std::string generateAndroidID();
    
    bool resetAll();
    bool resetMAC();
    bool resetCarrier();
    bool resetLocation();

private:
    bool applyNetworkSetting(const std::string& setting, const std::string& value);
    std::string getNetworkSetting(const std::string& setting);
    
    bool backupOriginalValue(const std::string& setting, const std::string& value);
    
    std::map<std::string, std::string> m_originalNetworkSettings;
    std::vector<std::string> m_appliedSettings;
    bool m_initialized;
    
    std::string generateRandomHex(int length);
    std::string formatMAC(const std::string& mac);
    
    bool setMockLocationApp(const std::string& packageName);
    bool clearMockLocationData();
    
    static const std::vector<std::string> CARRIER_PROPERTIES;
    static const std::vector<std::string> LOCATION_PROPERTIES;
};

}
