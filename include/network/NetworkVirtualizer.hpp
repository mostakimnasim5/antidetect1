#pragma once

/**
 * NetworkVirtualizer - Complete Network Virtualization Layer
 * 
 * Provides unique network identity for each VM instance.
 */

#include "../VirtualPhonePro.hpp"

namespace VirtualPhonePro {

// Network Interface
struct NetworkInterface {
    std::string name;
    std::string macAddress;
    std::string ipAddress;
    std::string subnet;
    std::string gateway;
    std::string dns1;
    std::string dns2;
    std::string networkMode;
    bool isUp;
    int speed; // Mbps
};

// TCP/IP Stack Fingerprint
struct TCPStackFingerprint {
    uint8_t windowSize;
    uint16_t maxSegmentSize;
    uint8_t ttl;
    uint8_t windowScaling;
    uint8_t sackOK;
    uint8_t nop;
    uint8_t timestamp;
    std::vector<uint8_t> options;
    
    // OS-specific signatures
    std::string osFamily; // Windows, Linux, Android
    std::string osVersion;
};

class NetworkVirtualizer {
public:
    static NetworkVirtualizer& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // Network Interface Management
    std::string createNetworkInterface(const std::string& vmId, NetworkMode mode);
    bool deleteNetworkInterface(const std::string& interfaceName);
    bool configureInterface(const std::string& interfaceName, const NetworkInterface& config);
    
    // MAC Address
    std::string generateUniqueMAC(const std::string& manufacturer = "");
    std::string getRandomMAC();
    std::string getSamsungMAC();
    std::string getAppleMAC();
    std::string getGoogleMAC();
    std::string getXiaomiMAC();
    
    // IP Address
    std::string generateIPAddress(const std::string& subnet);
    std::string getNextAvailableIP(const std::string& subnet);
    
    // DNS Configuration
    bool setDNS(const std::string& interfaceName, const std::string& dns1, const std::string& dns2);
    bool addDNSSpoof(const std::string& domain, const std::string& ip);
    bool clearDNSCache(const std::string& vmId);
    
    // VPN Support
    std::string connectVPN(const std::string& vmId, const std::string& server, int port);
    bool disconnectVPN(const std::string& vmId);
    bool isVPNConnected(const std::string& vmId);
    
    // TCP/IP Stack Fingerprint
    TCPStackFingerprint getTCPStackFingerprint(const std::string& osFamily);
    bool setTCPStackFingerprint(const std::string& vmId, const TCPStackFingerprint& fingerprint);
    
    // Network Isolation
    bool isolateVM(const std::string& vmId);
    bool allowInterVMCommunication(const std::string& vmId1, const std::string& vmId2);
    bool setupFirewallRules(const std::string& vmId);
    
    // Monitoring
    NetworkInterface getInterfaceInfo(const std::string& interfaceName);
    std::map<std::string, NetworkInterface> getAllInterfaces();
    uint64_t getBytesSent(const std::string& vmId);
    uint64_t getBytesReceived(const std::string& vmId);
    
private:
    NetworkVirtualizer();
    ~NetworkVirtualizer();
    NetworkVirtualizer(const NetworkVirtualizer&) = delete;
    NetworkVirtualizer& operator=(const NetworkVirtualizer&) = delete;
    
    std::string generateRandomHex(int length);
    std::string calculateBroadcast(const std::string& ip, const std::string& subnet);
    
    std::map<std::string, NetworkInterface> m_interfaces;
    std::map<std::string, TCPStackFingerprint> m_tcpFingerprints;
    std::mutex m_mutex;
};

// MAC Address Generator
class MACAddressGenerator {
public:
    MACAddressGenerator();
    
    // Generate random MAC
    std::string generateRandom();
    
    // Manufacturer-specific MACs
    std::string generateSamsung();
    std::string generateGoogle();
    std::string generateApple();
    std::string generateXiaomi();
    std::string generateOnePlus();
    std::string generateOppo();
    std::string generateVivo();
    std::string generateSony();
    std::string generateLG();
    std::string generateHuawei();
    
    // From OUI database
    std::string fromOUI(const std::string& oui);
    
    // Generate with prefix
    std::string generateWithPrefix(const std::string& prefix);
    
private:
    // OUI Database
    static const std::map<std::string, std::string> s_ouiDatabase;
    
    std::random_device m_random;
    std::mt19937 m_generator;
    std::uniform_int_distribution<int> m_byteDist;
};

// DNS Resolver
class DNSResolver {
public:
    DNSResolver();
    
    // DNS Resolution
    std::string resolve(const std::string& hostname);
    std::vector<std::string> resolveAll(const std::string& hostname);
    
    // DNS Spoofing
    bool addSpoofEntry(const std::string& hostname, const std::string& ip);
    bool removeSpoofEntry(const std::string& hostname);
    bool clearSpoofEntries();
    
    // DNS Server Configuration
    bool setDNSServer(const std::string& server);
    std::vector<std::string> getDNSServers();
    
    // Preset DNS
    void useGoogleDNS();
    void useCloudflareDNS();
    void useQuad9DNS();
    void useOpenDNS();
    
private:
    std::string m_dnsServer1;
    std::string m_dnsServer2;
    std::map<std::string, std::string> m_spoofEntries;
    std::mutex m_mutex;
};

// TCP Fingerprint Emulator
class TCPFingerprintEmulator {
public:
    TCPFingerprintEmulator();
    
    // Windows fingerprints
    TCPStackFingerprint getWindows10();
    TCPStackFingerprint getWindows11();
    
    // Linux fingerprints
    TCPStackFingerprint getLinuxKernel5();
    TCPStackFingerprint getLinuxKernel6();
    
    // Android fingerprints
    TCPStackFingerprint getAndroid11();
    TCPStackFingerprint getAndroid12();
    TCPStackFingerprint getAndroid13();
    TCPStackFingerprint getAndroid14();
    
    // Random fingerprint
    TCPStackFingerprint getRandom();
    
    // Custom fingerprint
    void setCustomFingerprint(const TCPStackFingerprint& fp);
    
private:
    TCPStackFingerprint createFingerprint(
        uint8_t windowSize,
        uint16_t mss,
        uint8_t ttl,
        bool windowScaling,
        bool sack,
        bool timestamps
    );
    
    TCPStackFingerprint m_customFingerprint;
    bool m_useCustom;
};

// VPN Controller
class VPNController {
public:
    VPNController();
    
    // VPN Connection
    bool connect(const std::string& vmId, const std::string& server, 
                 const std::string& protocol, int port);
    bool disconnect(const std::string& vmId);
    bool reconnect(const std::string& vmId);
    
    // VPN Status
    bool isConnected(const std::string& vmId);
    std::string getVPNIP(const std::string& vmId);
    std::string getVPNServer(const std::string& vmId);
    
    // Protocol Support
    bool supportsProtocol(const std::string& protocol);
    std::vector<std::string> getSupportedProtocols();
    
    // VPN Profiles
    bool saveProfile(const std::string& name, const std::string& server,
                     const std::string& protocol, int port);
    bool loadProfile(const std::string& name);
    
private:
    struct VPNConnection {
        std::string vmId;
        std::string server;
        std::string protocol;
        int port;
        std::string ip;
        bool connected;
        uint64_t connectedSince;
    };
    
    std::map<std::string, VPNConnection> m_connections;
    std::mutex m_mutex;
};

} // namespace VirtualPhonePro
