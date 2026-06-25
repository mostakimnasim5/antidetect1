#pragma once

/**
 * ProcEmulator - Realistic /proc/ filesystem emulation for anti-detection
 * 
 * Generates realistic /proc/cpuinfo, /proc/version, /proc/uptime, etc.
 * that match real device fingerprints exactly.
 * 
 * This is critical for passing advanced detection checks from:
 * - Banking apps with root detection
 * - Security apps (AV, MDM)
 * - Google Play integrity checks
 */

#include "../VirtualPhonePro.hpp"
#include <map>
#include <vector>
#include <string>

namespace VirtualPhonePro {

// Realistic CPU configurations per manufacturer/model
struct CPUConfig {
    std::string processor;      // "Processor" field
    std::string hardware;       // "Hardware" field
    std::string modelName;      // "model name" (ARM doesn't have this)
    std::string implementer;     // "CPU implementer"
    std::string architecture;   // "Architecture"
    std::vector<int> BogoMIPS; // Per-core BogoMIPS
    std::vector<int> features;  // Feature flags
};

// Realistic kernel configurations
struct KernelConfig {
    std::string version;        // Full version string
    std::string linuxVersion;   // "Linux version X.Y.Z"
    std::string compiler;       // GCC version used
    std::string buildDate;      // Build timestamp
};

class ProcEmulator {
public:
    static ProcEmulator& getInstance();
    
    // Initialize with specific device configuration
    void initialize(const std::string& manufacturer, const std::string& model);
    
    // /proc/cpuinfo generation
    std::string getCpuInfo();
    std::string getCpuInfoForCore(int core);
    
    // /proc/version
    std::string getVersion();
    
    // /proc/uptime
    std::string getUptime();
    
    // /proc/meminfo
    std::string getMemInfo();
    
    // /proc/self/mounts
    std::string getMounts();
    
    // /proc/self/status
    std::string getStatus();
    
    // /proc/self/cmdline
    std::string getCmdLine();
    
    // /sys/class/
    std::string getBatteryStatus();
    std::string getThermalStatus();
    
    // Realistic device info
    void setDeviceInfo(
        const std::string& manufacturer,
        const std::string& model,
        const std::string& hardware
    );
    
    // Set boot timing (for realistic uptime)
    void setBootTime(time_t bootTime);
    
    // Get all /proc/ values at once
    std::map<std::string, std::string> getAllProcValues();
    
private:
    ProcEmulator();
    ~ProcEmulator();
    
    // Load CPU configurations for different devices
    void loadSamsungCPUConfig();
    void loadGoogleCPUConfig();
    void loadXiaomiCPUConfig();
    void loadOnePlusCPUConfig();
    
    // Generate realistic values
    std::string generateCpuImplementer();
    std::string generateCpuVariant();
    std::string generateCpuPart();
    std::string generateCpuRevision();
    std::string generateFeatures();
    
    // Device configuration
    std::string m_manufacturer;
    std::string m_model;
    std::string m_hardware;
    std::string m_cpuHardware;
    
    // Boot time for uptime calculation
    time_t m_bootTime;
    
    // CPU configuration
    CPUConfig m_cpuConfig;
    KernelConfig m_kernelConfig;
    
    // Secure random for realistic variance
    std::string generateRandomHex(int bytes);
};

} // namespace VirtualPhonePro
