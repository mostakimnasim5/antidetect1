#pragma once

/**
 * BrowserFingerprinting - Complete Browser Fingerprint Simulation
 * 
 * For web testing purposes - simulates realistic browser fingerprints
 * to test anti-fingerprinting measures and browser automation detection.
 */

#include "../VirtualPhonePro.hpp"
#include <map>
#include <vector>
#include <variant>

namespace VirtualPhonePro {

enum class BrowserType { CHROME, FIREFOX, SAFARI, EDGE, OPERA, BRAVE };
enum class PlatformType { WINDOWS, MACOS, LINUX, ANDROID };

struct CanvasFingerprint {
    std::string hash;
    std::vector<uint8_t> rawData;
    std::string renderer;
    std::string vendor;
};

struct WebGLFingerprint {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::vector<std::string> extensions;
    std::string shadingLanguageVersion;
    std::map<std::string, std::string> parameters;
};

struct AudioFingerprint {
    std::string hash;
    std::vector<float> output;
};

struct BrowserProfile {
    BrowserType browser;
    std::string userAgent;
    std::string platform;
    int screenWidth;
    int screenHeight;
    int screenColorDepth;
    int devicePixelRatio;
    int hardwareConcurrency;
    int deviceMemory;
    int maxTouchPoints;
    std::string timezone;
    int timezoneOffset;
    std::string language;
    std::vector<std::string> languages;
    CanvasFingerprint canvas;
    WebGLFingerprint webgl;
    AudioFingerprint audio;
    std::vector<std::string> installedFonts;
    std::vector<std::string> plugins;
    std::map<std::string, std::string> permissions;
};

struct AutomationDetection {
    bool hasWebdriver = false;
    bool hasChromeFlags = false;
    bool hasNavigatorWebdriver = false;
    bool hasSelenium = false;
    bool hasPhantomJS = false;
    bool hasChromeRuntime = false;
    bool hasPermissions = false;
};

class BrowserFingerprinting {
public:
    static BrowserFingerprinting& getInstance();
    
    BrowserProfile generateRandomProfile();
    BrowserProfile generateChromeProfile();
    BrowserProfile generateFirefoxProfile();
    BrowserProfile generateSafariProfile();
    BrowserProfile generateEdgeProfile();
    
    void loadProfile(const BrowserProfile& profile);
    BrowserProfile getCurrentProfile();
    
    CanvasFingerprint generateCanvasFingerprint(const std::string& text);
    WebGLFingerprint generateWebGLFingerprint();
    AudioFingerprint generateAudioFingerprint();
    
    std::vector<std::string> getInstalledFonts();
    std::map<std::string, std::string> getNavigatorProperties();
    std::map<std::string, int> getScreenProperties();
    std::map<std::string, std::string> getTimezoneProperties();
    std::map<std::string, std::string> getAutomationBypassProperties();
    
    std::map<std::string, std::variant<int, float, bool>> getBatteryStatus();
    std::map<std::string, std::variant<std::string, int, bool>> getConnectionInfo();

private:
    BrowserFingerprinting();
    ~BrowserFingerprinting();
    
    std::string generateUserAgent(BrowserType browser);
    std::string hashString(const std::string& input);
    std::vector<uint8_t> simulateCanvasRendering(const std::string& text);
    std::vector<std::string> getChromeExtensions();
    std::vector<std::string> getFirefoxExtensions();
    
    BrowserProfile m_currentProfile;
    std::mutex m_mutex;
    std::random_device m_rd;
    std::mt19937 m_gen;
};

} // namespace VirtualPhonePro
