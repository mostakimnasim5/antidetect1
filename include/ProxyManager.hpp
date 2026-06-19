#pragma once

#include <string>
#include <vector>
#include <map>

namespace AntiDetect {

enum class ProxyType {
    HTTP,
    HTTPS,
    SOCKS4,
    SOCKS5
};

struct ProxyCredentials {
    std::string username;
    std::string password;
    bool useAuth;
    
    ProxyCredentials() : useAuth(false) {}
};

struct NetworkProxyInfo {
    ProxyType type;
    std::string host;
    int port;
    ProxyCredentials credentials;
    
    bool enabled;
    std::string profileId;
    
    NetworkProxyInfo() 
        : type(ProxyType::HTTP)
        , port(8080)
        , enabled(false)
    {}
    
    std::string getProxyString() const;
    std::string getFormattedProxy() const;
};

struct ProxyPool {
    std::string poolName;
    std::vector<NetworkProxyInfo> proxies;
    int currentIndex;
    bool rotateAutomatically;
    
    ProxyPool() : currentIndex(0), rotateAutomatically(false) {}
};

class ProxyManager {
public:
    static ProxyManager& getInstance();
    
    bool initialize();
    bool shutdown();
    
    // Proxy Operations
    bool setProxy(const NetworkProxyInfo& proxy);
    bool removeProxy();
    NetworkProxyInfo getCurrentProxy() const;
    bool isProxyEnabled() const;
    
    // Proxy Validation
    bool validateProxy(const NetworkProxyInfo& proxy);
    bool testProxy(const NetworkProxyInfo& proxy);
    std::string getProxyIP() const;
    
    // Profile Integration
    bool applyProxyToProfile(const std::string& profileId, const NetworkProxyInfo& proxy);
    bool removeProxyFromProfile(const std::string& profileId);
    NetworkProxyInfo getProfileProxy(const std::string& profileId) const;
    
    // Proxy Pool
    bool addToPool(const std::string& poolName, const NetworkProxyInfo& proxy);
    bool removeFromPool(const std::string& poolName, const std::string& host, int port);
    NetworkProxyInfo getNextFromPool(const std::string& poolName);
    bool rotatePool(const std::string& poolName);
    
    // Batch Import
    bool importFromFile(const std::string& filepath);
    bool importFromText(const std::string& text);
    std::vector<NetworkProxyInfo> parseProxyList(const std::string& proxyString);
    
    // Real Device IP (no proxy)
    bool useRealDeviceIP();
    std::string getDeviceRealIP() const;
    std::string getDeviceRealMAC() const;
    
    // DNS Configuration
    bool setDNS(const std::string& dnsServer);
    bool setGoogleDNS();
    bool setCloudflareDNS();
    bool resetDNS();
    
    // WiFi Proxy
    bool setWiFiProxy(const NetworkProxyInfo& proxy);
    bool removeWiFiProxy();
    
private:
    ProxyManager();
    ~ProxyManager();
    
    ProxyManager(const ProxyManager&) = delete;
    ProxyManager& operator=(const ProxyManager&) = delete;
    
    bool applyProxySettings(const NetworkProxyInfo& proxy);
    bool applyGlobalProxy(const NetworkProxyInfo& proxy);
    bool applyWiFiProxy(const NetworkProxyInfo& proxy, const std::string& wifiSSID);
    
    std::string proxyTypeToString(ProxyType type) const;
    ProxyType stringToProxyType(const std::string& type) const;
    
    NetworkProxyInfo m_currentProxy;
    std::map<std::string, NetworkProxyInfo> m_profileProxies;
    std::map<std::string, ProxyPool> m_proxyPools;
    bool m_initialized;
};

}
