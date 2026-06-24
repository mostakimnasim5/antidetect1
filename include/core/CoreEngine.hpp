#pragma once

/**
 * CoreEngine - Main orchestration engine for VirtualPhonePro
 * 
 * Manages all subsystems and provides unified API for VM operations.
 */

#include "../VirtualPhonePro.hpp"
#include "VMManager.hpp"
#include "ProfileManager.hpp"
#include "ResourceAllocator.hpp"
#include "StorageManager.hpp"
#include "SnapshotManager.hpp"

namespace VirtualPhonePro {

class CoreEngine {
public:
    static CoreEngine& getInstance();
    
    // Lifecycle
    bool initialize();
    void shutdown();
    bool isInitialized() const;
    
    // VM Operations
    std::string createVM(const VMConfig& config);
    VMResult startVM(const std::string& vmId);
    VMResult stopVM(const std::string& vmId);
    VMResult pauseVM(const std::string& vmId);
    VMResult resumeVM(const std::string& vmId);
    VMResult deleteVM(const std::string& vmId);
    VMResult cloneVM(const std::string& vmId, const std::string& newName);
    
    // VM Queries
    std::vector<VMInfo> listVMs();
    VMInfo getVMInfo(const std::string& vmId);
    VMStatus getVMStatus(const std::string& vmId);
    
    // Profile Operations
    std::string createProfile(const FingerprintConfig& config);
    FingerprintConfig generateRandomProfile(const std::string& region = "");
    std::vector<FingerprintConfig> getAvailableProfiles();
    VMResult applyProfile(const std::string& vmId, const std::string& profileId);
    
    // Snapshot Operations
    std::string createSnapshot(const std::string& vmId, const std::string& name);
    VMResult restoreSnapshot(const std::string& vmId, const std::string& snapshotId);
    std::vector<Snapshot> listSnapshots(const std::string& vmId);
    VMResult deleteSnapshot(const std::string& vmId, const std::string& snapshotId);
    
    // System Operations
    bool checkHardwareVirtualization();
    std::map<std::string, int> getSystemResources();
    bool allocateResources(const std::string& vmId, int cpuCores, int memoryMB);
    
    // Configuration
    void setConfig(const std::string& key, const std::string& value);
    std::string getConfig(const std::string& key);
    
    // Logging
    void setLogLevel(LogLevel level);
    void setLogCallback(LogCallback callback);
    void log(LogLevel level, const std::string& message);
    
    // Events
    void setStatusCallback(StatusCallback callback);
    void notifyStatusChange(const std::string& vmId, const VMStatus& status);
    
private:
    CoreEngine();
    ~CoreEngine();
    CoreEngine(const CoreEngine&) = delete;
    CoreEngine& operator=(const CoreEngine&) = delete;
    
    struct Impl;
    std::unique_ptr<Impl> m_impl;
};

} // namespace VirtualPhonePro
