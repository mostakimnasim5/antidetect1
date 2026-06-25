/**
 * BrowserFingerprinting - Enterprise-Grade Browser Fingerprint Implementation
 */

#include "browser/BrowserFingerprinting.hpp"
#include "core/CryptoUtils.hpp"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <numeric>

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
    profile.userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36";
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
    
    // Unique identifiers
    profile.clientId = generateClientId();
    profile.sessionId = generateSessionId();
    profile.generatedTime = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    // Hardware info
    profile.gpuVendor = "Google Inc. (NVIDIA)";
    profile.gpuRenderer = "ANGLE (NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)";
    profile.cpuModel = "Intel(R) Core(TM) i7-10700K CPU @ 3.80GHz";
    
    return profile;
}

BrowserProfile BrowserFingerprinting::generateFirefoxProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::FIREFOX;
    profile.userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:126.0) Gecko/20100101 Firefox/126.0";
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
    
    profile.clientId = generateClientId();
    profile.sessionId = generateSessionId();
    profile.generatedTime = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    profile.gpuVendor = "Mozilla";
    profile.gpuRenderer = "llvmpipe (LLVM 16.0.6, 256 bits)";
    profile.cpuModel = "Intel(R) Core(TM) i7-10700K CPU @ 3.80GHz";
    
    return profile;
}

BrowserProfile BrowserFingerprinting::generateSafariProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::SAFARI;
    profile.userAgent = "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_5) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.5 Safari/605.1.15";
    profile.platform = "MacIntel";
    profile.screenWidth = 2880;
    profile.screenHeight = 1800;
    profile.screenColorDepth = 30;
    profile.devicePixelRatio = 2.0;
    profile.hardwareConcurrency = 10;
    profile.deviceMemory = 16;
    profile.maxTouchPoints = 0;
    profile.timezone = "America/New_York";
    profile.timezoneOffset = -240;
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
    
    profile.clientId = generateClientId();
    profile.sessionId = generateSessionId();
    profile.generatedTime = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    profile.gpuVendor = "Apple Inc.";
    profile.gpuRenderer = "Apple M1 Pro";
    profile.cpuModel = "Apple M1 Pro";
    
    return profile;
}

BrowserProfile BrowserFingerprinting::generateEdgeProfile() {
    BrowserProfile profile;
    profile.browser = BrowserType::EDGE;
    profile.userAgent = "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 Edg/125.0.0.0";
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
    
    profile.clientId = generateClientId();
    profile.sessionId = generateSessionId();
    profile.generatedTime = std::to_string(
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count()
    );
    
    profile.gpuVendor = "Google Inc. (AMD)";
    profile.gpuRenderer = "ANGLE (AMD Radeon RX 6800 XT Direct3D11 vs_5_0 ps_5_0)";
    profile.cpuModel = "AMD Ryzen 9 5900X 12-Core Processor";
    
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
    fp.width = 280;
    fp.height = 60;
    
    // Generate GPU-specific canvas data
    std::string gpuVendor = "Google Inc. (NVIDIA)";
    std::string gpuRenderer = "ANGLE (NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)";
    
    fp.rawData = simulateGPUCanvasRendering(text, gpuVendor, gpuRenderer);
    fp.hash = hashBytes(fp.rawData.data(), fp.rawData.size());
    fp.vendor = gpuVendor;
    fp.renderer = gpuRenderer;
    fp.gpuMaskedVendor = "Google Inc.";
    fp.gpuMaskedRenderer = "ANGLE (NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)";
    
    return fp;
}

std::vector<uint8_t> BrowserFingerprinting::simulateGPUCanvasRendering(
    const std::string& text,
    const std::string& gpuVendor,
    const std::string& gpuRenderer) {
    
    std::vector<uint8_t> data;
    
    // Canvas dimensions
    int width = 280;
    int height = 60;
    
    // Generate pixel data with GPU-specific variations
    std::normal_distribution<float> dist(0.0f, 1.0f);
    
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Base gradient
            float r = static_cast<float>(x + y) / 2.0f;
            float g = static_cast<float>(x) / 2.0f;
            float b = static_cast<float>(y) / 2.0f;
            
            // Text region (roughly center)
            if (y > 15 && y < 45 && x > 20 && x < 260) {
                float textInfluence = std::sin(x * 0.1f) * std::cos(y * 0.1f);
                r += textInfluence * 50;
                g += textInfluence * 80;
                b += textInfluence * 40;
            }
            
            // GPU-specific noise
            if (gpuVendor.find("NVIDIA") != std::string::npos) {
                r += std::sin(x * 0.05f + y * 0.03f) * 3.0f;
            } else if (gpuVendor.find("AMD") != std::string::npos || gpuVendor.find("Radeon") != std::string::npos) {
                r += std::cos(x * 0.04f + y * 0.06f) * 3.0f;
            } else if (gpuVendor.find("Intel") != std::string::npos) {
                r += std::sin(x * 0.03f - y * 0.02f) * 2.0f;
            } else if (gpuVendor.find("Apple") != std::string::npos) {
                r += std::cos(x * 0.06f + y * 0.04f) * 2.5f;
            }
            
            // Anti-aliasing noise
            r += dist(m_gen) * 1.5f;
            g += dist(m_gen) * 1.5f;
            b += dist(m_gen) * 1.5f;
            
            // Clamp values
            data.push_back(static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, r))));
            data.push_back(static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, g))));
            data.push_back(static_cast<uint8_t>(std::max(0.0f, std::min(255.0f, b))));
            data.push_back(255); // Alpha
        }
    }
    
    // Add final checksum
    uint8_t checksum = 0;
    for (size_t i = 0; i < data.size(); i++) {
        checksum ^= data[i];
        checksum = (checksum << 1) | (checksum >> 7);
    }
    data.push_back(checksum);
    
    return data;
}

WebGLFingerprint BrowserFingerprinting::generateWebGLFingerprint() {
    WebGLFingerprint fp;
    
    fp.vendor = "Google Inc.";
    fp.renderer = "ANGLE (NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)";
    fp.version = "WebGL 1.0 (OpenGL ES 2.0 Chromium)";
    fp.shadingLanguageVersion = "OpenGL ES GLSL ES 1.00 (OpenGL ES 2.0 Chromium)";
    fp.version2 = "WebGL 2.0 (OpenGL ES 3.0 Chromium)";
    fp.shadingLanguageVersion2 = "OpenGL ES GLSL ES 3.00 (OpenGL ES 3.0 Chromium)";
    fp.unmaskedVendor = "Google Inc. (NVIDIA)";
    fp.unmaskedRenderer = "NVIDIA GeForce GTX 1060/PCIe/SSE2";
    
    fp.extensions = getChromeExtensions();
    fp.extensions2 = getChromeExtensions2();
    
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
    
    fp.parameters2 = {
        {"ALIASED_LINE_WIDTH_RANGE", "[1, 256]"},
        {"ALIASED_POINT_SIZE_RANGE", "[1, 1024]"},
        {"ALPHA_BITS", "8"},
        {"BLUE_BITS", "8"},
        {"DEPTH_BITS", "24"},
        {"GREEN_BITS", "8"},
        {"MAX_3D_TEXTURE_SIZE", "2048"},
        {"MAX_ARRAY_TEXTURE_LAYERS", "4096"},
        {"MAX_COLOR_ATTACHMENTS", "8"},
        {"MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS", "239647"},
        {"MAX_COMBINED_UNIFORM_BLOCKS", "23"},
        {"MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS", "4184447"},
        {"MAX_CUBE_MAP_TEXTURE_SIZE", "16384"},
        {"MAX_DRAW_BUFFERS", "8"},
        {"MAX_FRAGMENT_INPUT_COMPONENTS", "128"},
        {"MAX_FRAGMENT_UNIFORM_BLOCKS", "14"},
        {"MAX_FRAGMENT_UNIFORM_COMPONENTS", "35791394"},
        {"MAX_RENDERBUFFER_SIZE", "16384"},
        {"MAX_SAMPLES", "16"},
        {"MAX_TEXTURE_SIZE", "16384"},
        {"MAX_UNIFORM_BLOCK_SIZE", "1048576"},
        {"MAX_VERTEX_OUTPUT_COMPONENTS", "128"},
        {"MAX_VIEWPORT_DIMS", "[32767, 32767]"},
        {"RED_BITS", "8"},
        {"RENDERER", fp.renderer},
        {"SHADING_LANGUAGE_VERSION", fp.shadingLanguageVersion2},
        {"STENCIL_BITS", "0"},
        {"SUBPIXEL_BITS", "8"},
        {"VENDOR", fp.vendor},
        {"VERSION", fp.version2}
    };
    
    fp.maxTextureSize = 16384;
    fp.maxCubeMapTextureSize = 16384;
    fp.maxRenderbufferSize = 16384;
    
    return fp;
}

WebGLFingerprint BrowserFingerprinting::generateWebGL2Fingerprint() {
    return generateWebGLFingerprint();
}

AudioFingerprint BrowserFingerprinting::generateAudioFingerprint() {
    AudioFingerprint fp;
    
    fp.sampleRate = 44100.0;
    fp.frequencyData = processAudioSignal(44100, 5);
    fp.hash = Crypto::SHA256Hasher::hashHex(std::to_string(fp.frequencyData.size()));
    
    auto data48k = processAudioSignal(48000, 5);
    fp.hash48kHz = Crypto::SHA256Hasher::hashHex(std::to_string(data48k.size()));
    
    auto data96k = processAudioSignal(96000, 5);
    fp.hash96kHz = Crypto::SHA256Hasher::hashHex(std::to_string(data96k.size()));
    
    fp.waveformData.resize(100);
    std::normal_distribution<float> dist(0.0f, 0.1f);
    for (size_t i = 0; i < fp.waveformData.size(); i++) {
        fp.waveformData[i] = dist(m_gen);
    }
    
    return fp;
}

AudioFingerprint BrowserFingerprinting::generateAudioFingerprint48kHz() {
    AudioFingerprint fp;
    fp.sampleRate = 48000.0;
    fp.frequencyData = processAudioSignal(48000, 5);
    fp.hash = Crypto::SHA256Hasher::hashHex(std::to_string(fp.frequencyData.size()));
    return fp;
}

AudioFingerprint BrowserFingerprinting::generateAudioFingerprint96kHz() {
    AudioFingerprint fp;
    fp.sampleRate = 96000.0;
    fp.frequencyData = processAudioSignal(96000, 5);
    fp.hash = Crypto::SHA256Hasher::hashHex(std::to_string(fp.frequencyData.size()));
    return fp;
}

std::vector<double> BrowserFingerprinting::processAudioSignal(int sampleRate, int duration) {
    std::vector<double> samples(sampleRate * duration);
    
    const double baseFreq = 440.0;
    
    for (size_t i = 0; i < samples.size(); i++) {
        double t = static_cast<double>(i) / sampleRate;
        
        double osc1 = std::sin(2.0 * M_PI * baseFreq * t);
        double osc2 = 0.5 * std::sin(4.0 * M_PI * baseFreq * t);
        double osc3 = 0.25 * std::sin(6.0 * M_PI * baseFreq * t);
        
        double sample = osc1 + osc2 + osc3;
        
        std::normal_distribution<double> noise(0, 0.001);
        sample += noise(m_gen);
        
        sample = std::tanh(sample * 0.8);
        
        samples[i] = sample;
    }
    
    return samples;
}

double BrowserFingerprinting::computeAudioFingerprint(const std::vector<double>& samples) {
    if (samples.empty()) return 0.0;
    
    double sum = 0.0;
    for (size_t i = 0; i < samples.size() && i < 10000; i++) {
        sum += std::abs(samples[i]);
    }
    
    return sum / std::min(samples.size(), static_cast<size_t>(10000));
}

std::vector<std::string> BrowserFingerprinting::getInstalledFonts() {
    return {
        "Arial", "Arial Black", "Calibri", "Cambria", "Comic Sans MS", "Consolas",
        "Courier New", "Georgia", "Impact", "Lucida Console", "Palatino Linotype",
        "Segoe UI", "Tahoma", "Times New Roman", "Trebuchet MS", "Verdana",
        "Microsoft Sans Serif", "Roboto", "Open Sans", "Helvetica", "Noto Sans",
        "DejaVu Sans", "Ubuntu", "Droid Sans", "Lato", "Source Sans Pro"
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
        {"availHeight", m_currentProfile.screenHeight - 40},
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
    std::uniform_int_distribution<> chargeDist(50, 95);
    return {
        {"charging", false},
        {"chargingTime", 0},
        {"dischargingTime", 3600},
        {"level", chargeDist(m_gen)}
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

std::string BrowserFingerprinting::generateClientId() {
    Crypto::SecureRandomGenerator rng;
    return rng.generateHexString(64);
}

std::string BrowserFingerprinting::generateSessionId() {
    return Crypto::SHA256Hasher::hashHex(
        std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count())
    );
}

std::string BrowserFingerprinting::generateUserAgent(BrowserType browser) {
    switch (browser) {
        case BrowserType::CHROME:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36";
        case BrowserType::FIREFOX:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:126.0) Gecko/20100101 Firefox/126.0";
        case BrowserType::SAFARI:
            return "Mozilla/5.0 (Macintosh; Intel Mac OS X 14_5) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/17.5 Safari/605.1.15";
        case BrowserType::EDGE:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36 Edg/125.0.0.0";
        default:
            return "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/125.0.0.0 Safari/537.36";
    }
}

std::string BrowserFingerprinting::hashString(const std::string& input) {
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

std::string BrowserFingerprinting::hashBytes(const uint8_t* data, size_t length) {
    std::string input(reinterpret_cast<const char*>(data), length);
    return hashString(input);
}

std::vector<uint8_t> BrowserFingerprinting::simulateCanvasRendering(const std::string& text) {
    return simulateGPUCanvasRendering(text, "Google Inc. (NVIDIA)", 
        "ANGLE (NVIDIA GeForce GTX 1060 Direct3D11 vs_5_0 ps_5_0)");
}

std::vector<std::string> BrowserFingerprinting::getChromeExtensions() {
    return {
        "WEBGL_debug_renderer_info", "WEBGL_debug_shaders", "WEBGL_lose_context",
        "WEBGL_draw_buffers", "OES_texture_float", "OES_texture_float_linear",
        "OES_texture_half_float", "OES_texture_half_float_linear",
        "OES_standard_derivatives", "EXT_texture_filter_anisotropic",
        "EXT_frag_depth", "EXT_sRGB", "EXT_draw_buffers",
        "EXT_color_buffer_float", "EXT_color_buffer_half_float",
        "ANGLE_instanced_arrays", "WEBGL_compressed_texture_s3tc",
        "WEBGL_compressed_texture_es3", "WEBGL_depth_texture", "WEBGL_multi_draw"
    };
}

std::vector<std::string> BrowserFingerprinting::getFirefoxExtensions() {
    return {
        "EXT_texture_filter_anisotropic", "EXT_frag_depth", "EXT_sRGB",
        "OES_standard_derivatives", "WEBGL_debug_renderer_info",
        "WEBGL_debug_shaders", "WEBGL_lose_context"
    };
}

std::vector<std::string> BrowserFingerprinting::getChromeExtensions2() {
    return {
        "EXT_color_buffer_float", "EXT_depth_rendering", "EXT_disjoint_timer_query",
        "EXT_float_blend", "EXT_texture_compression_bptc", "EXT_texture_compression_rgtc",
        "EXT_texture_filter_anisotropic", "EXT_texture_norm16",
        "EXT_disjoint_timer_query_webgl2", "KHR_parallel_shader_compile",
        "OES_texture_float_linear", "WEBGL_compressed_texture_etc",
        "WEBGL_compressed_texture_s3tc", "WEBGL_compressed_texture_astc",
        "WEBGL_debug_renderer_info", "WEBGL_debug_shaders",
        "WEBGL_draw_buffers", "WEBGL_lose_context"
    };
}

uint32_t BrowserFingerprinting::crc32(const uint8_t* data, size_t length) {
    static uint32_t table[256] = {0};
    static bool initialized = false;
    
    if (!initialized) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t crc = i;
            for (int j = 0; j < 8; j++) {
                crc = (crc >> 1) ^ (crc & 1 ? 0xEDB88320 : 0);
            }
            table[i] = crc;
        }
        initialized = true;
    }
    
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc ^ 0xFFFFFFFF;
}

} // namespace VirtualPhonePro
