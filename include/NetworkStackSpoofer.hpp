#pragma once

#include <string>
#include <map>
#include <vector>

namespace AntiDetect {

struct NetworkStackConfig {
    std::string tcpCongestionControl;
    std::string tcpWindowScaling;
    std::string mtuSize;
    std::string tcpTimestamps;
    std::string tcpSACK;
    std::string ipForward;
    std::string tcpFastOpen;
    
    // TTL Settings
    int defaultTTL;
    int wifiTTL;
    int mobileTTL;
    
    // Window Size
    int tcpRmemMin;
    int tcpRmemDefault;
    int tcpRmemMax;
    int tcpWmemMin;
    int tcpWmemDefault;
    int tcpWmemMax;
};

struct NetworkSpoofResult {
    bool success;
    std::string message;
    std::string error;
    std::map<std::string, std::string> details;
};

class NetworkStackSpoofer {
public:
    static NetworkStackSpoofer& getInstance();
    
    NetworkStackSpoofer();
    ~NetworkStackSpoofer();
    
    bool initialize();
    bool isInitialized() const;
    void shutdown();
    
    // TCP/IP Stack Spoofing
    NetworkSpoofResult enableStackSpoofing();
    NetworkSpoofResult disableStackSpoofing();
    
    // TCP Congestion Control
    NetworkSpoofResult setCongestionControl(const std::string& algorithm);
    NetworkSpoofResult setCubicProfile();
    NetworkSpoofResult setBbrProfile();
    NetworkSpoofResult setRenoProfile();
    NetworkSpoofResult setWestwoodProfile();
    
    // TTL Spoofing
    NetworkSpoofResult spoofTTL(int ttl);
    NetworkSpoofResult spoofWifiTTL();
    NetworkSpoofResult spoofMobileTTL();
    NetworkSpoofResult setDeviceTTL();  // Real device TTL (64)
    
    // TCP Window Scaling
    NetworkSpoofResult enableWindowScaling();
    NetworkSpoofResult disableWindowScaling();
    NetworkSpoofResult setWindowScalingFactor(int factor);
    
    // TCP Options
    NetworkSpoofResult enableTimestamps();
    NetworkSpoofResult disableTimestamps();
    NetworkSpoofResult enableSACK();
    NetworkSpoofResult disableSACK();
    NetworkSpoofResult enableTcpFastOpen();
    NetworkSpoofResult disableTcpFastOpen();
    
    // MTU Spoofing
    NetworkSpoofResult setMTU(int mtu);
    NetworkSpoofResult setWifiMTU();
    NetworkSpoofResult setMobileMTU();
    
    // Buffer Sizes
    NetworkSpoofResult setReceiveBuffer(int min, int default_val, int max);
    NetworkSpoofResult setSendBuffer(int min, int default_val, int max);
    NetworkSpoofResult optimizeBuffers();
    
    // DNS Spoofing
    NetworkSpoofResult setCustomDNS(const std::vector<std::string>& dnsServers);
    NetworkSpoofResult setGoogleDNS();
    NetworkSpoofResult setCloudflareDNS();
    NetworkSpoofResult setISPDefaultDNS();
    
    // HTTP Headers Spoofing
    NetworkSpoofResult spoofUserAgent(const std::string& userAgent);
    NetworkSpoofResult spoofAcceptLanguage(const std::string& language);
    NetworkSpoofResult spoofAcceptEncoding(const std::string& encoding);
    NetworkSpoofResult spoofHTTPVersion(const std::string& version);
    NetworkSpoofResult setChromeUserAgent();
    NetworkSpoofResult setFirefoxUserAgent();
    NetworkSpoofResult setSafariUserAgent();
    
    // WebRTC Spoofing
    NetworkSpoofResult spoofWebRTCIP(const std::string& ipAddress);
    NetworkSpoofResult disableWebRTC();
    NetworkSpoofResult enableWebRTCProxyMode();
    
    // Proxy Detection Bypass
    NetworkSpoofResult hideProxySettings();
    NetworkSpoofResult bypassProxyDetection();
    
    // SSL/TLS Spoofing
    NetworkSpoofResult setTLSVersion(int min, int max);
    NetworkSpoofResult enableTLS12();
    NetworkSpoofResult enableTLS13();
    NetworkSpoofResult setCipherSuites(const std::string& ciphers);
    
    // Network Interface Spoofing
    NetworkSpoofResult spoofMACAddress(const std::string& mac);
    NetworkSpoofResult spoofInterfaceName();
    NetworkSpoofResult randomizeMAC();
    NetworkSpoofResult setSamsungMAC();
    NetworkSpoofResult setAppleMAC();
    
    // Mobile Network Spoofing
    NetworkSpoofResult spoofMobileOperator(const std::string& name);
    NetworkSpoofResult spoofMobileCountryCode(int mcc);
    NetworkSpoofResult spoofMobileNetworkCode(int mnc);
    NetworkSpoofResult spoofNetworkType(const std::string& type);  // 5G, 4G, 3G
    
    // Real Device Profiles
    NetworkSpoofResult applySamsungNetworkProfile();
    NetworkSpoofResult applyGoogleNetworkProfile();
    NetworkSpoofResult applyAppleNetworkProfile();
    
    // Validation
    NetworkSpoofResult validateSpoofing();
    bool isSpoofingActive() const;
    NetworkStackConfig getCurrentConfig();
    
    // Status
    std::map<std::string, std::string> getDetailedStatus();
    NetworkSpoofResult getStatus();

private:
    void applyAllChanges();
    void restoreOriginalSettings();
    int generateRandomMACOctet(bool multicast = false);
    
    bool m_initialized;
    bool m_spoofingActive;
    NetworkStackConfig m_currentConfig;
    NetworkStackConfig m_originalConfig;
    
    std::map<std::string, std::string> m_modifiedSettings;
    std::map<std::string, std::string> m_originalSettings;
};

}
