/**
 * NetworkStackSpoofer - Realistic Network Traffic Generation
 * 
 * Makes network traffic appear 100% authentic by emulating
 * real Android device network behavior.
 */

#include "anti_detect/NetworkStackSpoofer.hpp"
#include "core/CryptoUtils.hpp"
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

namespace VirtualPhonePro {
namespace AntiDetect {

NetworkStackSpoofer& NetworkStackSpoofer::getInstance() {
    static NetworkStackSpoofer instance;
    return instance;
}

NetworkStackSpoofer::NetworkStackSpoofer()
    : m_networkType(NetworkType::WIFI)
    , m_manufacturer("Samsung")
    , m_model("Galaxy S23")
    , m_androidVersion("14")
    , m_androidBuild("UP1A.231005.007")
    , m_timingEngine(Crypto::SecureRandomGenerator().generateUint64())
    , m_jitterDist(-20, 20)
    , m_seqEngine(Crypto::SecureRandomGenerator().generateUint64())
    , m_nextSeq(Crypto::SecureRandomGenerator().generateUint32())
    , m_nextIPID(Crypto::SecureRandomGenerator().generateUint16())
{
    initializeNetworkProfiles();
    buildUserAgent(); // Initialize user agent
}

NetworkStackSpoofer::~NetworkStackSpoofer() = default;

void NetworkStackSpoofer::initializeNetworkProfiles() {
    // WiFi Profile
    m_operatorProfiles["wifi"] = {
        NetworkType::WIFI,
        15,     // latency ms
        5,      // jitter ms
        0,      // packet loss %
        50000,  // up kbps
        100000, // down kbps
        "WiFi",
        "",
        ""
    };
    
    // LTE/4G Profile
    m_operatorProfiles["lte_bd"] = {
        NetworkType::LTE_4G,
        45,     // latency ms
        15,     // jitter ms
        0,      // packet loss %
        15000,  // up kbps
        80000,  // down kbps
        "Robi/Airtel",
        "BD",
        "47006" // MCC MNC for Bangladesh
    };
    
    // 5G Profile
    m_operatorProfiles["5g"] = {
        NetworkType::NR_5G,
        20,     // latency ms
        5,      // jitter ms
        0,      // packet loss %
        100000, // up kbps
        500000, // down kbps
        "5G",
        "",
        ""
    };
    
    // Set default profile
    m_chars = m_operatorProfiles["wifi"];
}

bool NetworkStackSpoofer::initialize(NetworkType type) {
    setNetworkProfile(type);
    return true;
}

void NetworkStackSpoofer::setNetworkProfile(NetworkType type) {
    m_networkType = type;
    
    switch (type) {
        case NetworkType::WIFI:
            m_chars = m_operatorProfiles["wifi"];
            break;
        case NetworkType::LTE_4G:
            m_chars = m_operatorProfiles["lte_bd"];
            break;
        case NetworkType::NR_5G:
            m_chars = m_operatorProfiles["5g"];
            break;
        default:
            m_chars = m_operatorProfiles["wifi"];
    }
}

void NetworkStackSpoofer::setDeviceProfile(const std::string& manufacturer,
                                          const std::string& model,
                                          const std::string& androidVersion) {
    m_manufacturer = manufacturer;
    m_model = model;
    m_androidVersion = androidVersion;
    buildUserAgent();
}

std::string NetworkStackSpoofer::buildUserAgent() {
    // Build realistic User-Agent string
    m_httpBehavior.keepAlive = true;
    m_httpBehavior.keepAliveTimeoutSec = 30;
    m_httpBehavior.maxConnections = 6;
    m_httpBehavior.pipelining = false;
    
    if (m_manufacturer == "Samsung") {
        m_httpBehavior.userAgent = 
            "Mozilla/5.0 (Linux; Android " + m_androidVersion + "; " + m_model + ") " +
            "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36 " +
            "(Samsung Browser/26)";
        m_httpBehavior.acceptLanguage = "en-US,en;q=0.9";
        m_httpBehavior.acceptEncoding = "gzip, deflate, br";
    } else if (m_manufacturer == "Google") {
        m_httpBehavior.userAgent = 
            "Mozilla/5.0 (Linux; Android " + m_androidVersion + "; " + m_model + ") " +
            "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36";
        m_httpBehavior.acceptLanguage = "en-US,en;q=0.9";
        m_httpBehavior.acceptEncoding = "gzip, deflate, br";
    } else if (m_manufacturer == "Xiaomi") {
        m_httpBehavior.userAgent = 
            "Mozilla/5.0 (Linux; Android " + m_androidVersion + "; " + m_model + ") " +
            "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36 " +
            "MIUIBrowser/14";
        m_httpBehavior.acceptLanguage = "en-US,en;q=0.9";
        m_httpBehavior.acceptEncoding = "gzip, deflate";
    } else {
        m_httpBehavior.userAgent = 
            "Mozilla/5.0 (Linux; Android " + m_androidVersion + "; " + m_model + ") " +
            "AppleWebKit/537.36 (KHTML, like Gecko) Chrome/120.0.0.0 Mobile Safari/537.36";
        m_httpBehavior.acceptLanguage = "en-US,en;q=0.9";
        m_httpBehavior.acceptEncoding = "gzip, deflate, br";
    }
    
    // Bangladesh-specific headers
    m_countryHeaders["BD"] = {
        {"X-Forwarded-For", "103.x.x.x"},
        {"CF-IPCountry", "BD"},
        {"X-Real-IP", "103.x.x.x"}
    };
    
    return m_httpBehavior.userAgent;
}

int NetworkStackSpoofer::getEmulatedLatency() {
    int base = m_chars.latencyMs;
    int jitter = m_jitterDist(m_timingEngine);
    return std::max(1, base + jitter);
}

std::chrono::milliseconds NetworkStackSpoofer::getPacketDelay(size_t packetSize) {
    auto delay = calculateBandwidthDelay(packetSize, m_chars.bandwidthDownKbps);
    auto latency = std::chrono::milliseconds(getEmulatedLatency());
    return delay + latency;
}

std::chrono::microseconds NetworkStackSpoofer::calculateBandwidthDelay(size_t bytes, int kbps) {
    if (kbps == 0) return std::chrono::microseconds(0);
    int64_t delayUs = (bytes * 8 * 1000) / kbps;
    return std::chrono::microseconds(delayUs);
}

void NetworkStackSpoofer::applyThrottling(size_t bytesToSend) {
    // Simulate realistic throttling
    auto delay = calculateBandwidthDelay(bytesToSend, m_chars.bandwidthUpKbps);
    usleep(delay.count());
}

std::vector<std::string> NetworkStackSpoofer::generateDNSQueries() {
    // Return common DNS queries for realistic behavior
    return {
        "clients3.google.com",
        "www.google.com",
        "dns.google",
        "mtalk.google.com",
        "www.googleapis.com",
        "play.google.com",
        "accounts.google.com"
    };
}

uint32_t NetworkStackSpoofer::getDNSCacheTTL(const std::string& domain) {
    // DNS TTL varies by domain type
    if (domain.find("google") != std::string::npos) {
        return 300; // 5 minutes
    }
    if (domain.find("facebook") != std::string::npos) {
        return 600; // 10 minutes
    }
    if (domain.find("bank") != std::string::npos) {
        return 60; // 1 minute
    }
    return 300; // Default 5 minutes
}

std::map<std::string, std::string> NetworkStackSpoofer::generateHTTPHeaders(
    const std::string& method,
    const std::string& host,
    const std::string& path) {
    
    std::map<std::string, std::string> headers;
    
    // Standard headers
    headers["Host"] = host;
    headers["Connection"] = m_httpBehavior.keepAlive ? "keep-alive" : "close";
    headers["User-Agent"] = m_httpBehavior.userAgent;
    headers["Accept"] = "text/html,application/xhtml+xml,application/xml;q=0.9,*/*;q=0.8";
    headers["Accept-Language"] = m_httpBehavior.acceptLanguage;
    headers["Accept-Encoding"] = m_httpBehavior.acceptEncoding;
    headers["DNT"] = "1";
    headers["Upgrade-Insecure-Requests"] = "1";
    
    // Mobile-specific headers
    headers["X-Requested-With"] = "com.android.chrome";
    headers["X-Device-Type"] = "mobile";
    
    // Content-type for POST
    if (method == "POST") {
        headers["Content-Type"] = "application/x-www-form-urlencoded; charset=UTF-8";
    }
    
    // Request specific
    if (!path.empty()) {
        headers["Referer"] = "https://" + host + "/";
    }
    
    // Cookies header (if any)
    headers["Cookie"] = "";
    
    // Cache control
    headers["Cache-Control"] = "no-cache";
    headers["Pragma"] = "no-cache";
    
    return headers;
}

std::string NetworkStackSpoofer::generateUserAgent(const std::string& appName) {
    if (appName.empty()) {
        return m_httpBehavior.userAgent;
    }
    
    // App-specific user agent
    if (appName == "facebook") {
        return "FBAV/420.0.0.38.107 Android/" + m_androidVersion + 
               " (Linux; U; Android " + m_androidVersion + "; " + m_model + 
               "; " + m_androidBuild + " Build/RP1A.200720.011) [FBAN/EMA;FBAV/420.0.0.38.107;FBPN/com.facebook.katana;FBLC/en_US;FBBV/4200;FBCR/null;FBMF/" + 
               m_manufacturer + ";FBBD/" + m_manufacturer + ";FBDV/" + m_model + 
               ";FBSV/" + m_androidVersion + ";FBSDK/420.0.0;FBAV/420.0.0.38.107;FBOP/1;FBCA/arm64-v8a:en-US;]";
    }
    
    return m_httpBehavior.userAgent;
}

std::chrono::seconds NetworkStackSpoofer::getConnectionTimeout() {
    switch (m_networkType) {
        case NetworkType::LTE_4G:
            return std::chrono::seconds(30);
        case NetworkType::NR_5G:
            return std::chrono::seconds(15);
        default:
            return std::chrono::seconds(20);
    }
}

bool NetworkStackSpoofer::shouldKeepAlive(const std::string& host) {
    // Google services always keep alive
    if (host.find("google") != std::string::npos) {
        return true;
    }
    // Banking apps typically close connection
    if (host.find("bank") != std::string::npos) {
        return false;
    }
    return m_httpBehavior.keepAlive;
}

uint16_t NetworkStackSpoofer::getTCPWindowSize() {
    // Realistic TCP window sizes per network type
    switch (m_networkType) {
        case NetworkType::WIFI:
            return 29200;
        case NetworkType::LTE_4G:
            return 14600;
        case NetworkType::NR_5G:
            return 65535;
        default:
            return 14600;
    }
}

uint16_t NetworkStackSpoofer::getMSS() {
    // MTU - IP headers - TCP headers
    return getMTU() - 40; // 1460 for standard
}

uint16_t NetworkStackSpoofer::getMTU() {
    switch (m_networkType) {
        case NetworkType::LTE_4G:
            return 1500; // Standard
        case NetworkType::NR_5G:
            return 1500;
        default:
            return 1500;
    }
}

std::vector<uint8_t> NetworkStackSpoofer::getTCPOptions() {
    // TCP options in wire format
    // MSS (2), SACK permitted (1), Timestamp (8), Window scale (3)
    return {
        0x02, 0x04, 0x05, 0xb4,  // MSS: 1460
        0x01,                     // No-op
        0x04, 0x02,              // SACK permitted
        0x01,                    // No-op
        0x01,                    // No-op
        0x08, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Timestamp
        0x03, 0x03, 0x07          // Window scale: 7 (2^7 = 128)
    };
}

bool NetworkStackSpoofer::supportsQUIC() {
    return m_networkType == NetworkType::NR_5G || 
           m_networkType == NetworkType::LTE_4G;
}

std::vector<std::string> NetworkStackSpoofer::getQUICVersions() {
    if (m_networkType == NetworkType::NR_5G) {
        return {"h3-29", "h3-28", "h3-27"};
    }
    return {"h3-28", "h3-27", "h3-25"};
}

std::map<std::string, std::string> NetworkStackSpoofer::getHTTP2Settings() {
    return {
        {"HEADER_TABLE_SIZE", "65536"},
        {"SETTINGS_MAX_CONCURRENT_STREAMS", "1000"},
        {"MAX_FRAME_SIZE", "16384"},
        {"INITIAL_WINDOW_SIZE", "6291456"},
        {"MAX_HEADER_LIST_SIZE", "262144"}
    };
}

uint16_t NetworkStackSpoofer::generateIPID() {
    m_nextIPID++;
    return m_nextIPID;
}

uint32_t NetworkStackSpoofer::generateTCPSeq() {
    m_nextSeq += Crypto::SecureRandomGenerator().generateUint32() % 1000 + 1000;
    return m_nextSeq;
}

uint32_t NetworkStackSpoofer::generateTCPACK() {
    return m_nextSeq;
}

uint8_t NetworkStackSpoofer::getIPTOS() {
    // Type of Service / DSCP
    // Interactive traffic: 0x10 (IPTOS_LOWDELAY)
    // Throughput: 0x08
    return 0x10; // Low delay
}

uint8_t NetworkStackSpoofer::getIPTTL() {
    switch (m_networkType) {
        case NetworkType::LTE_4G:
            return 64;
        case NetworkType::NR_5G:
            return 64;
        default:
            return 64;
    }
}

NetworkCharacteristics NetworkStackSpoofer::getNetworkCharacteristics() {
    return m_chars;
}

bool NetworkStackSpoofer::validateNetworkFingerprint() {
    // Check if network characteristics are realistic
    if (m_chars.latencyMs < 1 || m_chars.latencyMs > 500) {
        return false;
    }
    if (m_chars.packetLossPercent < 0 || m_chars.packetLossPercent > 5) {
        return false;
    }
    return true;
}

} // namespace AntiDetect
} // namespace VirtualPhonePro
