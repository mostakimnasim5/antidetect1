/**
 * VMManager - Virtual Machine Lifecycle Management
 */

#include "core/VMManager.hpp"
#include "hardware/HardwareEmulator.hpp"
#include "network/NetworkVirtualizer.hpp"
#include "anti_detect/AntiDetectionManager.hpp"
#include <iostream>
#include <algorithm>
#include <sys/stat.h>

#ifdef VPP_PLATFORM_WINDOWS
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/wait.h>
#endif

namespace VirtualPhonePro {

VMManager::VMManager() : m_running(false) {}

VMManager::~VMManager() {
    shutdown();
}

VMManager& VMManager::getInstance() {
    static VMManager instance;
    return instance;
}

bool VMManager::initialize() {
    if (m_running) return true;
    
    // Check hardware virtualization
    if (!checkIntelVT() && !checkAMDV()) {
        std::cerr << "[WARNING] Hardware virtualization not available" << std::endl;
    }
    
    // Initialize subsystems
    m_hardwareEmulator = std::make_unique<HardwareEmulator>();
    m_networkVirtualizer = std::make_unique<NetworkVirtualizer>();
    m_antiDetection = std::make_unique<AntiDetectionManager>();
    
    if (!m_hardwareEmulator->initialize()) {
        std::cerr << "[ERROR] Failed to initialize hardware emulator" << std::endl;
        return false;
    }
    
    if (!m_networkVirtualizer->initialize()) {
        std::cerr << "[ERROR] Failed to initialize network virtualizer" << std::endl;
        return false;
    }
    
    if (!m_antiDetection->initialize()) {
        std::cerr << "[ERROR] Failed to initialize anti-detection" << std::endl;
        return false;
    }
    
    m_running = true;
    std::cout << "[OK] VM Manager initialized" << std::endl;
    return true;
}

void VMManager::shutdown() {
    if (!m_running) return;
    
    // Stop all VMs
    stopAllVMs();
    
    // Shutdown subsystems
    if (m_hardwareEmulator) m_hardwareEmulator->shutdown();
    if (m_networkVirtualizer) m_networkVirtualizer->shutdown();
    if (m_antiDetection) m_antiDetection->shutdown();
    
    m_running = false;
    std::cout << "[OK] VM Manager shutdown complete" << std::endl;
}

std::string VMManager::createVM(const VMConfig& config) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    std::string vmId = generateUUID();
    
    auto instance = std::make_unique<VMInstance>();
    instance->id = vmId;
    instance->name = config.name;
    instance->profileId = config.profileId;
    instance->config = config;
    instance->state = VMStatus::State::STOPPED;
    instance->running = false;
    instance->workerThread = nullptr;
    instance->startTime = 0;
    instance->cpuUsage = 0;
    instance->memoryUsage = 0;
    
#ifdef VPP_PLATFORM_WINDOWS
    instance->processHandle = nullptr;
#else
    instance->processId = -1;
#endif
    
    // Generate default fingerprint if not provided
    if (instance->fingerprint.macAddress.empty()) {
        instance->fingerprint.macAddress = m_networkVirtualizer->generateUniqueMAC();
    }
    
    // Create VM directory
    std::string vmPath = "./vms/" + vmId;
#ifdef VPP_PLATFORM_WINDOWS
    CreateDirectoryA(vmPath.c_str(), nullptr);
#else
    mkdir(vmPath.c_str(), 0755);
#endif
    
    m_vms[vmId] = std::move(instance);
    
    std::cout << "[OK] VM created: " << vmId << " (" << config.name << ")" << std::endl;
    return vmId;
}

VMResult VMManager::startVM(const std::string& vmId) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return VMResult::Error("VM not found: " + vmId);
    }
    
    auto* instance = it->second.get();
    
    if (instance->state == VMStatus::State::RUNNING) {
        return VMResult::Error("VM already running: " + vmId);
    }
    
    // Update state
    instance->state = VMStatus::State::STARTING;
    
    // Launch VM process
    if (!launchVMProcess(instance)) {
        instance->state = VMStatus::State::ERROR;
        return VMResult::Error("Failed to launch VM process");
    }
    
    instance->running = true;
    instance->state = VMStatus::State::RUNNING;
    instance->startTime = time(nullptr);
    
    // Start worker thread for monitoring
    instance->workerThread = new std::thread(&VMManager::vmWorker, this, instance);
    
    std::cout << "[OK] VM started: " << vmId << std::endl;
    return VMResult::Ok("VM started: " + instance->name);
}

VMResult VMManager::stopVM(const std::string& vmId) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return VMResult::Error("VM not found: " + vmId);
    }
    
    auto* instance = it->second.get();
    
    if (instance->state == VMStatus::State::STOPPED) {
        return VMResult::Ok("VM already stopped");
    }
    
    instance->state = VMStatus::State::STOPPING;
    instance->running = false;
    
    // Wait for worker thread
    if (instance->workerThread && instance->workerThread->joinable()) {
        instance->workerThread->join();
        delete instance->workerThread;
        instance->workerThread = nullptr;
    }
    
    // Terminate process
#ifdef VPP_PLATFORM_WINDOWS
    if (instance->processHandle) {
        TerminateProcess(instance->processHandle, 0);
        CloseHandle(instance->processHandle);
        instance->processHandle = nullptr;
    }
#else
    if (instance->processId > 0) {
        kill(instance->processId, SIGTERM);
        waitpid(instance->processId, nullptr, 0);
        instance->processId = -1;
    }
#endif
    
    instance->state = VMStatus::State::STOPPED;
    
    std::cout << "[OK] VM stopped: " << vmId << std::endl;
    return VMResult::Ok("VM stopped: " + instance->name);
}

VMResult VMManager::pauseVM(const std::string& vmId) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return VMResult::Error("VM not found: " + vmId);
    }
    
    auto* instance = it->second.get();
    
    if (instance->state != VMStatus::State::RUNNING) {
        return VMResult::Error("VM not running: " + vmId);
    }
    
    instance->state = VMStatus::State::PAUSED;
    
#ifdef VPP_PLATFORM_WINDOWS
    if (instance->processHandle) {
        SuspendThread(instance->processHandle);
    }
#endif
    
    return VMResult::Ok("VM paused");
}

VMResult VMManager::resumeVM(const std::string& vmId) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return VMResult::Error("VM not found: " + vmId);
    }
    
    auto* instance = it->second.get();
    
    if (instance->state != VMStatus::State::PAUSED) {
        return VMResult::Error("VM not paused: " + vmId);
    }
    
    instance->state = VMStatus::State::RUNNING;
    
#ifdef VPP_PLATFORM_WINDOWS
    if (instance->processHandle) {
        ResumeThread(instance->processHandle);
    }
#endif
    
    return VMResult::Ok("VM resumed");
}

VMResult VMManager::deleteVM(const std::string& vmId) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return VMResult::Error("VM not found: " + vmId);
    }
    
    auto* instance = it->second.get();
    
    // Stop if running
    if (instance->running || instance->state == VMStatus::State::RUNNING) {
        stopVM(vmId);
    }
    
    // Delete VM files
    std::string vmPath = "./vms/" + vmId;
#ifdef VPP_PLATFORM_WINDOWS
    std::string cmd = "rmdir /s /q " + vmPath;
#else
    std::string cmd = "rm -rf " + vmPath;
#endif
    system(cmd.c_str());
    
    m_vms.erase(it);
    
    std::cout << "[OK] VM deleted: " << vmId << std::endl;
    return VMResult::Ok("VM deleted");
}

std::vector<VMInfo> VMManager::listVMs() {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    std::vector<VMInfo> vmList;
    
    for (const auto& [id, instance] : m_vms) {
        VMInfo info;
        info.id = instance->id;
        info.name = instance->name;
        info.profileId = instance->profileId;
        info.osVersion = instance->config.androidVersion;
        info.state = instance->state == VMStatus::State::RUNNING ? "running" : "stopped";
        info.cpuCores = instance->config.cpuCores;
        info.memoryMB = instance->config.memoryMB;
        info.storageMB = instance->config.storageMB;
        info.macAddress = instance->fingerprint.macAddress;
        info.fingerprint = instance->fingerprint.fingerprint;
        
        vmList.push_back(info);
    }
    
    return vmList;
}

VMInfo VMManager::getVMInfo(const std::string& vmId) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    VMInfo info;
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return info;
    }
    
    const auto* instance = it->second.get();
    
    info.id = instance->id;
    info.name = instance->name;
    info.profileId = instance->profileId;
    info.osVersion = instance->config.androidVersion;
    info.state = instance->state == VMStatus::State::RUNNING ? "running" : "stopped";
    info.cpuCores = instance->config.cpuCores;
    info.memoryMB = instance->config.memoryMB;
    info.storageMB = instance->config.storageMB;
    info.macAddress = instance->fingerprint.macAddress;
    info.fingerprint = instance->fingerprint.fingerprint;
    
    return info;
}

VMStatus VMManager::getVMStatus(const std::string& vmId) {
    VMStatus status;
    
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        status.state = VMStatus::State::ERROR;
        return status;
    }
    
    const auto* instance = it->second.get();
    
    status.state = instance->state;
    status.cpuUsage = instance->cpuUsage;
    status.memoryUsage = instance->memoryUsage;
    status.memoryTotal = instance->config.memoryMB;
    status.memoryUsed = instance->memoryUsage;
    status.uptime = time(nullptr) - instance->startTime;
    
    return status;
}

bool VMManager::allocateResources(const std::string& vmId, int cpuCores, int memoryMB) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return false;
    }
    
    auto* instance = it->second.get();
    
    instance->config.cpuCores = cpuCores;
    instance->config.memoryMB = memoryMB;
    
    return true;
}

void VMManager::releaseResources(const std::string& vmId) {
    // Release allocated resources
    std::lock_guard<std::mutex> lock(m_vmMutex);
    m_vms.erase(vmId);
}

VMResult VMManager::applyProfile(const std::string& vmId, const FingerprintConfig& profile) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it == m_vms.end()) {
        return VMResult::Error("VM not found: " + vmId);
    }
    
    auto* instance = it->second.get();
    instance->fingerprint = profile;
    instance->profileId = profile.model;
    
    // Apply hardware emulation
    m_hardwareEmulator->applyProfile(profile);
    
    // Apply network virtualization
    if (!profile.macAddress.empty()) {
        m_networkVirtualizer->createNetworkInterface(vmId, NetworkMode::NAT);
    }
    
    return VMResult::Ok("Profile applied: " + profile.model);
}

VMResult VMManager::startAllVMs() {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    for (const auto& [id, instance] : m_vms) {
        if (instance->state != VMStatus::State::RUNNING) {
            startVM(id);
        }
    }
    
    return VMResult::Ok("All VMs started");
}

VMResult VMManager::stopAllVMs() {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    for (const auto& [id, instance] : m_vms) {
        if (instance->state == VMStatus::State::RUNNING) {
            stopVM(id);
        }
    }
    
    return VMResult::Ok("All VMs stopped");
}

bool VMManager::launchVMProcess(VMInstance* instance) {
#ifdef VPP_PLATFORM_WINDOWS
    // Windows: Launch QEMU/Android emulator process
    std::string cmd = "qemu-system-x86_64.exe";
    cmd += " -m " + std::to_string(instance->config.memoryMB);
    cmd += " -smp " + std::to_string(instance->config.cpuCores);
    cmd += " -hda ./vms/" + instance->id + "/disk.img";
    cmd += " -display sdl";
    
    STARTUPINFOA si = {sizeof(si)};
    PROCESS_INFORMATION pi;
    
    if (CreateProcessA(nullptr, cmd.data(), nullptr, nullptr, FALSE, 
                      CREATE_NEW_CONSOLE, nullptr, nullptr, &si, &pi)) {
        instance->processHandle = pi.hProcess;
        instance->processId = pi.dwProcessId;
        return true;
    }
    
    return false;
#else
    // Linux: Fork process and run QEMU
    pid_t pid = fork();
    
    if (pid < 0) {
        return false;
    }
    
    if (pid == 0) {
        // Child process
        std::string cmd = "qemu-system-x86_64";
        execlp(cmd.c_str(), cmd.c_str(), nullptr);
        _exit(1);
    }
    
    instance->processId = pid;
    return true;
#endif
}

void VMManager::vmWorker(VMInstance* instance) {
    while (instance->running) {
        // Update resource usage
        instance->cpuUsage = rand() % 100; // Placeholder
        instance->memoryUsage = instance->config.memoryMB / 4; // Placeholder
        
        // Sleep for monitoring interval
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void VMManager::cleanupVM(VMInstance* instance) {
    // Cleanup VM resources
}

void VMManager::updateVMStatus(const std::string& vmId, VMStatus::State state) {
    std::lock_guard<std::mutex> lock(m_vmMutex);
    
    auto it = m_vms.find(vmId);
    if (it != m_vms.end()) {
        it->second->state = state;
    }
}

bool VMManager::checkIntelVT() {
#ifdef VPP_PLATFORM_WINDOWS
    return true; // Simplified check
#elif defined(VPP_PLATFORM_LINUX)
    return access("/dev/kvm", R_OK | W_OK) == 0;
#else
    return false;
#endif
}

bool VMManager::checkAMDV() {
    // Check for AMD-V virtualization
    return true; // Simplified
}

bool VMManager::checkHyperV() {
#ifdef VPP_PLATFORM_WINDOWS
    return true; // Simplified
#else
    return false;
#endif
}

} // namespace VirtualPhonePro
