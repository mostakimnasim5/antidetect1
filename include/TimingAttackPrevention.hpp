#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <random>
#include <chrono>
#include <functional>
#include <atomic>

namespace AntiDetect {

struct TimingMetrics {
    double avgExecutionTimeNs;
    double stdDeviationNs;
    double minTimeNs;
    double maxTimeNs;
    int sampleCount;
};

struct ExecutionProfile {
    double baseDelayNs;
    double randomJitterNs;
    double cacheTimingNoiseNs;
    double branchPredictorNoiseNs;
    double memoryAccessNoiseNs;
};

struct TimingResult {
    bool success;
    std::string message;
    std::string error;
    TimingMetrics metrics;
};

class TimingAttackPrevention {
public:
    static TimingAttackPrevention& getInstance();
    
    TimingAttackPrevention();
    ~TimingAttackPrevention();
    
    bool initialize();
    bool isInitialized() const;
    void shutdown();
    
    // Core timing protection
    TimingResult enableProtection();
    TimingResult disableProtection();
    
    // Execution timing randomization
    TimingResult enableExecutionRandomization();
    TimingResult disableExecutionRandomization();
    TimingResult setExecutionDelay(double baseDelayNs, double jitterNs);
    
    // Cache timing attack prevention
    TimingResult enableCacheNoise();
    TimingResult disableCacheNoise();
    TimingResult setCacheNoiseLevel(double level);
    
    // Branch predictor protection
    TimingResult enableBranchPredictorProtection();
    TimingResult disableBranchPredictorProtection();
    
    // Memory access timing
    TimingResult enableMemoryAccessNoise();
    TimingResult disableMemoryAccessNoise();
    TimingResult setMemoryAccessPattern(const std::string& pattern);
    
    // System call timing
    TimingResult normalizeSyscallTiming();
    TimingResult addSyscallDelay(double ms);
    
    // CPU frequency scaling simulation
    TimingResult enableFrequencyScaling();
    TimingResult disableFrequencyScaling();
    TimingResult setCPUFrequency(int freqMhz);
    
    // Device activity simulation
    TimingResult simulateUserActivity();
    TimingResult simulateNetworkActivity();
    TimingResult simulateBackgroundProcesses();
    
    // Profile management
    TimingResult setProfile(const std::string& profileName);
    std::vector<std::string> getAvailableProfiles();
    TimingResult createCustomProfile(const std::string& name, const ExecutionProfile& profile);
    
    // Metrics and status
    TimingMetrics getTimingMetrics();
    TimingResult getStatus();
    std::map<std::string, std::string> getDetailedStatus();
    
    // Utility functions
    double addRandomJitter(double baseValue);
    double generateCacheNoise();
    double generateExecutionNoise();
    
private:
    void updateMetrics(double executionTimeNs);
    std::string getDefaultProfileName();
    void applyProfile(const ExecutionProfile& profile);
    
    bool m_initialized;
    bool m_protectionEnabled;
    bool m_executionRandomizationEnabled;
    bool m_cacheNoiseEnabled;
    bool m_branchProtectionEnabled;
    bool m_memoryNoiseEnabled;
    bool m_frequencyScalingEnabled;
    bool m_userActivityEnabled;
    
    ExecutionProfile m_currentProfile;
    TimingMetrics m_metrics;
    
    std::string m_activeProfileName;
    std::map<std::string, ExecutionProfile> m_profiles;
    
    std::mt19937 m_randomGenerator;
    std::uniform_real_distribution<double> m_jitterDistribution;
    std::normal_distribution<double> m_noiseDistribution;
    
    std::atomic<int> m_executionCount;
    std::chrono::steady_clock::time_point m_lastUpdate;
};

}
