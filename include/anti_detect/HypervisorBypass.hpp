#pragma once

/**
 * HypervisorBypass - Advanced Hypervisor Detection Prevention
 * 
 * Closes all detection vectors that apps use to detect:
 * - CPU brand/model fingerprinting
 * - CPU cache timing attacks
 * - Hypervisor bit cpuid detection
 * - Hardware feature enumeration
 * - CPU topology detection (cores, threads)
 * - RDTSC/RDTSCP timing attacks
 * 
 * Protection Level: Enterprise
 */

#include "../VirtualPhonePro.hpp"
#include <cpuid.h>
#include <intrin.h>
#include <chrono>
#include <random>

namespace VirtualPhonePro {
namespace AntiDetect {

class HypervisorBypass {
public:
    static HypervisorBypass& getInstance();
    
    // Initialize hypervisor bypass
    bool initialize();
    
    // CPUID Emulation - Returns fake native CPU info
    bool emulateCPUID(uint32_t leaf, uint32_t subleaf,
                     uint32_t* eax, uint32_t* ebx, 
                     uint32_t* ecx, uint32_t* edx);
    
    // Timing Attack Prevention - Add random noise to rdtsc
    uint64_t getEmulatedTSC();
    uint64_t getEmulatedRDTSCP();
    
    // Cache Timing Protection
    void flushCache();
    uint64_t measureCacheTiming(void* addr);
    
    // CPU Brand String Emulation
    void setCPUBrandString(const std::string& brand);
    std::string getEmulatedCPUBrand();
    
    // CPU Features - Hide hypervisor/KVM/QEMU flags
    uint32_t getEmulatedCPUFeatures(uint32_t leaf);
    
    // Thread/Process Detection Prevention
    bool isThreadRealistic();
    
    // Memory Timing Randomization
    uint64_t getEmulatedMemoryLatency();
    
    // Disable specific CPUID leaves that reveal VM
    void disableLeaves(const std::vector<uint32_t>& leaves);
    
    // Set CPU model to emulate (Intel/AMD mobile chips)
    void setEmulatedCPUModel(const std::string& vendor,
                            const std::string& model,
                            const std::string& family);
    
private:
    HypervisorBypass();
    ~HypervisorBypass();
    HypervisorBypass(const HypervisorBypass&) = delete;
    HypervisorBypass& operator=(const HypervisorBypass&) = delete;
    
    // CPU Brand spoofing
    std::vector<std::string> m_cpuBrandCache;
    std::string m_emulatedBrand;
    
    // Timing noise generator
    std::mt19937_64 m_tscNoiseGenerator;
    std::uniform_int_distribution<uint64_t> m_tscNoiseDist;
    uint64_t m_tscOffset;
    uint64_t m_baseTSC;
    
    // CPU Model emulation
    std::string m_vendor;
    std::string m_model;
    std::string m_family;
    uint32_t m_stepping;
    uint32_t m_modelExt;
    
    // Disabled leaves
    std::set<uint32_t> m_disabledLeaves;
    
    // Cache timing baseline
    uint64_t m_cacheBaseline;
    
    // TSX (Transactional Synchronization Extensions) emulation
    bool m_tsxEnabled;
    
    // IA32_TSC_AUX emulation
    uint32_t m_tscAux;
    
    // CPU Topology
    uint32_t m_physicalCores;
    uint32_t m_logicalProcessors;
    uint32_t m_threadsPerCore;
    
    void initializeCPUModel();
    void initializeTSCEmulation();
    uint64_t applyTSCNoise(uint64_t rawTSC);
    
    // CPU Brand string normalization
    std::string normalizeCPUBrand(const std::string& input);
};

} // namespace AntiDetect
} // namespace VirtualPhonePro
