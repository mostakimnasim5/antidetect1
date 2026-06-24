#include "NetworkSpoofer.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include "openssl_stub.h"
#include "openssl_stub.h"

namespace AntiDetect {

const std::vector<std::string> NetworkSpoofer::CARRIER_PROPERTIES = {
    "gsm.operator.alpha",
    "gsm.operator.numeric",
    "persist.sys.operator.iso-country",
    "persist.sys.operator.name",
    "ro.carrier",
    "ro.config.carrier",
    "ro.cdma.operator.numeric",
    "ro.cdma.operator.alpha",
    "telephony.sms.pseudo_multipart"
};

const std::vector<std::string> NetworkSpoofer::LOCATION_PROPERTIES = {
    "mock_location",
    "persist.gps.noGpsSignal",
    "location.mode",
    "locationProvidersAllowed"
};

NetworkSpoofer::NetworkSpoofer()
    : m_initialized(false)
{
}

NetworkSpoofer::~NetworkSpoofer() {
}

bool NetworkSpoofer::initialize() {
    Logger::getInstance().info("Initializing Network Spoofer...");
    
    auto& adb = ADBManager::getInstance();
    if (!adb.isConnected()) {
        Logger::getInstance().error("ADB not connected - cannot initialize network spoofer");
        return false;
    }
    
    m_originalNetworkSettings.clear();
    
    std::string macAddr = adb.executeShellCommand("settings get secure android_id");
    if (!macAddr.empty()) {
        m_originalNetworkSettings["android_id"] = macAddr;
    }
    
    std::string carrier = adb.getProperty("ro.carrier");
    if (!carrier.empty()) {
        m_originalNetworkSettings["ro.carrier"] = carrier;
    }
    
    m_initialized = true;
    Logger::getInstance().info("Network Spoofer initialized successfully");
    
    return true;
}

bool NetworkSpoofer::isInitialized() const {
    return m_initialized;
}

std::map<std::string, std::string> NetworkSpoofer::getCurrentNetworkInfo() {
    std::map<std::string, std::string> info;
    auto& adb = ADBManager::getInstance();
    
    info["wlan0_mac"] = adb.executeShellCommand("cat /sys/class/net/wlan0/address");
    info["android_id"] = adb.executeShellCommand("settings get secure android_id");
    info["carrier"] = adb.getProperty("ro.carrier");
    info["operator"] = adb.getProperty("gsm.operator.alpha");
    info["network_type"] = adb.executeShellCommand("getprop gsm.network.type");
    
    return info;
}

std::string NetworkSpoofer::generateRandomHex(int length) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    for (int i = 0; i < length; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

std::string NetworkSpoofer::formatMAC(const std::string& mac) {
    std::string formatted = mac;
    std::transform(formatted.begin(), formatted.end(), formatted.begin(), ::toupper);
    
    if (formatted.find(':') == std::string::npos && formatted.length() == 12) {
        std::string result;
        for (size_t i = 0; i < formatted.length(); i += 2) {
            if (i > 0) result += ':';
            result += formatted.substr(i, 2);
        }
        return result;
    }
    
    return formatted;
}

std::string NetworkSpoofer::generateRandomMAC(const std::string& prefix) {
    std::string mac;
    
    if (!prefix.empty()) {
        mac = prefix;
        while (mac.length() < 12) {
            mac += generateRandomHex(2);
        }
    } else {
        std::vector<std::string> validPrefixes = {
            "AA:BB:CC", "00:1A:2B", "A1:B2:C3", "D4:E5:F6",
            "DC:D0:0A", "54:27:1E", "F0:1F:AF", "00:0C:29",
            "00:50:56", "08:00:27"
        };
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, static_cast<int>(validPrefixes.size()) - 1);
        
        mac = validPrefixes[dis(gen)];
        
        for (int i = 0; i < 3; ++i) {
            mac += ":" + generateRandomHex(2);
        }
    }
    
    return formatMAC(mac);
}

std::string NetworkSpoofer::generateAndroidID() {
    return generateRandomHex(16);
}

bool NetworkSpoofer::applyNetworkSetting(const std::string& setting, const std::string& value) {
    auto& adb = ADBManager::getInstance();
    
    if (setting == "android_id") {
        std::string cmd = "settings put secure android_id " + value;
        std::string result = adb.executeShellCommand(cmd);
        return result.find("error") == std::string::npos;
    }
    else if (setting == "mock_location") {
        std::string cmd = "settings put secure mock_location " + value;
        adb.executeShellCommand(cmd);
        return true;
    }
    else {
        return adb.setProperty(setting, value);
    }
}

std::string NetworkSpoofer::getNetworkSetting(const std::string& setting) {
    auto& adb = ADBManager::getInstance();
    
    if (setting == "android_id") {
        return adb.executeShellCommand("settings get secure android_id");
    }
    else {
        return adb.getProperty(setting);
    }
}

bool NetworkSpoofer::backupOriginalValue(const std::string& setting, const std::string& value) {
    if (m_originalNetworkSettings.find(setting) == m_originalNetworkSettings.end()) {
        m_originalNetworkSettings[setting] = value;
        return true;
    }
    return false;
}

NetworkSpoofResult NetworkSpoofer::spoofMACAddress(const std::string& macAddress) {
    NetworkSpoofResult result = {false, "MAC", "", macAddress, "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string currentMAC = adb.executeShellCommand("cat /sys/class/net/wlan0/address 2>/dev/null");
    result.originalValue = currentMAC;
    
    std::string formattedMAC = formatMAC(macAddress);
    
    std::string cmd = "ip link set wlan0 address " + formattedMAC;
    std::string setResult = adb.executeShellCommand(cmd);
    
    if (setResult.find("error") != std::string::npos || setResult.find("RTNETLINK") != std::string::npos) {
        cmd = "ip link set wlan0 address " + formattedMAC;
        adb.executeShellCommand(cmd);
    }
    
    adb.executeShellCommand("svc wifi disable");
    adb.executeShellCommand("ip link set wlan0 down");
    adb.executeShellCommand("ip link set wlan0 address " + formattedMAC);
    adb.executeShellCommand("ip link set wlan0 up");
    adb.executeShellCommand("svc wifi enable");
    
    adb.setProperty("persist.net.wlan0.mac", formattedMAC);
    adb.setProperty("ro.mac.address", formattedMAC);
    
    backupOriginalValue("wlan0_mac", currentMAC);
    
    result.success = true;
    Logger::getInstance().info("MAC address spoofed to: " + formattedMAC);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofWiFiMACAddress(const std::string& macAddress) {
    return spoofMACAddress(macAddress);
}

NetworkSpoofResult NetworkSpoofer::spoofBluetoothMAC(const std::string& macAddress) {
    NetworkSpoofResult result = {false, "Bluetooth", "", macAddress, "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string currentMAC = adb.executeShellCommand("settings get secure bluetooth_address");
    result.originalValue = currentMAC;
    
    std::string formattedMAC = formatMAC(macAddress);
    
    adb.executeShellCommand("service call bluetooth_manager 8 i32 0 i32 0 i32 0");
    
    adb.setProperty("persist.bluetooth.bdaddr", formattedMAC);
    adb.executeShellCommand("settings put secure bluetooth_address " + formattedMAC);
    
    backupOriginalValue("bluetooth_mac", currentMAC);
    
    result.success = true;
    Logger::getInstance().info("Bluetooth MAC spoofed to: " + formattedMAC);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofEthernetMAC(const std::string& macAddress) {
    NetworkSpoofResult result = {false, "Ethernet", "", macAddress, "", ""};
    
    auto& adb = ADBManager::getInstance();
    std::string formattedMAC = formatMAC(macAddress);
    
    adb.executeShellCommand("ip link set eth0 down");
    adb.executeShellCommand("ip link set eth0 address " + formattedMAC);
    adb.executeShellCommand("ip link set eth0 up");
    
    adb.setProperty("persist.net.eth.mac", formattedMAC);
    
    backupOriginalValue("eth0_mac", "");
    
    result.success = true;
    Logger::getInstance().info("Ethernet MAC spoofed to: " + formattedMAC);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofCarrierName(const std::string& carrierName) {
    NetworkSpoofResult result = {false, "Carrier", "", carrierName, "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    result.originalValue = adb.getProperty("ro.carrier");
    
    adb.setProperty("ro.carrier", carrierName);
    adb.setProperty("ro.config.carrier", carrierName);
    adb.setProperty("gsm.operator.alpha", carrierName);
    adb.executeShellCommand("settings put global carrier_name " + carrierName);
    
    bool alreadyInList = false;
    for (const auto& s : m_appliedSettings) {
        if (s == "ro.carrier") {
            alreadyInList = true;
            break;
        }
    }
    if (!alreadyInList) {
        m_appliedSettings.push_back("ro.carrier");
    }
    
    result.success = true;
    Logger::getInstance().info("Carrier name spoofed to: " + carrierName);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofCarrierCountry(const std::string& country) {
    NetworkSpoofResult result = {false, "Country", "", country, "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("persist.sys.operator.iso-country", country);
    adb.setProperty("ro.cdma.mcc", country);
    adb.executeShellCommand("settings put global carrier_country " + country);
    
    backupOriginalValue("country", "");
    
    result.success = true;
    Logger::getInstance().info("Carrier country spoofed to: " + country);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofSIMOperator(const std::string& operatorCode) {
    NetworkSpoofResult result = {false, "SIM Operator", "", operatorCode, "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("gsm.operator.numeric", operatorCode);
    adb.setProperty("ro.cdma.operator.numeric", operatorCode);
    adb.executeShellCommand("settings put secure sim_operator_numeric " + operatorCode);
    
    backupOriginalValue("sim_operator", "");
    
    result.success = true;
    Logger::getInstance().info("SIM operator spoofed to: " + operatorCode);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofNetworkType(const std::string& networkType) {
    NetworkSpoofResult result = {false, "Network Type", "", networkType, "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("gsm.network.type", networkType);
    adb.setProperty("ro.telephony.default_network", networkType);
    
    std::map<std::string, std::string> networkMap = {
        {"5G", "16"},
        {"LTE", "9"},
        {"4G", "9"},
        {"3G", "3"},
        {"HSPA", "5"},
        {"2G", "1"}
    };
    
    auto it = networkMap.find(networkType);
    if (it != networkMap.end()) {
        adb.executeShellCommand("settings put global preferred_network_mode " + it->second);
    }
    
    backupOriginalValue("network_type", "");
    
    result.success = true;
    Logger::getInstance().info("Network type spoofed to: " + networkType);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::spoofLocation(double latitude, double longitude) {
    NetworkSpoofResult result = {false, "Location", "", 
                                  std::to_string(latitude) + "," + std::to_string(longitude), ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << latitude;
    std::string latStr = ss.str();
    
    ss.str("");
    ss << std::fixed << std::setprecision(6) << longitude;
    std::string lonStr = ss.str();
    
    adb.setProperty("mock_location", "1");
    adb.executeShellCommand("settings put secure mock_location 1");
    adb.executeShellCommand("settings put secure location_providers_allowed +gps");
    
    adb.executeShellCommand("appops set android.mock_location allow");
    
    backupOriginalValue("latitude", "");
    backupOriginalValue("longitude", "");
    
    result.success = true;
    Logger::getInstance().info("Location spoofed to: " + latStr + ", " + lonStr);
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::enableGPSSpoofing() {
    NetworkSpoofResult result = {false, "GPS", "", "", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings put secure location_providers_allowed +gps");
    adb.executeShellCommand("settings put global location_mode 3");
    adb.setProperty("persist.gps.noGpsSignal", "false");
    
    result.success = true;
    Logger::getInstance().info("GPS spoofing enabled");
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::disableGPSSpoofing() {
    NetworkSpoofResult result = {false, "GPS", "", "", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings put secure location_providers_allowed -gps");
    adb.executeShellCommand("settings put global location_mode 0");
    
    result.success = true;
    Logger::getInstance().info("GPS spoofing disabled");
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::enableMockLocation() {
    NetworkSpoofResult result = {false, "Mock Location", "", "", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings put secure mock_location 1");
    adb.executeShellCommand("appops set android.mock_location allow");
    adb.setProperty("mock_location", "1");
    
    result.success = true;
    Logger::getInstance().info("Mock location enabled");
    
    return result;
}

NetworkSpoofResult NetworkSpoofer::disableMockLocation() {
    NetworkSpoofResult result = {false, "Mock Location", "", "", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings put secure mock_location 0");
    adb.executeShellCommand("appops set android.mock_location deny");
    adb.setProperty("mock_location", "0");
    
    result.success = true;
    Logger::getInstance().info("Mock location disabled");
    
    return result;
}

bool NetworkSpoofer::setMockLocationApp(const std::string& packageName) {
    auto& adb = ADBManager::getInstance();
    
    std::string cmd = "appops set " + packageName + " android:mock_location allow";
    std::string result = adb.executeShellCommand(cmd);
    
    return result.find("Unknown") == std::string::npos;
}

bool NetworkSpoofer::clearMockLocationData() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings delete secure mock_location");
    adb.executeShellCommand("settings delete secure location_providers_allowed");
    
    return true;
}

bool NetworkSpoofer::resetAll() {
    Logger::getInstance().info("Resetting all network spoofing changes...");
    
    bool allSuccess = true;
    
    for (const auto& entry : m_originalNetworkSettings) {
        if (!applyNetworkSetting(entry.first, entry.second)) {
            allSuccess = false;
        }
    }
    
    clearMockLocationData();
    m_appliedSettings.clear();
    
    return allSuccess;
}

bool NetworkSpoofer::resetMAC() {
    auto it = m_originalNetworkSettings.find("wlan0_mac");
    if (it != m_originalNetworkSettings.end()) {
        spoofMACAddress(it->second);
        return true;
    }
    return false;
}

bool NetworkSpoofer::resetCarrier() {
    auto it = m_originalNetworkSettings.find("ro.carrier");
    if (it != m_originalNetworkSettings.end()) {
        spoofCarrierName(it->second);
        return true;
    }
    return false;
}

bool NetworkSpoofer::resetLocation() {
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings put secure mock_location 0");
    adb.executeShellCommand("settings put secure location_providers_allowed -gps,-network");
    
    return true;
}

}
