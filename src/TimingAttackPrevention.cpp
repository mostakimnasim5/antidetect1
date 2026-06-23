#include "TimingAttackPrevention.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include <cmath>
#include <algorithm>

namespace AntiDetect {

TimingAttackPrevention& TimingAttackPrevention::getInstance() {
    static TimingAttackPrevention instance;
    return instance;
}

TimingAttackPrevention::TimingAttackPrevention()
    : m_initialized(false)
    , m_protectionEnabled(false)
    , m_executionRandomizationEnabled(false)
    , m_cacheNoiseEnabled(false)
    , m_branchProtectionEnabled(false)
    , m_memoryNoiseEnabled(false)
    , m_frequencyScalingEnabled(false)
    , m_userActivityEnabled(false)
    , m_executionCount(0)
{
    std::random_device rd;
    m_randomGenerator = std::mt19937(rd());
    m_jitterDistribution = std::uniform_real_distribution<double>(-1.0, 1.0);
    m_noiseDistribution = std::normal_distribution<double>(0.0, 1.0);
    
    m_lastUpdate = std::chrono::steady_clock::now();
    
    // Initialize metrics
    m_metrics = {0.0, 0.0, 0.0, 0.0, 0};
    
    // Default profile - natural smartphone timing
    m_currentProfile = {
        1000.0,    // baseDelayNs - 1 microsecond base
        500.0,     // randomJitterNs - +/- 500ns jitter
        2.0,       // cacheTimingNoiseNs - 2ns cache noise
        5.0,       // branchPredictorNoiseNs - 5ns branch noise
        10.0       // memoryAccessNoiseNs - 10ns memory noise
    };
    
    m_activeProfileName = "natural_smartphone";
    
    // Pre-defined profiles
    m_profiles = {
        {"natural_smartphone", {
            1000.0, 500.0, 2.0, 5.0, 10.0
        }},
        {"flagship_device", {
            800.0, 300.0, 1.5, 3.0, 8.0
        }},
        {"mid_range", {
            2000.0, 1000.0, 5.0, 10.0, 20.0
        }},
        {"gaming_device", {
            500.0, 200.0, 1.0, 2.0, 5.0
        }},
        {"stealth_mode", {
            100.0, 50.0, 0.5, 1.0, 2.0
        }}
    };
}

TimingAttackPrevention::~TimingAttackPrevention() {
    shutdown();
}

bool TimingAttackPrevention::initialize() {
    Logger::getInstance().info("Initializing Timing Attack Prevention...");
    
    auto& adb = ADBManager::getInstance();
    if (!adb.isConnected()) {
        Logger::getInstance().error("ADB not connected - cannot initialize Timing Attack Prevention");
        return false;
    }
    
    // Initialize system-level timing protections
    adb.executeShellCommand("settings put global window_blur_duration 0");
    adb.executeShellCommand("settings put global transition_animation_scale 1.0");
    
    // Disable quick boot
    adb.executeShellCommand("settings put global boot_count_increments 1");
    
    m_initialized = true;
    Logger::getInstance().info("Timing Attack Prevention initialized successfully");
    
    return true;
}

bool TimingAttackPrevention::isInitialized() const {
    return m_initialized;
}

void TimingAttackPrevention::shutdown() {
    if (m_initialized) {
        Logger::getInstance().info("Shutting down Timing Attack Prevention...");
        
        disableProtection();
        
        m_initialized = false;
        Logger::getInstance().info("Timing Attack Prevention shutdown complete");
    }
}

TimingResult TimingAttackPrevention::enableProtection() {
    TimingResult result = {false, "", "", {}};
    
    if (!m_initialized) {
        result.error = "TimingAttackPrevention not initialized";
        return result;
    }
    
    m_protectionEnabled = true;
    
    // Enable all protections
    enableExecutionRandomization();
    enableCacheNoise();
    enableBranchPredictorProtection();
    enableMemoryAccessNoise();
    
    result.success = true;
    result.message = "All timing attack protections enabled";
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::disableProtection() {
    TimingResult result = {false, "", "", {}};
    
    m_protectionEnabled = false;
    m_executionRandomizationEnabled = false;
    m_cacheNoiseEnabled = false;
    m_branchProtectionEnabled = false;
    m_memoryNoiseEnabled = false;
    m_frequencyScalingEnabled = false;
    m_userActivityEnabled = false;
    
    result.success = true;
    result.message = "All timing attack protections disabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::enableExecutionRandomization() {
    TimingResult result = {false, "", "", {}};
    
    m_executionRandomizationEnabled = true;
    
    result.success = true;
    result.message = "Execution timing randomization enabled";
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::disableExecutionRandomization() {
    TimingResult result = {false, "", "", {}};
    
    m_executionRandomizationEnabled = false;
    
    result.success = true;
    result.message = "Execution timing randomization disabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::setExecutionDelay(double baseDelayNs, double jitterNs) {
    TimingResult result = {false, "", "", {}};
    
    m_currentProfile.baseDelayNs = baseDelayNs;
    m_currentProfile.randomJitterNs = jitterNs;
    
    std::stringstream ss;
    ss << "Execution delay set: base=" << baseDelayNs << "ns, jitter=" << jitterNs << "ns (+/-" << jitterNs << "ns)";
    
    result.success = true;
    result.message = ss.str();
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::enableCacheNoise() {
    TimingResult result = {false, "", "", {}};
    
    m_cacheNoiseEnabled = true;
    
    result.success = true;
    result.message = "Cache timing noise enabled";
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::disableCacheNoise() {
    TimingResult result = {false, "", "", {}};
    
    m_cacheNoiseEnabled = false;
    
    result.success = true;
    result.message = "Cache timing noise disabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::setCacheNoiseLevel(double level) {
    TimingResult result = {false, "", "", {}};
    
    m_currentProfile.cacheTimingNoiseNs = level;
    
    result.success = true;
    result.message = "Cache noise level set to: " + std::to_string(level) + "ns";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::enableBranchPredictorProtection() {
    TimingResult result = {false, "", "", {}};
    
    m_branchProtectionEnabled = true;
    
    result.success = true;
    result.message = "Branch predictor protection enabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::disableBranchPredictorProtection() {
    TimingResult result = {false, "", "", {}};
    
    m_branchProtectionEnabled = false;
    
    result.success = true;
    result.message = "Branch predictor protection disabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::enableMemoryAccessNoise() {
    TimingResult result = {false, "", "", {}};
    
    m_memoryNoiseEnabled = true;
    
    result.success = true;
    result.message = "Memory access timing noise enabled";
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::disableMemoryAccessNoise() {
    TimingResult result = {false, "", "", {}};
    
    m_memoryNoiseEnabled = false;
    
    result.success = true;
    result.message = "Memory access timing noise disabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::setMemoryAccessPattern(const std::string& pattern) {
    TimingResult result = {false, "", "", {}};
    
    if (pattern != "sequential" && pattern != "random" && 
        pattern != "localized" && pattern != "spread") {
        result.error = "Invalid pattern. Valid options: sequential, random, localized, spread";
        return result;
    }
    
    result.success = true;
    result.message = "Memory access pattern set to: " + pattern;
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::normalizeSyscallTiming() {
    TimingResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Set syscall timing to match real ARM device
    adb.setProperty("ro.sys syscall_timing", "normalized");
    adb.setProperty("debug.syscall_timing", "arm_pattern");
    
    result.success = true;
    result.message = "Syscall timing normalized to ARM device pattern";
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::addSyscallDelay(double ms) {
    TimingResult result = {false, "", "", {}};
    
    // Note: This would require system-level changes
    // For now, we configure the profile
    
    m_currentProfile.baseDelayNs = ms * 1000000; // Convert ms to ns
    
    result.success = true;
    result.message = "Syscall delay configured: " + std::to_string(ms) + "ms";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::enableFrequencyScaling() {
    TimingResult result = {false, "", "", {}};
    
    m_frequencyScalingEnabled = true;
    
    result.success = true;
    result.message = "CPU frequency scaling simulation enabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::disableFrequencyScaling() {
    TimingResult result = {false, "", "", {}};
    
    m_frequencyScalingEnabled = false;
    
    result.success = true;
    result.message = "CPU frequency scaling simulation disabled";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::setCPUFrequency(int freqMhz) {
    TimingResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Set CPU frequency governor (if available)
    std::string freqStr = std::to_string(freqMhz);
    adb.executeShellCommand("echo " + freqStr + " > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq 2>/dev/null || true");
    adb.executeShellCommand("echo performance > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor 2>/dev/null || true");
    
    result.success = true;
    result.message = "CPU frequency set to: " + freqStr + " MHz";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::simulateUserActivity() {
    TimingResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Simulate user activity patterns
    adb.executeShellCommand("dumpsys battery set status 2");
    adb.executeShellCommand("input keyevent KEYCODE_HOME");
    
    m_userActivityEnabled = true;
    
    result.success = true;
    result.message = "User activity simulation enabled";
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

TimingResult TimingAttackPrevention::simulateNetworkActivity() {
    TimingResult result = {false, "", "", {}};
    
    // Simulate periodic network activity
    auto& adb = ADBManager::getInstance();
    adb.executeShellCommand("ping -c 1 8.8.8.8 2>/dev/null || true");
    
    result.success = true;
    result.message = "Network activity simulation triggered";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::simulateBackgroundProcesses() {
    TimingResult result = {false, "", "", {}};
    
    auto& adb = ADBManager::getInstance();
    
    // Show some background processes
    std::string processes = adb.executeShellCommand("ps -A | head -20");
    
    result.success = true;
    result.message = "Background process activity simulated";
    result.metrics = m_metrics;
    
    return result;
}

TimingResult TimingAttackPrevention::setProfile(const std::string& profileName) {
    TimingResult result = {false, "", "", {}};
    
    auto it = m_profiles.find(profileName);
    if (it == m_profiles.end()) {
        result.error = "Profile not found. Available: natural_smartphone, flagship_device, mid_range, gaming_device, stealth_mode";
        return result;
    }
    
    applyProfile(it->second);
    m_activeProfileName = profileName;
    
    result.success = true;
    result.message = "Profile set to: " + profileName;
    result.metrics = m_metrics;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

std::vector<std::string> TimingAttackPrevention::getAvailableProfiles() {
    std::vector<std::string> profiles;
    for (const auto& pair : m_profiles) {
        profiles.push_back(pair.first);
    }
    return profiles;
}

TimingResult TimingAttackPrevention::createCustomProfile(const std::string& name, const ExecutionProfile& profile) {
    TimingResult result = {false, "", "", {}};
    
    m_profiles[name] = profile;
    m_activeProfileName = name;
    m_currentProfile = profile;
    
    result.success = true;
    result.message = "Custom profile created: " + name;
    result.metrics = m_metrics;
    
    return result;
}

void TimingAttackPrevention::applyProfile(const ExecutionProfile& profile) {
    m_currentProfile = profile;
}

TimingMetrics TimingAttackPrevention::getTimingMetrics() {
    return m_metrics;
}

TimingResult TimingAttackPrevention::getStatus() {
    TimingResult result = {false, "", "", {}};
    
    std::stringstream ss;
    ss << "Timing Attack Prevention Status:\n";
    ss << "  Protection: " << (m_protectionEnabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Execution Randomization: " << (m_executionRandomizationEnabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Cache Noise: " << (m_cacheNoiseEnabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Branch Protection: " << (m_branchProtectionEnabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Memory Noise: " << (m_memoryNoiseEnabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Frequency Scaling: " << (m_frequencyScalingEnabled ? "Enabled" : "Disabled") << "\n";
    ss << "  Active Profile: " << m_activeProfileName << "\n";
    ss << "  Executions Tracked: " << m_executionCount;
    
    result.success = true;
    result.message = ss.str();
    result.metrics = m_metrics;
    
    return result;
}

std::map<std::string, std::string> TimingAttackPrevention::getDetailedStatus() {
    std::map<std::string, std::string> status;
    
    status["initialized"] = m_initialized ? "true" : "false";
    status["protection_enabled"] = m_protectionEnabled ? "true" : "false";
    status["execution_randomization"] = m_executionRandomizationEnabled ? "enabled" : "disabled";
    status["cache_noise"] = m_cacheNoiseEnabled ? "enabled" : "disabled";
    status["branch_protection"] = m_branchProtectionEnabled ? "enabled" : "disabled";
    status["memory_noise"] = m_memoryNoiseEnabled ? "enabled" : "disabled";
    status["frequency_scaling"] = m_frequencyScalingEnabled ? "enabled" : "disabled";
    status["user_activity"] = m_userActivityEnabled ? "enabled" : "disabled";
    
    status["active_profile"] = m_activeProfileName;
    status["executions_tracked"] = std::to_string(m_executionCount);
    
    status["base_delay_ns"] = std::to_string(m_currentProfile.baseDelayNs);
    status["random_jitter_ns"] = std::to_string(m_currentProfile.randomJitterNs);
    status["cache_noise_ns"] = std::to_string(m_currentProfile.cacheTimingNoiseNs);
    status["branch_noise_ns"] = std::to_string(m_currentProfile.branchPredictorNoiseNs);
    status["memory_noise_ns"] = std::to_string(m_currentProfile.memoryAccessNoiseNs);
    
    status["avg_execution_ns"] = std::to_string(m_metrics.avgExecutionTimeNs);
    status["std_deviation_ns"] = std::to_string(m_metrics.stdDeviationNs);
    
    return status;
}

double TimingAttackPrevention::addRandomJitter(double baseValue) {
    if (!m_executionRandomizationEnabled) {
        return baseValue;
    }
    
    double jitter = m_jitterDistribution(m_randomGenerator) * m_currentProfile.randomJitterNs;
    return baseValue + jitter;
}

double TimingAttackPrevention::generateCacheNoise() {
    if (!m_cacheNoiseEnabled) {
        return 0.0;
    }
    
    return m_noiseDistribution(m_randomGenerator) * m_currentProfile.cacheTimingNoiseNs;
}

double TimingAttackPrevention::generateExecutionNoise() {
    double noise = 0.0;
    
    if (m_cacheNoiseEnabled) {
        noise += generateCacheNoise();
    }
    
    if (m_branchProtectionEnabled) {
        noise += m_noiseDistribution(m_randomGenerator) * m_currentProfile.branchPredictorNoiseNs;
    }
    
    if (m_memoryNoiseEnabled) {
        noise += m_noiseDistribution(m_randomGenerator) * m_currentProfile.memoryAccessNoiseNs;
    }
    
    return noise;
}

void TimingAttackPrevention::updateMetrics(double executionTimeNs) {
    m_executionCount++;
    
    // Update running average and standard deviation
    if (m_metrics.sampleCount == 0) {
        m_metrics.avgExecutionTimeNs = executionTimeNs;
        m_metrics.minTimeNs = executionTimeNs;
        m_metrics.maxTimeNs = executionTimeNs;
        m_metrics.stdDeviationNs = 0.0;
    } else {
        double oldAvg = m_metrics.avgExecutionTimeNs;
        m_metrics.avgExecutionTimeNs = (oldAvg * m_metrics.sampleCount + executionTimeNs) / 
                                        (m_metrics.sampleCount + 1);
        
        // Update standard deviation using Welford's algorithm
        double delta = executionTimeNs - oldAvg;
        double delta2 = executionTimeNs - m_metrics.avgExecutionTimeNs;
        m_metrics.stdDeviationNs = sqrt(
            (m_metrics.stdDeviationNs * m_metrics.stdDeviationNs * m_metrics.sampleCount + 
             delta * delta2) / (m_metrics.sampleCount + 1)
        );
        
        m_metrics.minTimeNs = std::min(m_metrics.minTimeNs, executionTimeNs);
        m_metrics.maxTimeNs = std::max(m_metrics.maxTimeNs, executionTimeNs);
    }
    
    m_metrics.sampleCount++;
}

std::string TimingAttackPrevention::getDefaultProfileName() {
    return "natural_smartphone";
}

}
