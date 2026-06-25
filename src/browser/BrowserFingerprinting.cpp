/**
 * BrowserFingerprinting - Complete Browser Fingerprint Simulation Implementation
 */

#include "browser/BrowserFingerprinting.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace VirtualPhonePro {

BrowserFingerprinting::BrowserFingerprinting()
    : m_rd(), m_gen(m_rd()) {
    m_currentProfile = generateChromeProfile();
}

BrowserFingerprinting::~BrowserFingerprinting() {}

BrowserFingerprinting& BrowserFingerprinting::getInstance() {
    static BrowserFingerprinting instance;
    return instance;
}

BrowserProfile BrowserFingerprinting::generateChromeProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::CHROME;
    profile.userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
    profile.platform = "Win32";
    profile.screenWidth = 1920;
    profile.screenHeight = 1080;
    profile.screenColorDepth = 24;
    profile.devicePixelRatio = 1.0;
    profile.hardwareConcurrency = 8;
    profile.deviceMemory = 8;
    profile.maxTouchPoints = 0;
    profile.timezone = "Asia/Dhaka";
    profile.timezoneOffset = -360;
    profile.language = "en-US";
    profile.languages = {"en-US", "en", "bn"};
    profile.canvas = generateCanvasFingerprint("Browser Fingerprint Test");
    profile.webgl = generateWebGLFingerprint();
    profile.audio = generateAudioFingerprint();
    profile.installedFonts = getInstalledFonts();
    profile.plugins = {"Chrome PDF Plugin", "Chrome PDF Viewer", "Native Client"};
    profile.permissions = {
        {"geolocation", "granted"},
        {"notifications", "default"},
        {"microphone", "prompt"},
        {"camera", "prompt"}
    };
    return profile;
}

BrowserProfile BrowserFingerprinting::generateFirefoxProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::FIREFOX;
    profile.userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:121.0) Gecko/20100101 Firefox/121.0";
    profile.platform = "Win32";
    profile.screenWidth = 1920;
    profile.screenHeight = 1080;
    profile.screenColorDepth = 24;
    profile.devicePixelRatio = 1.0;
    profile.hardwareConcurrency = 8;
    profile.deviceMemory = 0;  // Firefox doesn't expose this
    profile.maxTouchPoints = 0;
    profile.timezone = "Asia/Dhaka";
    profile.timezoneOffset = -360;
    profile.language = "en-US";
    profile.languages = {"en-US", "en"};
    profile.canvas = generateCanvasFingerprint("Browser Fingerprint Test");
    profile.webgl = generateWebGLFingerprint();
    profile.audio = generateAudioFingerprint();
    profile.installedFonts = getInstalledFonts();
    profile.plugins = {};
    profile.permissions = {
        {"geolocation", "prompt"},
        {"notifications", "prompt"}
    };
    return profile;
}

BrowserProfile BrowserFingerprinting::generateSafariProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::SAFARI;
    profile.userAgent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_2) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15";
    profile.platform = "MacIntel";
    profile.screenWidth = 2560;
    profile.screenHeight = 1440;
    profile.screenColorDepth = 24;
    profile.devicePixelRatio = 2.0;
    profile.hardwareConcurrency = 10;
    profile.deviceMemory = 16;
    profile.maxTouchPoints = 0;
    profile.timezone = "Asia/Dhaka";
    profile.timezoneOffset = -360;
    profile.language = "en-US";
    profile.languages = {"en-US", "en"};
    profile.canvas = generateCanvasFingerprint("Browser Fingerprint Test");
    profile.webgl = generateWebGLFingerprint();
    profile.audio = generateAudioFingerprint();
    profile.installedFonts = getInstalledFonts();
    profile.plugins = {};
    profile.permissions = {
        {"geolocation", "prompt"},
        {"notifications", "denied"}
    };
    return profile;
}

BrowserProfile BrowserFingerprinting::generateEdgeProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::EDGE;
    profile.userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0";
    profile.platform = "Win32";
    profile.screenWidth = 1920;
    profile.screenHeight = 1080;
    profile.screenColorDepth = 24;
    profile.devicePixelRatio = 1.0;
    profile.hardwareConcurrency = 8;
    profile.deviceMemory = 8;
    profile.maxTouchPoints = 0;
    profile.timezone = "Asia/Dhaka";
    profile.timezoneOffset = -360;
    profile.language = "en-US";
    profile.languages = {"en-US", "en", "bn"};
    profile.canvas = generateCanvasFingerprint("Browser Fingerprint Test");
    profile.webgl = generateWebGLFingerprint();
    profile.audio = generateAudioFingerprint();
    profile.installedFonts = getInstalledFonts();
    profile.plugins = {"Microsoft Edge PDF Plugin", "Microsoft Edge PDF Viewer"};
    profile.permissions = {
        {"geolocation", "granted"},
        {"notifications", "default"}
    };
    return profile;
}

BrowserProfile BrowserFingerprinting::generateRandomProfile() {
    std::uniform_int_distribution<> dis(0, 3);
    switch (dis(m_gen)) {
        case 0: return generateChromeProfile();
        case 1: return generateFirefoxProfile();
        case 2: return generateSafariProfile();
        case 3: return generateEdgeProfile();
        default: return generateChromeProfile();
    }
}

void BrowserFingerprinting::loadProfile(const BrowserProfile& profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentProfile = profile;
}

BrowserProfile BrowserFingerprinting::getCurrentProfile() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentProfile;
}

CanvasFingerprint BrowserFingerprinting::generateCanvasFingerprint(const std::string& text) {
    CanvasFingerprint fp;
    fp.rawData = simulateCanvasRendering(text);
    
    // Hash the raw data
    fp.hash = hashString(std::string(fp.rawData.begin(), fp.rawData.end()));
    
    // WebGL vendor/renderer
    fp.vendor = "Google Inc. (NVIDIA)";
    fp.renderer = "ANGLE (NVIDIA, NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)";
    
    return fp;
}

WebGLFingerprint BrowserFingerprinting::generateWebGLFingerprint() {
    WebGLFingerprint fp;
    
    fp.vendor = "Google Inc.";
    fp.renderer = "ANGLE (NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)";
    fp.version = "WebGL 1.0 (OpenGL ES 2.0 Chromium)";
    fp.shadingLanguageVersion = "OpenGL ES GLSL ES 1.00 (OpenGL ES 2.0 Chromium)";
    
    fp.extensions = getChromeExtensions();
    
    fp.parameters = {
        {"ALIASED_LINE_WIDTH_RANGE", "[1, 1]"},
        {"ALIASED_POINT_SIZE_RANGE", "[1, 1]"},
        {"ALPHA_BITS", "8"},
        {"ANTIALIASING_LINE_WIDTH_RANGE", "[1, 1]"},
        {"ANTIALIASING_POINT_SIZE_RANGE", "[1, 1]"},
        {"ANTIALIASING_SAMPLES", "0"},
        {"BLUE_BITS", "8"},
        {"DEPTH_BITS", "24"},
        {"GREEN_BITS", "8"},
        {"MAX_COMBINED_TEXTURE_IMAGE_UNITS", "16"},
        {"MAX_CUBE_MAP_TEXTURE_SIZE", "16384"},
        {"MAX_FRAGMENT_UNIFORM_VECTORS", "1024"},
        {"MAX_RENDERBUFFER_SIZE", "16384"},
        {"MAX_TEXTURE_IMAGE_UNITS", "16"},
        {"MAX_TEXTURE_SIZE", "16384"},
        {"MAX_VARYING_VECTORS", "15"},
        {"MAX_VERTEX_ATTRIBS", "16"},
        {"MAX_VERTEX_TEXTURE_IMAGE_UNITS", "16"},
        {"MAX_VERTEX_UNIFORM_VECTORS", "4096"},
        {"MAX_VIEWPORT_DIMS", "[32767, 32767]"},
        {"RED_BITS", "8"},
        {"RENDERER", fp.renderer},
        {"SHADING_LANGUAGE_VERSION", fp.shadingLanguageVersion},
        {"STENCIL_BITS", "0"},
        {"SUBPIXEL_BITS", "8"},
        {"UNPACK_ALIGNMENT", "4"},
        {"VENDOR", fp.vendor},
        {"VERSION", fp.version}
    };
    
    return fp;
}

AudioFingerprint BrowserFingerprinting::generateAudioFingerprint() {
    AudioFingerprint fp;
    
    // Simulate audio processing output (Fourier transform)
    std::vector<float> audioData;
    std::normal_distribution<float> dist(0.0f, 0.1f);
    
    for (int i = 0; i < 1000; i++) {
        float value = dist(m_gen);
        // Add some frequency characteristics
        if (i < 100) value += 0.5f * exp(-i / 20.0f);
        audioData.push_back(value);
    }
    
    fp.output = audioData;
    fp.hash = hashString(std::to_string(audioData.size()));
    
    return fp;
}

std::vector<std::string> BrowserFingerprinting::getInstalledFonts() {
    return {
        "Arial",
        "Arial Black",
        "Calibri",
        "Cambria",
        "Comic Sans MS",
        "Consolas",
        "Courier New",
        "Georgia",
        "Impact",
        "Lucida Console",
        "Palatino Linotype",
        "Segoe UI",
        "Tahoma",
        "Times New Roman",
        "Trebuchet MS",
        "Verdana",
        "Microsoft Sans Serif",
        "Roboto",
        "Open Sans",
        "Helvetica",
        "Noto Sans",
        "DejaVu Sans"
    };
}

std::map<std::string, std::string> BrowserFingerprinting::getNavigatorProperties() {
    return {
        {"appCodeName", "Mozilla"},
        {"appName", "Netscape"},
        {"appVersion", "5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36"},
        {"platform", m_currentProfile.platform},
        {"product", "Gecko"},
        {"productSub", "20030107"},
        {"userAgent", m_currentProfile.userAgent},
        {"vendor", "Google Inc."},
        {"vendorSub", ""},
        {"language", m_currentProfile.language},
        {"languages", m_currentProfile.language},
        {"hardwareConcurrency", std::to_string(m_currentProfile.hardwareConcurrency)},
        {"deviceMemory", std::to_string(m_currentProfile.deviceMemory)},
        {"maxTouchPoints", std::to_string(m_currentProfile.maxTouchPoints)},
        {"doNotTrack", "1"},
        {"cookieEnabled", "true"},
        {"pdfViewerEnabled", "true"}
    };
}

std::map<std::string, int> BrowserFingerprinting::getScreenProperties() {
    return {
        {"width", m_currentProfile.screenWidth},
        {"height", m_currentProfile.screenHeight},
        {"availWidth", m_currentProfile.screenWidth},
        {"availHeight", m_currentProfile.screenHeight - 40},  // Minus taskbar
        {"colorDepth", m_currentProfile.screenColorDepth},
        {"pixelDepth", m_currentProfile.screenColorDepth},
        {"devicePixelRatio", m_currentProfile.devicePixelRatio}
    };
}

std::map<std::string, std::string> BrowserFingerprinting::getTimezoneProperties() {
    return {
        {"timezone", m_currentProfile.timezone},
        {"offset", std::to_string(m_currentProfile.timezoneOffset)}
    };
}

std::map<std::string, std::string> BrowserFingerprinting::getAutomationBypassProperties() {
    return {
        // Remove automation indicators
        {"webdriver", ""},
        {"__webdriver_evaluate", ""},
        {"__selenium_evaluate", ""},
        {"__webglrenderer", ""},
        {"__googling", ""},
        {"__driver_evaluate", ""},
        {"__ghost", ""},
        {"__webdriver_script_function", ""},
        {"__webdriver_script_func", ""},
        {"__webdriver_script_fn", ""},
        {"_phantom", ""},
        {"_selenium", ""},
        {"callPhantom", ""},
        {"callSelenium", ""},
        {"_chromeRuntime", ""},
        {"navigator.webdriver", ""},
        {"navigator.plugins", "Chrome PDF Plugin,Chrome PDF Viewer,Native Client"},
        {"navigator.languages", "en-US,en,bn"},
        {"chrome.runtime", ""},
        {"chrome.loadTimes", ""},
        {"chrome.csi", ""}
    };
}

std::map<std::string, std::variant<int, float, bool>> BrowserFingerprinting::getBatteryStatus() {
    return {
        {"charging", false},
        {"chargingTime", 0},
        {"dischargingTime", 3600},
        {"level", 85}
    };
}

std::map<std::string, std::variant<std::string, int, bool>> BrowserFingerprinting::getConnectionInfo() {
    return {
        {"effectiveType", std::string("4g")},
        {"downlink", 10},
        {"rtt", 50},
        {"saveData", false},
        {" downlinkMax", 100}
    };
}

std::string BrowserFingerprinting::generateUserAgent(BrowserType browser) {
    switch (browser) {
        case BrowserType::CHROME:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
        case BrowserType::FIREFOX:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:121.0) Gecko/20100101 Firefox/121.0";
        case BrowserType::SAFARI:
            return "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_2) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.2 Safari/605.1.15";
        case BrowserType::EDGE:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36 Edg/120.0.0.0";
        default:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Safari/537.36";
    }
}

std::string BrowserFingerprinting::hashString(const std::string& input) {
    // Simple hash for fingerprinting
    uint64_t hash = 0xcbf29ce484222325ULL;
    for (char c : input) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 0x100000001b3ULL;
    }
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 8; i++) {
        ss << std::setw(2) << ((hash >> (56 - i * 8)) & 0xff);
    }
    return ss.str();
}

std::vector<uint8_t> BrowserFingerprinting::simulateCanvasRendering(const std::string& text) {
    std::vector<uint8_t> data;
    
    // Simulate canvas pixel data with realistic variations
    std::normal_distribution<float> dist(0.0f, 1.0f);
    
    // Create a simple "rendered" canvas image
    for (int y = 0; y < 100; y++) {
        for (int x = 0; x < 200; x++) {
            // Simple gradient + noise
            uint8_t r = static_cast<uint8_t>(std::max(0, std::min(255, (x + y) / 2 + static_cast<int>(dist(m_gen) * 10))));
            uint8_t g = static_cast<uint8_t>(std::max(0, std::min(255, (x) / 2 + static_cast<int>(dist(m_gen) * 10))));
            uint8_t b = static_cast<uint8_t>(std::max(0, std::min(255, (y) / 2 + static_cast<int>(dist(m_gen) * 10))));
            uint8_t a = 255;
            
            data.push_back(r);
            data.push_back(g);
            data.push_back(b);
            data.push_back(a);
        }
    }
    
    // Add device-specific variations
    std::uniform_int_distribution<> gpuNoise(0, 3);
    for (size_t i = 0; i < data.size(); i += 4) {
        int noise = gpuNoise(m_gen);
        data[i] = static_cast<uint8_t>(std::min(255, data[i] + noise));
    }
    
    return data;
}

std::vector<std::string> BrowserFingerprinting::getChromeExtensions() {
    return {
        "WEBGL_debug_renderer_info",
        "WEBGL_debug_shaders",
        "WEBGL_lose_context",
        "WEBGL_draw_buffers",
        "OES_texture_float",
        "OES_texture_float_linear",
        "OES_texture_half_float",
        "OES_texture_half_float_linear",
        "OES_standard_derivatives",
        "EXT_texture_filter_anisotropic",
        "EXT_frag_depth",
        "EXT_sRGB",
        "EXT_draw_buffers",
        "EXT_color_buffer_float",
        "EXT_color_buffer_half_float",
        "ANGLE_instanced_arrays",
        "WEBGL_compressed_texture_s3tc",
        "WEBGL_compressed_texture_es3",
        "WEBGL_depth_texture",
        "WEBGL_draw_buffers",
        "WEBGL_multi_draw"
    };
}

std::vector<std::string> BrowserFingerprinting::getFirefoxExtensions() {
    return {
        "EXT_texture_filter_anisotropic",
        "EXT_frag_depth",
        "EXT_sRGB",
        "OES_standard_derivatives",
        "WEBGL_debug_renderer_info",
        "WEBGL_debug_shaders",
        "WEBGL_lose_context"
    };
}

} // namespace VirtualPhonePro
