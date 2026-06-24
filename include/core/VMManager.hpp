#pragma once

/**
 * VMManager - Virtual Machine Lifecycle Management
 * 
 * Handles creation, starting, stopping, and management of Android VMs.
 */

#include "../VirtualPhonePro.hpp"
#include <thread>
#include <mutex>
#include <queue>
#include <atomic>

namespace VirtualPhonePro {

// Forward declarations
class HardwareEmulator;
class NetworkVirtualizer;
class AntiDetectionManager;

struct VMInstance {
    std::string id;
    std::string name;
    std::string profileId;
    VMConfig config;
    FingerprintConfig fingerprint;
    VMStatus::State state;
    std::thread* workerThread;
    std::mutex stateMutex;
    std::atomic<bool> running;
    uint64_t startTime;
    
    // Resource tracking
    int cpuUsage;
    int memoryUsage;
    
    // Process/Handle
#ifdef VPP_PLATFORM_WINDOWS
    void* processHandle;
#else
    int processId;
#endif
};

class VMManager {
public:
    static VMManager& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    
    // VM Operations
    std::string createVM(const VMConfig& config);
    VMResult startVM(const std::string& vmId);
    VMResult stopVM(const std::string& vmId);
    VMResult pauseVM(const std::string& vmId);
    VMResult resumeVM(const std::string& vmId);
    VMResult deleteVM(const std::string& vmId);
    
    // Query
    std::vector<VMInfo> listVMs();
    VMInfo getVMInfo(const std::string& vmId);
    VMStatus getVMStatus(const std::string& vmId);
    
    // Resource Management
    bool allocateResources(const std::string& vmId, int cpuCores, int memoryMB);
    void releaseResources(const std::string& vmId);
    
    // Profile Application
    VMResult applyProfile(const std::string& vmId, const FingerprintConfig& profile);
    
    // Batch Operations
    VMResult startAllVMs();
    VMResult stopAllVMs();
    
private:
    VMManager();
    ~VMManager();
    VMManager(const VMManager&) = delete;
    VMManager& operator=(const VMManager&) = delete;
    
    // Internal methods
    bool launchVMProcess(VMInstance* instance);
    void vmWorker(VMInstance* instance);
    void cleanupVM(VMInstance* instance);
    void updateVMStatus(const std::string& vmId, VMStatus::State state);
    
    // Hardware virtualization check
    bool checkIntelVT();
    bool checkAMDV();
    bool checkHyperV();
    
    std::map<std::string, std::unique_ptr<VMInstance>> m_vms;
    std::mutex m_vmMutex;
    std::atomic<bool> m_running;
    
    // Hardware emulation subsystem
    std::unique_ptr<HardwareEmulator> m_hardwareEmulator;
    
    // Network virtualization subsystem
    std::unique_ptr<NetworkVirtualizer> m_networkVirtualizer;
    
    // Anti-detection subsystem
    std::unique_ptr<AntiDetectionManager> m_antiDetection;
};

} // namespace VirtualPhonePro
