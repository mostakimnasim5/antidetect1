#pragma once

#include <string>
#include <vector>
#include <map>

namespace AntiDetect {

struct SystemResult {
    bool success;
    std::string operation;
    std::string message;
    std::string error;
};

enum class SystemState {
    NORMAL,
    MODIFIED,
    ERROR
};

class SystemManager {
public:
    SystemManager();
    ~SystemManager();
    
    bool initialize();
    bool isInitialized() const;
    
    std::map<std::string, std::string> getSystemInfo();
    
    SystemResult enableSELinux();
    SystemResult disableSELinux();
    SystemResult getSELinuxStatus();
    
    SystemResult enableDebugMode();
    SystemResult disableDebugMode();
    
    SystemResult enableRootDetectionBypass();
    SystemResult disableRootDetectionBypass();
    
    SystemResult setTimezone(const std::string& timezone);
    SystemResult setLocale(const std::string& locale);
    SystemResult setLanguage(const std::string& language);
    
    SystemResult setBatteryStatus(int level, const std::string& status, const std::string& chargingType);
    
    SystemResult enableDSU(const std::string& gsiImagePath);
    SystemResult disableDSU();
    SystemResult getDSUStatus();
    
    SystemResult grantRootAccess();
    SystemResult revokeRootAccess();
    
    SystemResult installGms();
    SystemResult uninstallGms();
    
    bool isDeviceRooted();
    bool isSELinuxEnforcing();
    bool isDebugEnabled();
    bool isMockLocationEnabled();
    
    bool resetAllChanges();
    bool backupSystemState();
    bool restoreSystemState();

private:
    bool executeSystemCommand(const std::string& command);
    std::string getSystemStateValue(const std::string& key);
    bool setSystemStateValue(const std::string& key, const std::string& value);
    
    std::map<std::string, std::string> m_originalSystemState;
    std::vector<std::string> m_appliedChanges;
    SystemState m_currentState;
    bool m_initialized;
    
    static const std::vector<std::string> SELINUX_PROPERTIES;
    static const std::vector<std::string> DEBUG_PROPERTIES;
    static const std::vector<std::string> ROOT_DETECTION_PROPERTIES;
};

}
