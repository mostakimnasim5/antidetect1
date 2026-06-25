#pragma once

/**
 * BrowserFingerprinting - Enterprise-Grade Browser Fingerprint Simulation
 * 
 * Provides realistic browser fingerprint generation with perfect canvas,
 * WebGL, and audio fingerprints that are indistinguishable from real devices.
 */

#include "../VirtualPhonePro.hpp"
#include <map>
#include <vector>
#include <variant>
#include <mutex>
#include <random>
#include <complex>
#include <cmath>

namespace VirtualPhonePro {

enum class BrowserType { CHROME, FIREFOX, SAFARI, EDGE, OPERA, BRAVE };
enum class PlatformType { WINDOWS, MACOS, LINUX, ANDROID };

struct CanvasFingerprint {
    std::string hash;
    std::vector<uint8_t> rawData;
    std::string renderer;
    std::string vendor;
    std::string gpuMaskedVendor;
    std::string gpuMaskedRenderer;
    int width;
    int height;
};

struct WebGLFingerprint {
    std::string vendor;
    std::string renderer;
    std::string version;
    std::string version2;
    std::string shadingLanguageVersion;
    std::string shadingLanguageVersion2;
    std::string unmaskedVendor;
    std::string unmaskedRenderer;
    std::vector<std::string> extensions;
    std::vector<std::string> extensions2;
    std::map<std::string, std::string> parameters;
    std::map<std::string, std::string> parameters2;
    int maxTextureSize;
    int maxCubeMapTextureSize;
    int maxRenderbufferSize;
};

struct AudioFingerprint {
    std::string hash;
    std::string hash48kHz;
    std::string hash96kHz;
    std::vector<double> frequencyData;
    std::vector<double> waveformData;
    double sampleRate;
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
    
    // Unique identifiers
    std::string clientId;
    std::string sessionId;
    std::string generatedTime;
    
    // Hardware info
    std::string gpuVendor;
    std::string gpuRenderer;
    std::string cpuModel;
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
    
    // Enhanced fingerprinting
    CanvasFingerprint generateCanvasFingerprint(const std::string& text);
    WebGLFingerprint generateWebGLFingerprint();
    WebGLFingerprint generateWebGL2Fingerprint();
    AudioFingerprint generateAudioFingerprint();
    AudioFingerprint generateAudioFingerprint48kHz();
    AudioFingerprint generateAudioFingerprint96kHz();
    
    std::vector<std::string> getInstalledFonts();
    std::map<std::string, std::string> getNavigatorProperties();
    std::map<std::string, int> getScreenProperties();
    std::map<std::string, std::string> getTimezoneProperties();
    std::map<std::string, std::string> getAutomationBypassProperties();
    
    std::map<std::string, std::variant<int, float, bool>> getBatteryStatus();
    std::map<std::string, std::variant<std::string, int, bool>> getConnectionInfo();
    
    // Unique ID generation
    std::string generateClientId();
    std::string generateSessionId();
    
private:
    BrowserFingerprinting();
    ~BrowserFingerprinting();
    
    std::string generateUserAgent(BrowserType browser);
    std::string hashString(const std::string& input);
    std::string hashBytes(const uint8_t* data, size_t length);
    std::vector<uint8_t> simulateCanvasRendering(const std::string& text);
    std::vector<uint8_t> simulateGPUCanvasRendering(
        const std::string& text,
        const std::string& gpuVendor,
        const std::string& gpuRenderer);
    std::vector<std::string> getChromeExtensions();
    std::vector<std::string> getFirefoxExtensions();
    std::vector<std::string> getChromeExtensions2();
    
    // Audio processing
    std::vector<double> processAudioSignal(int sampleRate, int duration);
    double computeAudioFingerprint(const std::vector<double>& samples);
    uint32_t crc32(const uint8_t* data, size_t length);
    
    BrowserProfile m_currentProfile;
    std::mutex m_mutex;
    std::random_device m_rd;
    std::mt19937 m_gen;
};

} // namespace VirtualPhonePro
