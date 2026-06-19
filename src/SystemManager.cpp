#include "SystemManager.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include <algorithm>

namespace AntiDetect {

const std::vector<std::string> SystemManager::SELINUX_PROPERTIES = {
    "ro.build.selinux.enforce",
    "ro.build.selinux.status",
    "security.selinux.enforce"
};

const std::vector<std::string> SystemManager::DEBUG_PROPERTIES = {
    "ro.debuggable",
    "ro.adb.secure",
    "persist.sys.usb.config"
};

const std::vector<std::string> SystemManager::ROOT_DETECTION_PROPERTIES = {
    "ro.setupwizard.mode",
    "ro.build.tags",
    "ro.build.type",
    "system_as_root"
};

SystemManager::SystemManager()
    : m_currentState(SystemState::NORMAL)
    , m_initialized(false)
{
}

SystemManager::~SystemManager() {
}

bool SystemManager::initialize() {
    Logger::getInstance().info("Initializing System Manager...");
    
    auto& adb = ADBManager::getInstance();
    if (!adb.isConnected()) {
        Logger::getInstance().error("ADB not connected - cannot initialize system manager");
        return false;
    }
    
    m_originalSystemState.clear();
    
    for (const auto& prop : SELINUX_PROPERTIES) {
        std::string value = adb.getProperty(prop);
        if (!value.empty()) {
            m_originalSystemState[prop] = value;
        }
    }
    
    for (const auto& prop : DEBUG_PROPERTIES) {
        std::string value = adb.getProperty(prop);
        if (!value.empty()) {
            m_originalSystemState[prop] = value;
        }
    }
    
    std::string selinuxStatus = adb.executeShellCommand("getenforce");
    m_originalSystemState["selinux_current"] = selinuxStatus;
    
    std::string rootStatus = adb.executeShellCommand("which su");
    m_originalSystemState["root_available"] = rootStatus.empty() ? "false" : "true";
    
    m_initialized = true;
    m_currentState = SystemState::NORMAL;
    
    Logger::getInstance().info("System Manager initialized successfully");
    
    return true;
}

bool SystemManager::isInitialized() const {
    return m_initialized;
}

std::map<std::string, std::string> SystemManager::getSystemInfo() {
    std::map<std::string, std::string> info;
    auto& adb = ADBManager::getInstance();
    
    info["android_version"] = adb.getProperty("ro.build.version.release");
    info["sdk_version"] = adb.getProperty("ro.build.version.sdk");
    info["security_patch"] = adb.getProperty("ro.build.version.security_patch");
    info["build_id"] = adb.getProperty("ro.build.id");
    info["build_type"] = adb.getProperty("ro.build.type");
    info["build_tags"] = adb.getProperty("ro.build.tags");
    
    info["selinux_status"] = adb.executeShellCommand("getenforce");
    info["debuggable"] = adb.getProperty("ro.debuggable");
    
    std::string rootCheck = adb.executeShellCommand("su -c id 2>/dev/null || echo 'not_root'");
    info["root_status"] = (rootCheck.find("uid=0") != std::string::npos) ? "rooted" : "not_rooted";
    
    info["timezone"] = adb.executeShellCommand("getprop persist.sys.timezone");
    info["locale"] = adb.executeShellCommand("getprop persist.sys.locale");
    info["language"] = adb.getProperty("ro.product.locale");
    
    info["battery_level"] = adb.executeShellCommand("dumpsys battery | grep level");
    info["battery_status"] = adb.executeShellCommand("dumpsys battery | grep status");
    
    return info;
}

bool SystemManager::executeSystemCommand(const std::string& command) {
    auto& adb = ADBManager::getInstance();
    std::string result = adb.executeShellCommand(command);
    return result.find("error") == std::string::npos && result.find("fail") == std::string::npos;
}

std::string SystemManager::getSystemStateValue(const std::string& key) {
    auto& adb = ADBManager::getInstance();
    return adb.getProperty(key);
}

bool SystemManager::setSystemStateValue(const std::string& key, const std::string& value) {
    auto& adb = ADBManager::getInstance();
    return adb.setProperty(key, value);
}

SystemResult SystemManager::enableSELinux() {
    SystemResult result = {false, "SELinux", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string currentStatus = adb.executeShellCommand("getenforce");
    Logger::getInstance().info("Current SELinux status: " + currentStatus);
    
    std::string cmd = "setenforce 1";
    std::string output = adb.executeShellCommand(cmd);
    
    std::string newStatus = adb.executeShellCommand("getenforce");
    
    if (newStatus.find("Enforcing") != std::string::npos) {
        result.success = true;
        result.message = "SELinux enabled - Enforcing mode";
        m_currentState = SystemState::MODIFIED;
        
        m_appliedChanges.push_back("selinux_enforce");
        
        Logger::getInstance().info(result.message);
    } else {
        result.error = "Failed to enable SELinux - may require root privileges";
        Logger::getInstance().error(result.error);
    }
    
    return result;
}

SystemResult SystemManager::disableSELinux() {
    SystemResult result = {false, "SELinux", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string currentStatus = adb.executeShellCommand("getenforce");
    Logger::getInstance().info("Current SELinux status: " + currentStatus);
    
    std::string cmd = "setenforce 0";
    std::string output = adb.executeShellCommand(cmd);
    
    std::string newStatus = adb.executeShellCommand("getenforce");
    
    if (newStatus.find("Permissive") != std::string::npos) {
        result.success = true;
        result.message = "SELinux disabled - Permissive mode";
        m_currentState = SystemState::MODIFIED;
        
        m_appliedChanges.push_back("selinux_permissive");
        
        Logger::getInstance().info(result.message);
    } else {
        result.error = "Failed to disable SELinux - may require root privileges";
        Logger::getInstance().error(result.error);
    }
    
    return result;
}

SystemResult SystemManager::getSELinuxStatus() {
    SystemResult result = {true, "SELinux", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string status = adb.executeShellCommand("getenforce");
    status.erase(std::remove(status.begin(), status.end(), '\n'), status.end());
    status.erase(std::remove(status.begin(), status.end(), '\r'), status.end());
    
    result.message = "Current SELinux status: " + status;
    
    if (status.find("Enforcing") != std::string::npos) {
        result.message += " (Enforcing)";
    } else if (status.find("Permissive") != std::string::npos) {
        result.message += " (Permissive)";
    }
    
    return result;
}

SystemResult SystemManager::enableDebugMode() {
    SystemResult result = {false, "Debug", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("ro.debuggable", "1");
    adb.executeShellCommand("settings put global adb_enabled 1");
    adb.setProperty("ro.adb.secure", "0");
    
    adb.executeShellCommand("setprop persist.service.debuggable 1");
    adb.executeShellCommand("setprop service.adb.enable 1");
    
    result.success = true;
    result.message = "Debug mode enabled";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("debug_enabled");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::disableDebugMode() {
    SystemResult result = {false, "Debug", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("ro.debuggable", "0");
    adb.setProperty("ro.adb.secure", "1");
    
    adb.executeShellCommand("settings put global developer_options 0");
    
    result.success = true;
    result.message = "Debug mode disabled";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("debug_disabled");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::enableRootDetectionBypass() {
    SystemResult result = {false, "Root Detection", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("mount -o rw,remount /system");
    
    adb.executeShellCommand("mv /system/xbin/su /system/xbin/su.bak 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/su /system/bin/su.bak 2>/dev/null || true");
    
    adb.setProperty("ro.build.tags", "release-keys");
    adb.setProperty("ro.build.type", "user");
    adb.setProperty("ro.setupwizard.mode", "OPTIONAL");
    
    adb.executeShellCommand("pm hide com.topjohnwu.magisk 2>/dev/null || true");
    adb.executeShellCommand("pm disable-user --user 0 com.topjohnwu.magisk 2>/dev/null || true");
    
    result.success = true;
    result.message = "Root detection bypass enabled";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("root_bypass_enabled");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::disableRootDetectionBypass() {
    SystemResult result = {false, "Root Detection", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("mv /system/xbin/su.bak /system/xbin/su 2>/dev/null || true");
    adb.executeShellCommand("mv /system/bin/su.bak /system/bin/su 2>/dev/null || true");
    
    adb.executeShellCommand("pm enable com.topjohnwu.magisk 2>/dev/null || true");
    
    result.success = true;
    result.message = "Root detection bypass disabled";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("root_bypass_disabled");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::setTimezone(const std::string& timezone) {
    SystemResult result = {false, "Timezone", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string cmd = "setprop persist.sys.timezone " + timezone;
    std::string output = adb.executeShellCommand(cmd);
    
    adb.executeShellCommand("settings put global time_zone_24_hour 1");
    
    result.success = true;
    result.message = "Timezone set to: " + timezone;
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("timezone:" + timezone);
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::setLocale(const std::string& locale) {
    SystemResult result = {false, "Locale", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("settings put global device_provisioned false");
    adb.setProperty("persist.sys.locale", locale);
    adb.setProperty("persist.sys.language", locale.substr(0, 2));
    adb.setProperty("persist.sys.country", locale.substr(3, 2));
    
    adb.executeShellCommand("settings put global locale_force_three_letters_language_name false");
    
    result.success = true;
    result.message = "Locale set to: " + locale;
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("locale:" + locale);
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::setLanguage(const std::string& language) {
    SystemResult result = {false, "Language", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.setProperty("persist.sys.language", language);
    adb.setProperty("ro.product.locale.language", language);
    adb.executeShellCommand("settings put secure locale_display_native_locale_name true");
    
    result.success = true;
    result.message = "Language set to: " + language;
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("language:" + language);
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::setBatteryStatus(int level, const std::string& status, const std::string& chargingType) {
    SystemResult result = {false, "Battery", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string levelStr = std::to_string(level);
    
    adb.executeShellCommand("dumpsys battery set level " + levelStr);
    
    if (status == "Charging") {
        adb.executeShellCommand("dumpsys battery set status 2");
    } else if (status == "Discharging") {
        adb.executeShellCommand("dumpsys battery set status 3");
    } else if (status == "Full") {
        adb.executeShellCommand("dumpsys battery set status 5");
    }
    
    if (chargingType == "USB") {
        adb.executeShellCommand("dumpsys battery set usb 1");
        adb.executeShellCommand("dumpsys battery set ac 0");
    } else if (chargingType == "AC") {
        adb.executeShellCommand("dumpsys battery set ac 1");
        adb.executeShellCommand("dumpsys battery set usb 0");
    } else if (chargingType == "Wireless") {
        adb.executeShellCommand("dumpsys battery set wireless 1");
    }
    
    adb.executeShellCommand("dumpsys battery set invalid 0");
    
    result.success = true;
    result.message = "Battery status set - Level: " + levelStr + "%, Status: " + status;
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("battery");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::enableDSU(const std::string& gsiImagePath) {
    SystemResult result = {false, "DSU", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string cmd = "ssu ssl";
    adb.executeShellCommand(cmd);
    
    cmd = "cmd tasker dsu --load " + gsiImagePath;
    std::string output = adb.executeShellCommand(cmd);
    
    if (output.find("success") != std::string::npos || output.find("DSU") != std::string::npos) {
        result.success = true;
        result.message = "DSU enabled with image: " + gsiImagePath;
        m_currentState = SystemState::MODIFIED;
        
        m_appliedChanges.push_back("dsu_enabled");
        
        Logger::getInstance().info(result.message);
    } else {
        result.error = "Failed to enable DSU - device may not support DSU or image path is invalid";
        Logger::getInstance().error(result.error);
    }
    
    return result;
}

SystemResult SystemManager::disableDSU() {
    SystemResult result = {false, "DSU", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string cmd = "cmd tasker dsu --unload";
    std::string output = adb.executeShellCommand(cmd);
    
    result.success = true;
    result.message = "DSU disabled";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("dsu_disabled");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::getDSUStatus() {
    SystemResult result = {true, "DSU", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string status = adb.executeShellCommand("cmd tasker dsu --status");
    
    result.message = "DSU Status: " + status;
    
    return result;
}

SystemResult SystemManager::grantRootAccess() {
    SystemResult result = {false, "Root", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::string suCheck = adb.executeShellCommand("which su");
    if (suCheck.empty()) {
        result.error = "Root (su) binary not found on device";
        Logger::getInstance().error(result.error);
        return result;
    }
    
    adb.executeShellCommand("su -c 'mount -o rw,remount /system'");
    
    adb.executeShellCommand("chmod 6755 /system/bin/su 2>/dev/null || true");
    adb.executeShellCommand("chmod 6755 /system/xbin/su 2>/dev/null || true");
    
    adb.setProperty("ro.adb.secure", "0");
    adb.setProperty("ro.debuggable", "1");
    
    result.success = true;
    result.message = "Root access granted";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("root_granted");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::revokeRootAccess() {
    SystemResult result = {false, "Root", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    adb.executeShellCommand("chmod 755 /system/bin/su 2>/dev/null || true");
    adb.executeShellCommand("chmod 755 /system/xbin/su 2>/dev/null || true");
    
    adb.setProperty("ro.adb.secure", "1");
    
    result.success = true;
    result.message = "Root access revoked";
    m_currentState = SystemState::MODIFIED;
    
    m_appliedChanges.push_back("root_revoked");
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::installGms() {
    SystemResult result = {false, "GMS", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::vector<std::string> gmsPackages = {
        "com.google.android.gms",
        "com.google.android.gsf",
        "com.google.android.gsf.login",
        "com.google.android.apps.wellbeing",
        "com.google.android.backuptransport",
        "com.google.android.onemediacontrol"
    };
    
    int installed = 0;
    for (const auto& pkg : gmsPackages) {
        std::string cmd = "pm install-existing " + pkg + " 2>/dev/null || echo 'not_found'";
        std::string output = adb.executeShellCommand(cmd);
        if (output.find("not_found") == std::string::npos) {
            installed++;
        }
    }
    
    result.success = true;
    result.message = "GMS installation completed - " + std::to_string(installed) + " packages processed";
    m_currentState = SystemState::MODIFIED;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SystemResult SystemManager::uninstallGms() {
    SystemResult result = {false, "GMS", "", ""};
    
    auto& adb = ADBManager::getInstance();
    
    std::vector<std::string> gmsPackages = {
        "com.google.android.gms",
        "com.google.android.gsf",
        "com.google.android.gsf.login",
        "com.google.android.apps.wellbeing"
    };
    
    int uninstalled = 0;
    for (const auto& pkg : gmsPackages) {
        std::string cmd = "pm uninstall -k " + pkg;
        std::string output = adb.executeShellCommand(cmd);
        if (output.find("success") != std::string::npos || output.find("Failure") == std::string::npos) {
            uninstalled++;
        }
    }
    
    result.success = true;
    result.message = "GMS uninstallation completed - " + std::to_string(uninstalled) + " packages processed";
    m_currentState = SystemState::MODIFIED;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

bool SystemManager::isDeviceRooted() {
    auto& adb = ADBManager::getInstance();
    
    std::string suCheck = adb.executeShellCommand("which su");
    if (suCheck.empty()) {
        return false;
    }
    
    std::string rootCheck = adb.executeShellCommand("su -c id 2>/dev/null || echo 'not_root'");
    return rootCheck.find("uid=0") != std::string::npos;
}

bool SystemManager::isSELinuxEnforcing() {
    auto& adb = ADBManager::getInstance();
    std::string status = adb.executeShellCommand("getenforce");
    return status.find("Enforcing") != std::string::npos;
}

bool SystemManager::isDebugEnabled() {
    auto& adb = ADBManager::getInstance();
    std::string debuggable = adb.getProperty("ro.debuggable");
    return debuggable.find("1") != std::string::npos;
}

bool SystemManager::isMockLocationEnabled() {
    auto& adb = ADBManager::getInstance();
    std::string mockLocation = adb.executeShellCommand("settings get secure mock_location");
    return mockLocation.find("1") != std::string::npos;
}

bool SystemManager::resetAllChanges() {
    Logger::getInstance().info("Resetting all system changes...");
    
    auto& adb = ADBManager::getInstance();
    
    for (const auto& change : m_appliedChanges) {
        if (change.find("selinux") != std::string::npos) {
            adb.executeShellCommand("setenforce 1");
        } else if (change.find("debug") != std::string::npos) {
            adb.setProperty("ro.debuggable", "0");
        } else if (change.find("timezone:") != std::string::npos) {
            std::string tz = change.substr(9);
            adb.executeShellCommand("setprop persist.sys.timezone " + tz);
        } else if (change.find("locale:") != std::string::npos) {
            std::string locale = change.substr(7);
            adb.setProperty("persist.sys.locale", locale);
        }
    }
    
    m_appliedChanges.clear();
    m_currentState = SystemState::NORMAL;
    
    Logger::getInstance().info("All system changes reset");
    
    return true;
}

bool SystemManager::backupSystemState() {
    Logger::getInstance().info("Backing up system state...");
    
    m_originalSystemState.clear();
    
    auto& adb = ADBManager::getInstance();
    
    for (const auto& prop : SELINUX_PROPERTIES) {
        std::string value = adb.getProperty(prop);
        if (!value.empty()) {
            m_originalSystemState[prop] = value;
        }
    }
    
    for (const auto& prop : DEBUG_PROPERTIES) {
        std::string value = adb.getProperty(prop);
        if (!value.empty()) {
            m_originalSystemState[prop] = value;
        }
    }
    
    Logger::getInstance().info("System state backed up successfully");
    
    return true;
}

bool SystemManager::restoreSystemState() {
    Logger::getInstance().info("Restoring system state...");
    
    auto& adb = ADBManager::getInstance();
    
    for (const auto& entry : m_originalSystemState) {
        adb.setProperty(entry.first, entry.second);
    }
    
    m_currentState = SystemState::NORMAL;
    
    Logger::getInstance().info("System state restored successfully");
    
    return true;
}

}
