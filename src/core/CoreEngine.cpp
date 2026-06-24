/**
 * CoreEngine - Implementation
 */

#include "core/CoreEngine.hpp"
#include <iostream>
#include <algorithm>

namespace VirtualPhonePro {

// ============================================
// CoreEngine::Impl - PIMPL Pattern
// ============================================
struct CoreEngine::Impl {
    VMManager& vmManager;
    ProfileManager& profileManager;
    ResourceAllocator resourceAllocator;
    StorageManager storageManager;
    SnapshotManager snapshotManager;
    
    LogLevel currentLogLevel = LogLevel::INFO;
    LogCallback logCallback;
    StatusCallback statusCallback;
    
    bool initialized = false;
    
    Impl() : vmManager(VMManager::getInstance()),
             profileManager(ProfileManager::getInstance()) {}
};

CoreEngine::CoreEngine() : m_impl(std::make_unique<Impl>()) {}

CoreEngine::~CoreEngine() = default;

CoreEngine& CoreEngine::getInstance() {
    static CoreEngine instance;
    return instance;
}

bool CoreEngine::initialize() {
    if (m_impl->initialized) {
        return true;
    }
    
    log(LogLevel::INFO, "Initializing VirtualPhonePro Engine v1.0.0");
    
    // Initialize subsystems
    if (!m_impl->vmManager.initialize()) {
        log(LogLevel::ERROR, "Failed to initialize VM Manager");
        return false;
    }
    
    if (!m_impl->profileManager.initialize()) {
        log(LogLevel::ERROR, "Failed to initialize Profile Manager");
        return false;
    }
    
    // Check hardware virtualization support
    if (!checkHardwareVirtualization()) {
        log(LogLevel::WARNING, "Hardware virtualization not detected. Performance may be reduced.");
    }
    
    m_impl->initialized = true;
    log(LogLevel::INFO, "VirtualPhonePro Engine initialized successfully");
    return true;
}

void CoreEngine::shutdown() {
    if (!m_impl->initialized) {
        return;
    }
    
    log(LogLevel::INFO, "Shutting down VirtualPhonePro Engine");
    
    // Stop all VMs
    m_impl->vmManager.stopAllVMs();
    
    // Shutdown subsystems
    m_impl->vmManager.shutdown();
    m_impl->profileManager.shutdown();
    
    m_impl->initialized = false;
    log(LogLevel::INFO, "VirtualPhonePro Engine shutdown complete");
}

bool CoreEngine::isInitialized() const {
    return m_impl->initialized;
}

std::string CoreEngine::createVM(const VMConfig& config) {
    return m_impl->vmManager.createVM(config);
}

VMResult CoreEngine::startVM(const std::string& vmId) {
    auto result = m_impl->vmManager.startVM(vmId);
    if (result.success) {
        notifyStatusChange(vmId, m_impl->vmManager.getVMStatus(vmId));
    }
    return result;
}

VMResult CoreEngine::stopVM(const std::string& vmId) {
    return m_impl->vmManager.stopVM(vmId);
}

VMResult CoreEngine::pauseVM(const std::string& vmId) {
    return m_impl->vmManager.pauseVM(vmId);
}

VMResult CoreEngine::resumeVM(const std::string& vmId) {
    return m_impl->vmManager.resumeVM(vmId);
}

VMResult CoreEngine::deleteVM(const std::string& vmId) {
    return m_impl->vmManager.deleteVM(vmId);
}

VMResult CoreEngine::cloneVM(const std::string& vmId, const std::string& newName) {
    // Get source VM info
    auto sourceVM = m_impl->vmManager.getVMInfo(vmId);
    if (sourceVM.id.empty()) {
        return VMResult::Error("Source VM not found");
    }
    
    // Create new VM with same config
    VMConfig newConfig = {};
    newConfig.name = newName;
    newConfig.profileId = sourceVM.profileId;
    newConfig.cpuCores = 4;
    newConfig.memoryMB = 4096;
    newConfig.storageMB = 16384;
    
    std::string newVmId = m_impl->vmManager.createVM(newConfig);
    
    // Generate new fingerprint for cloned VM
    auto profile = m_impl->profileManager.generateRandomProfile();
    m_impl->profileManager.saveProfile(profile, newVmId);
    m_impl->vmManager.applyProfile(newVmId, profile);
    
    return VMResult::Ok("VM cloned successfully: " + newVmId);
}

std::vector<VMInfo> CoreEngine::listVMs() {
    return m_impl->vmManager.listVMs();
}

VMInfo CoreEngine::getVMInfo(const std::string& vmId) {
    return m_impl->vmManager.getVMInfo(vmId);
}

VMStatus CoreEngine::getVMStatus(const std::string& vmId) {
    return m_impl->vmManager.getVMStatus(vmId);
}

std::string CoreEngine::createProfile(const FingerprintConfig& config) {
    std::string profileId = generateUUID();
    m_impl->profileManager.saveProfile(config, profileId);
    return profileId;
}

FingerprintConfig CoreEngine::generateRandomProfile(const std::string& region) {
    return m_impl->profileManager.generateRandomProfile(region);
}

std::vector<FingerprintConfig> CoreEngine::getAvailableProfiles() {
    return m_impl->profileManager.listSavedProfiles();
}

VMResult CoreEngine::applyProfile(const std::string& vmId, const std::string& profileId) {
    auto profile = m_impl->profileManager.loadProfile(profileId);
    if (!profile) {
        return VMResult::Error("Profile not found: " + profileId);
    }
    return m_impl->vmManager.applyProfile(vmId, *profile);
}

std::string CoreEngine::createSnapshot(const std::string& vmId, const std::string& name) {
    return m_impl->snapshotManager.createSnapshot(vmId, name);
}

VMResult CoreEngine::restoreSnapshot(const std::string& vmId, const std::string& snapshotId) {
    return m_impl->snapshotManager.restoreSnapshot(vmId, snapshotId);
}

std::vector<Snapshot> CoreEngine::listSnapshots(const std::string& vmId) {
    return m_impl->snapshotManager.listSnapshots(vmId);
}

VMResult CoreEngine::deleteSnapshot(const std::string& vmId, const std::string& snapshotId) {
    return m_impl->snapshotManager.deleteSnapshot(vmId, snapshotId);
}

bool CoreEngine::checkHardwareVirtualization() {
#ifdef VPP_PLATFORM_WINDOWS
    // Check Hyper-V, VT-x, etc.
    return true;
#elif defined(VPP_PLATFORM_LINUX)
    // Check KVM availability
    return access("/dev/kvm", F_OK) == 0;
#else
    return false;
#endif
}

std::map<std::string, int> CoreEngine::getSystemResources() {
    std::map<std::string, int> resources;
    resources["totalCPU"] = std::thread::hardware_concurrency();
    // Add memory, storage info
    return resources;
}

bool CoreEngine::allocateResources(const std::string& vmId, int cpuCores, int memoryMB) {
    return m_impl->vmManager.allocateResources(vmId, cpuCores, memoryMB);
}

void CoreEngine::setConfig(const std::string& key, const std::string& value) {
    m_impl->storageManager.setConfig(key, value);
}

std::string CoreEngine::getConfig(const std::string& key) {
    return m_impl->storageManager.getConfig(key);
}

void CoreEngine::setLogLevel(LogLevel level) {
    m_impl->currentLogLevel = level;
}

void CoreEngine::setLogCallback(LogCallback callback) {
    m_impl->logCallback = callback;
}

void CoreEngine::log(LogLevel level, const std::string& message) {
    if (level < m_impl->currentLogLevel) {
        return;
    }
    
    std::string levelStr;
    switch (level) {
        case LogLevel::DEBUG: levelStr = "DEBUG"; break;
        case LogLevel::INFO: levelStr = "INFO"; break;
        case LogLevel::WARNING: levelStr = "WARN"; break;
        case LogLevel::ERROR: levelStr = "ERROR"; break;
        case LogLevel::CRITICAL: levelStr = "CRITICAL"; break;
    }
    
    std::string fullMessage = "[" + levelStr + "] " + message;
    
    if (m_impl->logCallback) {
        m_impl->logCallback(level, fullMessage);
    }
    
    std::cout << fullMessage << std::endl;
}

void CoreEngine::setStatusCallback(StatusCallback callback) {
    m_impl->statusCallback = callback;
}

void CoreEngine::notifyStatusChange(const std::string& vmId, const VMStatus& status) {
    if (m_impl->statusCallback) {
        m_impl->statusCallback(status);
    }
}

} // namespace VirtualPhonePro
