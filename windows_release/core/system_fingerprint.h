/**
 * OS-Level Anti-Detect System
 * Professional-grade fingerprint manipulation at system level
 * 
 * This module provides low-level system fingerprint control without
 * relying on JavaScript or third-party APIs.
 */

#ifndef SYSTEM_FINGERPRINT_H
#define SYSTEM_FINGERPRINT_H

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <mutex>

namespace AntiDetect {

// ============================================
// SYSTEM IDENTIFIER TYPES
// ============================================

struct HardwareIdentifiers {
    std::string machine_id;          // /etc/machine-id
    std::string product_uuid;        // /sys/class/dmi/id/product_uuid
    std::string board_serial;        // DMI board serial
    std::string chassis_serial;      // DMI chassis serial
    std::string bios_version;        // BIOS/UEFI version
    std::string baseboard_version;   // Baseboard version
};

struct NetworkIdentifiers {
    std::string mac_address;         // MAC address
    std::string hostname;            // System hostname
    std::string domain_name;         // Network domain
    std::string dns_servers;         // Custom DNS
};

struct SystemInfo {
    std::string kernel_version;
    std::string os_version;
    std::string architecture;
    std::string cpu_model;
    int cpu_cores;
    int cpu_threads;
    std::string memory_total;
    std::string gpu_info;
    std::string kernel_args;
};

struct BrowserProfile {
    std::string profile_id;
    std::string user_agent;
    std::string platform;
    std::string accept_language;
    std::string accept_encoding;
    std::string screen_resolution;
    std::string color_depth;
    std::string timezone;
    std::string canvas_fingerprint;
    std::string webgl_vendor;
    std::string webgl_renderer;
    std::string audio_fingerprint;
    std::string fonts_hash;
    bool hardware_acceleration;
    bool webrtc_enabled;
};

// ============================================
// CORE INTERFACES
// ============================================

class ISystemManipulator {
public:
    virtual ~ISystemManipulator() = default;
    virtual bool apply() = 0;
    virtual bool revert() = 0;
    virtual std::string getStatus() const = 0;
};

class IFingerprintGenerator {
public:
    virtual ~IFingerprintGenerator() = default;
    virtual std::string generate() = 0;
    virtual bool validate(const std::string& value) = 0;
};

// ============================================
// FINGERPRINT MANAGER
// ============================================

class SystemFingerprintManager {
public:
    static SystemFingerprintManager& getInstance();
    
    // Hardware manipulation
    bool spoofMachineId(const std::string& new_id);
    bool spoofProductUUID(const std::string& new_uuid);
    bool spoofMACAddress(const std::string& interface, const std::string& new_mac);
    bool spoofDMIInfo(const std::map<std::string, std::string>& dmi_values);
    
    // Network manipulation
    bool spoofHostname(const std::string& new_hostname);
    bool setCustomDNS(const std::vector<std::string>& dns_servers);
    bool enableProxy(const std::string& proxy_url);
    bool disableIPv6();
    
    // Browser profile management
    std::string createBrowserProfile(const BrowserProfile& profile);
    bool applyBrowserProfile(const std::string& profile_id);
    bool deleteBrowserProfile(const std::string& profile_id);
    BrowserProfile getBrowserProfile(const std::string& profile_id);
    
    // System info retrieval
    HardwareIdentifiers getHardwareIdentifiers();
    NetworkIdentifiers getNetworkIdentifiers();
    SystemInfo getSystemInfo();
    
    // Batch operations
    bool applyFingerprintPreset(const std::string& preset_name);
    bool resetToRealSystem();
    
    // Cleanup
    void cleanup();

private:
    SystemFingerprintManager();
    ~SystemFingerprintManager();
    SystemFingerprintManager(const SystemFingerprintManager&) = delete;
    SystemFingerprintManager& operator=(const SystemFingerprintManager&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

// ============================================
// PRESET MANAGER
// ============================================

struct FingerprintPreset {
    std::string name;
    std::string description;
    std::string region;
    
    // Hardware
    std::string machine_id;
    std::string product_uuid;
    std::string mac_prefix;
    
    // Network
    std::string hostname_pattern;
    std::string timezone;
    std::string language;
    
    // Browser
    std::string user_agent;
    std::string platform;
    std::string screen_resolution;
};

class PresetManager {
public:
    static PresetManager& getInstance();
    
    // Preset operations
    void registerPreset(const FingerprintPreset& preset);
    bool applyPreset(const std::string& preset_name);
    FingerprintPreset getPreset(const std::string& preset_name);
    std::vector<std::string> listPresets();
    
    // Default presets
    void loadDefaultPresets();

private:
    PresetManager();
    ~PresetManager();
    PresetManager(const PresetManager&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> pImpl;
};

// ============================================
// UTILITY FUNCTIONS
// ============================================

namespace Utils {
    std::string generateRandomHex(int length);
    std::string generateRandomMAC(const std::string& prefix = "");
    std::string generateRandomUUID();
    std::string generateMachineId();
    std::string hashString(const std::string& input);
    bool writeToFile(const std::string& path, const std::string& content);
    std::string readFromFile(const std::string& path);
    bool fileExists(const std::string& path);
    std::string executeCommand(const std::string& cmd);
    bool isRoot();
}

} // namespace AntiDetect

#endif // SYSTEM_FINGERPRINT_H