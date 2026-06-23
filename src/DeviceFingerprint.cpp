#include "DeviceFingerprint.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"

namespace AntiDetect {

const std::vector<std::string> DeviceFingerprint::DEVICE_PROPERTIES = {
    "ro.product.manufacturer",
    "ro.product.brand",
    "ro.product.model",
    "ro.product.device",
    "ro.product.name",
    "ro.product.first_api_level",
    "ro.product.cpu.abi",
    "ro.product.cpu.abi2"
};

const std::vector<std::string> DeviceFingerprint::BUILD_PROPERTIES = {
    "ro.build.id",
    "ro.build.display.id",
    "ro.build.version.release",
    "ro.build.version.sdk",
    "ro.build.version.security_patch",
    "ro.build.fingerprint",
    "ro.build.tags",
    "ro.build.type",
    "ro.build.user",
    "ro.build.host",
    "ro.build.flavor",
    "ro.build.product"
};

const std::vector<std::string> DeviceFingerprint::DISPLAY_PROPERTIES = {
    "ro.sf.lcd_density",
    "ro.sf.lcd_density_value",
    "ro.product.property_source",
    "ro.setupwizard.mode",
    "sys.display-size",
    "persist.sys.display-size"
};

const std::vector<std::string> DeviceFingerprint::HARDWARE_PROPERTIES = {
    "ro.hardware",
    "ro.board.platform",
    "ro.arch",
    "ro.kernel.qemu",
    "ro.bootimage.build.fingerprint",
    "ro.vendor.product.cpu.abi",
    "ro.vendor.product.manufacturer",
    "ro.vendor.product.brand",
    "ro.vendor.product.model",
    "ro.vendor.build.fingerprint",
    "vendor.sys.ggm.loaded",
    "vendor.debug.egl.cfg",
    "debug.hwui.render_adreno_profiler.idle_timeout",
    "persist.sys.angle.hwui.disable",
    "ro.opengles.version"
};

DeviceFingerprint::DeviceFingerprint()
    : m_initialized(false)
{
}

DeviceFingerprint::~DeviceFingerprint() {
    if (m_initialized && !m_appliedChanges.empty()) {
        Logger::getInstance().warning("DeviceFingerprint destroyed with uncommitted changes");
    }
}

bool DeviceFingerprint::initialize() {
    Logger::getInstance().info("Initializing Device Fingerprint Engine...");
    
    auto& adb = ADBManager::getInstance();
    if (!adb.isConnected()) {
        Logger::getInstance().error("ADB not connected - cannot initialize fingerprint engine");
        return false;
    }
    
    Logger::getInstance().info("Capturing original device fingerprint...");
    
    m_originalValues.clear();
    std::vector<std::string> allProps = getRelevantDeviceProperties();
    
    for (const auto& prop : allProps) {
        std::string value = adb.getProperty(prop);
        if (!value.empty()) {
            m_originalValues[prop] = value;
            m_currentValues[prop] = value;
            Logger::getInstance().debug("Captured: " + prop + " = " + value);
        }
    }
    
    m_initialized = true;
    Logger::getInstance().info("Fingerprint engine initialized. Captured " + 
                               std::to_string(m_originalValues.size()) + " properties");
    
    return true;
}

bool DeviceFingerprint::isInitialized() const {
    return m_initialized;
}

std::map<std::string, std::string> DeviceFingerprint::getCurrentFingerprint() {
    std::map<std::string, std::string> fingerprint;
    
    auto& adb = ADBManager::getInstance();
    for (const auto& prop : getRelevantDeviceProperties()) {
        std::string value = adb.getProperty(prop);
        if (!value.empty()) {
            fingerprint[prop] = value;
        }
    }
    
    return fingerprint;
}

std::vector<std::string> DeviceFingerprint::getFingerprintCategories() {
    return {
        "DEVICE_INFO",
        "HARDWARE_INFO",
        "BUILD_PROPERTIES",
        "DISPLAY_INFO",
        "NETWORK_INFO",
        "SENSOR_INFO",
        "SYSTEM_INFO"
    };
}

std::string DeviceFingerprint::getPropertyValue(const std::string& property) {
    return ADBManager::getInstance().getProperty(property);
}

bool DeviceFingerprint::applyPropertyChange(const std::string& property, const std::string& value) {
    auto& adb = ADBManager::getInstance();
    
    std::string currentValue = adb.getProperty(property);
    
    if (m_originalValues.find(property) == m_originalValues.end()) {
        m_originalValues[property] = currentValue;
    }
    
    if (value == currentValue) {
        Logger::getInstance().debug("Property already set: " + property + " = " + value);
        return true;
    }
    
    if (m_currentValues[property] == value) {
        Logger::getInstance().debug("Property change already applied: " + property);
        return true;
    }
    
    bool success = adb.setProperty(property, value);
    
    if (success) {
        m_currentValues[property] = value;
        
        bool alreadyInList = false;
        for (const auto& p : m_appliedChanges) {
            if (p == property) {
                alreadyInList = true;
                break;
            }
        }
        if (!alreadyInList) {
            m_appliedChanges.push_back(property);
        }
        
        Logger::getInstance().info("Applied: " + property + " = " + value);
    } else {
        Logger::getInstance().error("Failed to set property: " + property);
    }
    
    return success;
}

bool DeviceFingerprint::backupOriginalValue(const std::string& property, const std::string& value) {
    if (m_originalValues.find(property) == m_originalValues.end()) {
        m_originalValues[property] = value;
        return true;
    }
    return false;
}

bool DeviceFingerprint::restoreOriginalValue(const std::string& property) {
    auto it = m_originalValues.find(property);
    if (it != m_originalValues.end()) {
        return applyPropertyChange(property, it->second);
    }
    return false;
}

FingerprintResult DeviceFingerprint::spoofDeviceManufacturer(const std::string& manufacturer) {
    FingerprintResult result = {false, "ro.product.manufacturer", "", manufacturer, ""};
    
    result.originalValue = getPropertyValue("ro.product.manufacturer");
    
    std::vector<std::string> properties = {
        "ro.product.manufacturer",
        "ro.product.brand",
        "ro.product.name",
        "ro.vendor.product.manufacturer",
        "ro.vendor.product.brand"
    };
    
    for (const auto& prop : properties) {
        if (!applyPropertyChange(prop, manufacturer)) {
            result.error = "Failed to set: " + prop;
        }
    }
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofDeviceModel(const std::string& model) {
    FingerprintResult result = {false, "ro.product.model", "", model, ""};
    
    result.originalValue = getPropertyValue("ro.product.model");
    
    std::vector<std::string> properties = {
        "ro.product.model",
        "ro.product.device",
        "ro.product.name",
        "ro.vendor.product.model",
        "ro.vendor.product.device"
    };
    
    for (const auto& prop : properties) {
        if (!applyPropertyChange(prop, model)) {
            result.error = "Failed to set: " + prop;
        }
    }
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofDeviceBrand(const std::string& brand) {
    FingerprintResult result = {false, "ro.product.brand", "", brand, ""};
    
    result.originalValue = getPropertyValue("ro.product.brand");
    
    std::vector<std::string> properties = {
        "ro.product.brand",
        "ro.vendor.product.brand"
    };
    
    for (const auto& prop : properties) {
        if (!applyPropertyChange(prop, brand)) {
            result.error = "Failed to set: " + prop;
        }
    }
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofBuildFingerprint(const std::string& fingerprint) {
    FingerprintResult result = {false, "ro.build.fingerprint", "", fingerprint, ""};
    
    result.originalValue = getPropertyValue("ro.build.fingerprint");
    
    std::vector<std::string> properties = {
        "ro.build.fingerprint",
        "ro.bootimage.build.fingerprint",
        "ro.vendor.build.fingerprint"
    };
    
    for (const auto& prop : properties) {
        applyPropertyChange(prop, fingerprint);
    }
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofBuildId(const std::string& buildId) {
    FingerprintResult result = {false, "ro.build.id", "", buildId, ""};
    
    result.originalValue = getPropertyValue("ro.build.id");
    result.success = applyPropertyChange("ro.build.id", buildId);
    
    if (result.success) {
        applyPropertyChange("ro.build.display.id", buildId);
    }
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofAndroidVersion(const std::string& version) {
    FingerprintResult result = {"ro.build.version.release", "", "", version, ""};
    
    result.originalValue = getPropertyValue("ro.build.version.release");
    result.success = applyPropertyChange("ro.build.version.release", version);
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofSDKVersion(const std::string& sdkVersion) {
    FingerprintResult result = {"ro.build.version.sdk", "", "", sdkVersion, ""};
    
    result.originalValue = getPropertyValue("ro.build.version.sdk");
    result.success = applyPropertyChange("ro.build.version.sdk", sdkVersion);
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofSecurityPatch(const std::string& patch) {
    FingerprintResult result = {"ro.build.version.security_patch", "", "", patch, ""};
    
    result.originalValue = getPropertyValue("ro.build.version.security_patch");
    result.success = applyPropertyChange("ro.build.version.security_patch", patch);
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofScreenResolution(int width, int height) {
    FingerprintResult result = {false, "", "", std::to_string(width) + "x" + std::to_string(height), ""};
    
    result.originalValue = getPropertyValue("ro.sf.lcd_density");
    
    std::string resolution = std::to_string(width) + "x" + std::to_string(height);
    
    applyPropertyChange("sys.display-size", resolution);
    applyPropertyChange("persist.sys.display-size", resolution);
    applyPropertyChange("persist.sys.viewpixelsperinch", "640");
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofScreenDensity(int density) {
    FingerprintResult result = {"ro.sf.lcd_density", "", "", std::to_string(density), ""};
    
    result.originalValue = getPropertyValue("ro.sf.lcd_density");
    
    applyPropertyChange("ro.sf.lcd_density", std::to_string(density));
    applyPropertyChange("ro.sf.lcd_density_value", std::to_string(density));
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofScreenDPI(const std::string& dpi) {
    FingerprintResult result = {"ro.sf.lcd_density", "", "", dpi, ""};
    
    result.originalValue = getPropertyValue("ro.sf.lcd_density");
    
    applyPropertyChange("ro.sf.lcd_density", dpi);
    applyPropertyChange("persist.sys.display-density", dpi);
    applyPropertyChange("persist.sys.viewpixelsperinch", dpi);
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofHardwareInfo(const std::string& hardware, const std::string& board) {
    FingerprintResult result = {"ro.hardware", "", "", hardware, ""};
    
    result.originalValue = getPropertyValue("ro.hardware");
    
    applyPropertyChange("ro.hardware", hardware);
    applyPropertyChange("ro.board.platform", hardware);
    applyPropertyChange("ro.arch", hardware);
    
    if (!board.empty()) {
        applyPropertyChange("ro.product.board", board);
    }
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofGPUInfo(const std::string& vendor, const std::string& renderer) {
    FingerprintResult result = {false, "", "", vendor + " " + renderer, ""};
    
    applyPropertyChange("debug.hwui.render_adreno_profiler.idle_timeout", "0");
    applyPropertyChange("persist.sys.angle.hwui.disable", "1");
    
    Logger::getInstance().info("GPU spoofed: " + vendor + " - " + renderer);
    result.success = true;
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofOpenGLVersion(const std::string& version) {
    FingerprintResult result = {"ro.opengles.version", "", "", version, ""};
    
    result.originalValue = getPropertyValue("ro.opengles.version");
    result.success = applyPropertyChange("ro.opengles.version", version);
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofBootloader(const std::string& bootloader) {
    FingerprintResult result = {"ro.build.bootloader", "", "", bootloader, ""};
    
    result.originalValue = getPropertyValue("ro.build.bootloader");
    result.success = applyPropertyChange("ro.build.bootloader", bootloader);
    
    return result;
}

FingerprintResult DeviceFingerprint::spoofRadioVersion(const std::string& version) {
    FingerprintResult result = {"ro.modem.wifi.version", "", "", version, ""};
    
    result.originalValue = getPropertyValue("ro.modem.wifi.version");
    
    applyPropertyChange("ro.modem.wifi.version", version);
    applyPropertyChange("ro.radio.version", version);
    
    result.success = true;
    return result;
}

FingerprintResult DeviceFingerprint::spoofDeviceName(const std::string& name) {
    FingerprintResult result = {"ro.product.device", "", "", name, ""};
    
    result.originalValue = getPropertyValue("ro.product.device");
    
    applyPropertyChange("ro.product.device", name);
    applyPropertyChange("ro.product.name", name);
    
    result.success = true;
    return result;
}

bool DeviceFingerprint::resetToFactory() {
    Logger::getInstance().info("Resetting all fingerprint changes...");
    
    bool allSuccess = true;
    for (const auto& change : m_appliedChanges) {
        if (!resetProperty(change)) {
            allSuccess = false;
        }
    }
    
    m_appliedChanges.clear();
    m_currentValues = m_originalValues;
    
    return allSuccess;
}

bool DeviceFingerprint::resetProperty(const std::string& property) {
    auto it = m_originalValues.find(property);
    if (it != m_originalValues.end()) {
        bool success = applyPropertyChange(property, it->second);
        if (success) {
            m_appliedChanges.erase(
                std::remove(m_appliedChanges.begin(), m_appliedChanges.end(), property),
                m_appliedChanges.end()
            );
        }
        return success;
    }
    return false;
}

std::string DeviceFingerprint::getOriginalValue(const std::string& property) {
    auto it = m_originalValues.find(property);
    if (it != m_originalValues.end()) {
        return it->second;
    }
    return "";
}

std::vector<std::string> DeviceFingerprint::getAllSystemProperties() {
    std::vector<std::string> properties;
    std::string output = ADBManager::getInstance().executeShellCommand("getprop");
    
    std::istringstream stream(output);
    std::string line;
    while (std::getline(stream, line)) {
        if (!line.empty() && line[0] == '[') {
            size_t start = line.find('[');
            size_t mid1 = line.find(']', start);
            size_t mid2 = line.find('[', mid1 + 1);
            size_t end = line.find(']', mid2 + 1);
            
            if (start != std::string::npos && end != std::string::npos) {
                std::string prop = line.substr(mid1 + 1, mid2 - mid1 - 1);
                std::string value = line.substr(mid2 + 1, end - mid2 - 1);
                properties.push_back(prop);
            }
        }
    }
    
    return properties;
}

std::vector<std::string> DeviceFingerprint::getRelevantDeviceProperties() {
    std::vector<std::string> all;
    all.insert(all.end(), DEVICE_PROPERTIES.begin(), DEVICE_PROPERTIES.end());
    all.insert(all.end(), BUILD_PROPERTIES.begin(), BUILD_PROPERTIES.end());
    all.insert(all.end(), DISPLAY_PROPERTIES.begin(), DISPLAY_PROPERTIES.end());
    all.insert(all.end(), HARDWARE_PROPERTIES.begin(), HARDWARE_PROPERTIES.end());
    return all;
}

bool DeviceFingerprint::applyFullProfile(const std::map<std::string, std::string>& profile) {
    Logger::getInstance().info("Applying full fingerprint profile...");
    
    bool allSuccess = true;
    for (const auto& entry : profile) {
        if (!applyPropertyChange(entry.first, entry.second)) {
            Logger::getInstance().warning("Failed to apply: " + entry.first);
            allSuccess = false;
        }
    }
    
    return allSuccess;
}

}
