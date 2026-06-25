/**
 * SELinuxEmulator - Realistic SELinux Context Emulation
 */

#include "anti_detect/SELinuxEmulator.hpp"
#include <sstream>
#include <fstream>

namespace VirtualPhonePro {

SELinuxEmulator::SELinuxEmulator() 
    : m_policyVersion("31.0")
    , m_policyBuild("20230101.1")
    , m_state(SELinuxEnforceState::ENFORCING) {
}

SELinuxEmulator::~SELinuxEmulator() {}

SELinuxEmulator& SELinuxEmulator::getInstance() {
    static SELinuxEmulator instance;
    return instance;
}

void SELinuxEmulator::initialize() {
    // Android 12/13 process contexts
    m_processContexts = {
        {"init", "u:object_r:init:s0"},
        {"zygote", "u:object_r:zygote_exec:s0"},
        {"zygote64", "u:object_r:zygote_exec:s0"},
        {"system_server", "u:object_r:system_server_exec:s0"},
        {"surfaceflinger", "u:object_r:surfaceflinger_exec:s0"},
        {"servicemanager", "u:object_r:servicemanager_exec:s0"},
        {"hwservicemanager", "u:object_r:hwservicemanager_exec:s0"},
        {"vndservicemanager", "u:object_r:vndservicemanager_exec:s0"},
        {"bootanim", "u:object_r:bootanim_exec:s0"},
        {"netd", "u:object_r:netd_exec:s0"},
        {"tid", "u:object_r:untrusted_app_25_exec:s0"},
        {"com.android.systemui", "u:object_r:system_app_service:s0"},
        {"com.google.android.gms", "u:object_r:platform_app_service:s0"},
        {"untrusted_app", "u:object_r:untrusted_app_exec:s0"},
        {"untrusted_app_29", "u:object_r:untrusted_app_29_exec:s0"},
        {"untrusted_app_30", "u:object_r:untrusted_app_30_exec:s0"},
        {"untrusted_app_31", "u:object_r:untrusted_app_31_exec:s0"},
        {"untrusted_app_32", "u:object_r:untrusted_app_32_exec:s0"},
        {"platform_app", "u:object_r:platform_app_exec:s0"},
        {"mediaserver", "u:object_r:mediaserver_exec:s0"},
        {"audioserver", "u:object_r:audioserver_exec:s0"},
        {"cameraserver", "u:object_r:cameraserver_exec:s0"},
        {"drmserver", "u:object_r:drmserver_exec:s0"},
        {"installd", "u:object_r:installd_exec:s0"},
        {"keystore", "u:object_r:keystore_exec:s0"},
        {"gatekeeperd", "u:object_r:gatekeeperd_exec:s0"},
        {"vold", "u:object_r:vold_exec:s0"},
        {"statsd", "u:object_r:statsd_exec:s0"},
        {"perfetto", "u:object_r:perfetto_exec:s0"},
    };
    
    // File contexts
    m_fileContexts = {
        {"/system", "u:object_r:system_file:s0"},
        {"/system/bin/surfaceflinger", "u:object_r:surfaceflinger_exec:s0"},
        {"/system/bin/system_server", "u:object_r:system_server_exec:s0"},
        {"/system/bin/app_process64", "u:object_r:zygote_exec:s0"},
        {"/system/bin/bootanimation", "u:object_r:bootanim_exec:s0"},
        {"/system/lib", "u:object_r:system_lib_file:s0"},
        {"/system/lib64", "u:object_r:system_lib_file:s0"},
        {"/system/apex", "u:object_r:system_file:s0"},
        {"/vendor", "u:object_r:vendor_file:s0"},
        {"/vendor/lib", "u:object_r:vendor_lib_file:s0"},
        {"/vendor/lib64", "u:object_r:vendor_lib_file:s0"},
        {"/product", "u:object_r:product_file:s0"},
        {"/data", "u:object_r:system_data_file:s0"},
        {"/data/data", "u:object_r:app_data_file:s0"},
        {"/data/app", "u:object_r:apk_data_file:s0"},
        {"/data/dalvik-cache", "u:object_r:dalvikcache_data_file:s0"},
        {"/data/local/tmp", "u:object_r:shell_data_file:s0"},
        {"/proc", "u:object_r:proc:s0"},
        {"/sys", "u:object_r:sysfs:s0"},
        {"/dev", "u:object_r:device:s0"},
        {"/dev/block", "u:object_r:block_device:s0"},
        {"/dev/urandom", "u:object_r:urandom_device:s0"},
        {"/dev/null", "u:object_r:null_device:s0"},
        {"/dev/zero", "u:object_r:zero_device:s0"},
    };
    
    // Socket contexts
    m_socketContexts = {
        {"surfaceflinger", "u:object_r:surfaceflinger_socket:s0"},
        {"system_server", "u:object_r:system_server_socket:s0"},
        {"zygote", "u:object_r:zygote_socket:s0"},
        {"servicemanager", "u:object_r:servicemanager_socket:s0"},
        {"crash_dump", "u:object_r:crash_dump_socket:s0"},
    };
}

SELinuxEnforceState SELinuxEmulator::getEnforceState() {
    return m_state;
}

bool SELinuxEmulator::isEnforcing() {
    return m_state == SELinuxEnforceState::ENFORCING;
}

std::string SELinuxEmulator::getProcessContext(const std::string& processName) {
    auto it = m_processContexts.find(processName);
    if (it != m_processContexts.end()) {
        return it->second;
    }
    
    // Default for unknown processes
    if (processName.find("com.") == 0) {
        return "u:object_r:untrusted_app_exec:s0";
    }
    
    return "u:object_r:shell_exec:s0";
}

std::string SELinuxEmulator::getFileContext(const std::string& path) {
    // Check for exact match first
    auto it = m_fileContexts.find(path);
    if (it != m_fileContexts.end()) {
        return it->second;
    }
    
    // Check for prefix match
    for (const auto& pair : m_fileContexts) {
        if (path.find(pair.first) == 0) {
            return pair.second;
        }
    }
    
    // Default
    if (path.find("/system") == 0) {
        return "u:object_r:system_file:s0";
    }
    if (path.find("/data") == 0) {
        return "u:object_r:system_data_file:s0";
    }
    if (path.find("/vendor") == 0) {
        return "u:object_r:vendor_file:s0";
    }
    
    return "u:object_r:default_exec:s0";
}

std::string SELinuxEmulator::getSocketContext(const std::string& socketName) {
    auto it = m_socketContexts.find(socketName);
    if (it != m_socketContexts.end()) {
        return it->second;
    }
    return "u:object_r:unlabeled:s0";
}

bool SELinuxEmulator::isTransitionAllowed(
    const std::string& sourceContext,
    const std::string& targetContext,
    const std::string& objectClass
) {
    // In enforcing mode, allow realistic transitions
    // This is simplified - real SELinux policy is much more complex
    
    if (m_state != SELinuxEnforceState::ENFORCING) {
        return true;
    }
    
    // Zygote spawning apps
    if (sourceContext == "u:object_r:zygote_exec:s0" && 
        targetContext.find("untrusted_app") != std::string::npos) {
        return true;
    }
    
    // System server
    if (sourceContext == "u:object_r:system_server_exec:s0") {
        return true;
    }
    
    // Default deny
    return false;
}

std::map<std::string, std::string> SELinuxEmulator::getPolicyInfo() {
    std::map<std::string, std::string> info;
    info["PolicyVersion"] = m_policyVersion;
    info["PolicyBuild"] = m_policyBuild;
    info["MLSEnabled"] = "true";
    info["EnforceMode"] = isEnforcing() ? "Enforcing" : "Permissive";
    info["LoadedPolicy"] = "58";
    return info;
}

std::string SELinuxEmulator::getFileContexts() {
    std::stringstream ss;
    
    ss << "# Realistic Android 13 file contexts\n";
    
    for (const auto& pair : m_fileContexts) {
        ss << pair.first << "\t" << pair.second << "\n";
    }
    
    // Common patterns
    ss << "/system/bin/.*\tu:object_r:system_file:s0\n";
    ss << "/system/xbin/.*\tu:object_r:system_file:s0\n";
    ss << "/system/app/.*\tu:object_r:system_app_file:s0\n";
    ss << "/system/priv-app/.*\tu:object_r:system_app_file:s0\n";
    ss << "/data/app/.*\tu:object_r:app_data_file:s0\n";
    ss << "/data/data/.*\tu:object_r:app_data_file:s0\n";
    ss << "/data/local/tmp/.*\tu:object_r:shell_data_file:s0\n";
    ss << "/data/local/.*\tu:object_r:shell_data_file:s0\n";
    ss << "/vendor/bin/.*\tu:object_r:vendor_bin_file:s0\n";
    ss << "/product/bin/.*\tu:object_r:product_bin_file:s0\n";
    
    return ss.str();
}

std::string SELinuxEmulator::getSeAppContexts() {
    std::stringstream ss;
    
    ss << "# Android SELinux app contexts\n";
    ss << "name=system_server domain=system_server type=system_data_file\n";
    ss << "name=platform_app domain=platform_app type=platform_app_data_file levelFrom=user\n";
    ss << "name=shared_gid domain=platform_app type=app_data_file levelFrom=user\n";
    ss << "isSystemServer=true domain=system_server type=system_data_file\n";
    ss << "user=system seinfo=platform_app domain=platform_app type=system_data_file\n";
    ss << "user=_app seinfo=default domain=untrusted_app type=app_data_file levelFrom=user\n";
    ss << "user=_app seinfo=default domain=untrusted_app_29 type=app_data_file levelFrom=user\n";
    ss << "user=_app seinfo=default domain=untrusted_app_30 type=app_data_file levelFrom=user\n";
    ss << "user=_app seinfo=default domain=untrusted_app_31 type=app_data_file levelFrom=user\n";
    ss << "user=_app seinfo=default domain=untrusted_app_32 type=app_data_file levelFrom=user\n";
    ss << "user=_app isTrainingMode=true domain=untrusted_app type=app_data_file levelFrom=all\n";
    ss << "user=bluetooth seinfo=platform_app domain=bluetooth type=bluetooth_data_file levelFrom=user\n";
    ss << "user=nfc seinfo=platform_app domain=nfc type=nfc_data_file levelFrom=user\n";
    ss << "user=radio seinfo=platform_app domain=radio type=radio_data_file levelFrom=user\n";
    
    return ss.str();
}

} // namespace VirtualPhonePro
