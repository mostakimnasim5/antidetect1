/**
 * BatteryMaskingManager.cpp
 * 
 * Implementation of realistic smartphone battery behavior simulation.
 * Provides dynamic decay, temperature fluctuation, and sysfs mocking.
 * 
 * Author: AntiDetectPro
 * License: Commercial - Elite Level
 */

#include "BatteryMaskingManager.hpp"
#include "Logger.hpp"
#include "CryptoUtils.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>

namespace AntiDetect {

// ============================================================================
// Constants
// ============================================================================

namespace BatteryConst {
    // Default sysfs paths
    constexpr const char* DEFAULT_POWER_SUPPLY_PATH = "/sys/class/power_supply";
    constexpr const char* BATTERY_DIR = "battery";
    constexpr const char* BATTERY_ALT = "battery_power_supply";
    
    // Status strings
    constexpr const char* STATUS_CHARGING = "Charging";
    constexpr const char* STATUS_DISCHARGING = "Discharging";
    constexpr const char* STATUS_FULL = "Full";
    constexpr const char* STATUS_NOT_CHARGING = "Not Charging";
    constexpr const char* STATUS_UNKNOWN = "Unknown";
    
    // Health strings
    constexpr const char* HEALTH_GOOD = "Good";
    constexpr const char* HEALTH_OVERHEAT = "Overheat";
    constexpr const char* HEALTH_DEAD = "Dead";
    constexpr const char* HEALTH_OVER_VOLTAGE = "Over voltage";
    constexpr const char* HEALTH_UNSPEC_FAILURE = "Unspecified failure";
    constexpr const char* HEALTH_COLD = "Cold";
    constexpr const char* HEALTH_UNKNOWN = "Unknown";
    
    // Technology
    constexpr const char* TECH_LI_ION = "Li-ion";
    
    // Timing
    constexpr int MIN_SIMULATION_INTERVAL_MS = 1000;
    constexpr int MAX_SIMULATION_INTERVAL_MS = 10000;
    constexpr int DEFAULT_SIMULATION_INTERVAL_MS = 5000;
}

// ============================================================================
// Helper Functions
// ============================================================================

namespace {

std::string statusToString(BatteryStatus status) {
    switch (status) {
        case BatteryStatus::CHARGING: return BatteryConst::STATUS_CHARGING;
        case BatteryStatus::DISCHARGING: return BatteryConst::STATUS_DISCHARGING;
        case BatteryStatus::FULL: return BatteryConst::STATUS_FULL;
        case BatteryStatus::NOT_CHARGING: return BatteryConst::STATUS_NOT_CHARGING;
        default: return BatteryConst::STATUS_UNKNOWN;
    }
}

std::string healthToString(BatteryHealth health) {
    switch (health) {
        case BatteryHealth::GOOD: return BatteryConst::HEALTH_GOOD;
        case BatteryHealth::OVERHEAT: return BatteryConst::HEALTH_OVERHEAT;
        case BatteryHealth::DEAD: return BatteryConst::HEALTH_DEAD;
        case BatteryHealth::OVER_VOLTAGE: return BatteryConst::HEALTH_OVER_VOLTAGE;
        case BatteryHealth::UNSPEC_FAILURE: return BatteryConst::HEALTH_UNSPEC_FAILURE;
        case BatteryHealth::COLD: return BatteryConst::HEALTH_COLD;
        default: return BatteryConst::HEALTH_UNKNOWN;
    }
}

BatteryStatus stringToStatus(const std::string& str) {
    if (str == BatteryConst::STATUS_CHARGING) return BatteryStatus::CHARGING;
    if (str == BatteryConst::STATUS_DISCHARGING) return BatteryStatus::DISCHARGING;
    if (str == BatteryConst::STATUS_FULL) return BatteryStatus::FULL;
    if (str == BatteryConst::STATUS_NOT_CHARGING) return BatteryStatus::NOT_CHARGING;
    return BatteryStatus::UNKNOWN;
}

} // anonymous namespace

// ============================================================================
// BatteryMaskingManager Implementation
// ============================================================================

BatteryMaskingManager& BatteryMaskingManager::getInstance() {
    static BatteryMaskingManager instance;
    return instance;
}

BatteryMaskingManager::BatteryMaskingManager()
    : m_temperatureDist(32.0f, 1.0f)
    , m_fluctuationDist(-1.0f, 1.0f)
{
    // Initialize sysfs paths
    m_sysfsPaths.powerSupplyPath = BatteryConst::DEFAULT_POWER_SUPPLY_PATH;
    m_sysfsPaths.batteryPath = BatteryConst::DEFAULT_POWER_SUPPLY_PATH + 
                               std::string("/") + BatteryConst::BATTERY_DIR;
    
    // Default CPU load config
    m_cpuConfig = {
        .baseLoad = 0.2f,
        .peakLoad = 0.8f,
        .busyDurationSeconds = 120,
        .idleDurationSeconds = 300,
        .loadChangeSpeed = 0.1f
    };
}

BatteryMaskingManager::~BatteryMaskingManager() {
    shutdown();
}

// ============================================================================
// Initialization
// ============================================================================

bool BatteryMaskingManager::initialize(const BatteryConfig& config) {
    if (m_initialized) {
        Logger::getInstance().warning("BatteryMaskingManager already initialized");
        return true;
    }
    
    m_config = config;
    
    // Initialize state
    m_currentState.percentage = m_config.initialLevel;
    m_currentState.status = m_config.defaultStatus;
    m_currentState.health = m_config.defaultHealth;
    m_currentState.powerSource = m_config.defaultPowerSource;
    m_currentState.temperature = m_config.baseTemperature;
    m_currentState.voltage = m_config.baseVoltage;
    m_currentState.current = m_config.baseCurrent;
    m_currentState.timestamp = std::chrono::steady_clock::now();
    
    // Find battery sysfs path
    findBatteryPath();
    
    m_initialized = true;
    
    Logger::getInstance().info("BatteryMaskingManager initialized");
    Logger::getInstance().debug("Battery path: " + m_sysfsPaths.batteryPath);
    
    return true;
}

bool BatteryMaskingManager::shutdown() {
    if (!m_initialized) {
        return true;
    }
    
    // Stop simulation
    stopSimulation();
    
    m_initialized = false;
    
    Logger::getInstance().info("BatteryMaskingManager shutdown complete");
    
    return true;
}

bool BatteryMaskingManager::isInitialized() const {
    return m_initialized;
}

// ============================================================================
// Configuration
// ============================================================================

void BatteryMaskingManager::setConfig(const BatteryConfig& config) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_config = config;
}

BatteryConfig BatteryMaskingManager::getConfig() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_config;
}

void BatteryMaskingManager::setBatteryLevel(int percentage) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.percentage = std::clamp(percentage, 0, 100);
    m_currentState.level = m_currentState.percentage * 40; // Estimate: 4000mAh total
}

void BatteryMaskingManager::setBatteryStatus(BatteryStatus status) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = status;
    m_currentState.isCharging = (status == BatteryStatus::CHARGING);
    m_currentState.isFull = (status == BatteryStatus::FULL);
}

void BatteryMaskingManager::setCharging(bool charging) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (charging) {
        m_currentState.status = BatteryStatus::CHARGING;
        m_currentState.isCharging = true;
        m_currentState.powerSource = PowerSource::USB;
        m_currentState.current = m_config.chargeCurrent;
    } else {
        m_currentState.status = BatteryStatus::DISCHARGING;
        m_currentState.isCharging = false;
        m_currentState.powerSource = PowerSource::BATTERY;
        m_currentState.current = m_config.baseCurrent;
    }
}

void BatteryMaskingManager::setTemperature(float celsius) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.temperature = std::clamp(celsius, m_config.minTemperature, m_config.maxTemperature);
}

// ============================================================================
// Dynamic Simulation
// ============================================================================

void BatteryMaskingManager::startSimulation() {
    if (m_simulationRunning.load()) {
        Logger::getInstance().warning("Simulation already running");
        return;
    }
    
    m_simulationRunning.store(true);
    m_simulationPaused.store(false);
    
    m_stats.simulationStartTime = time(nullptr);
    
    m_simulationThread = std::thread(&BatteryMaskingManager::simulationLoop, this);
    
    Logger::getInstance().info("Battery simulation started");
}

void BatteryMaskingManager::stopSimulation() {
    if (!m_simulationRunning.load()) {
        return;
    }
    
    m_simulationRunning.store(false);
    
    if (m_simulationThread.joinable()) {
        m_simulationThread.join();
    }
    
    Logger::getInstance().info("Battery simulation stopped");
}

bool BatteryMaskingManager::isSimulationRunning() const {
    return m_simulationRunning.load() && !m_simulationPaused.load();
}

void BatteryMaskingManager::pauseSimulation() {
    m_simulationPaused.store(true);
    Logger::getInstance().debug("Battery simulation paused");
}

void BatteryMaskingManager::resumeSimulation() {
    m_simulationPaused.store(false);
    Logger::getInstance().debug("Battery simulation resumed");
}

void BatteryMaskingManager::setSimulatedCPULoad(float load) {
    m_targetCPULoad.store(std::clamp(load, 0.0f, 1.0f));
}

float BatteryMaskingManager::getSimulatedCPULoad() const {
    return m_currentCPULoad.load();
}

void BatteryMaskingManager::simulationLoop() {
    using namespace std::chrono;
    
    Logger::getInstance().debug("Battery simulation loop started");
    
    while (m_simulationRunning.load()) {
        if (!m_simulationPaused.load()) {
            // Update CPU load towards target (smooth transition)
            float currentLoad = m_currentCPULoad.load();
            float targetLoad = m_targetCPULoad.load();
            float loadDiff = targetLoad - currentLoad;
            
            if (std::abs(loadDiff) > 0.01f) {
                currentLoad += loadDiff * m_cpuConfig.loadChangeSpeed;
                m_currentCPULoad.store(currentLoad);
            }
            
            // Update battery components
            updateBatteryDecay();
            updateTemperature();
            updateCurrent();
            
            // Add realistic fluctuations
            if (m_config.addRealisticFluctuations) {
                addFluctuations();
            }
            
            // Auto-manage status based on level
            autoManageStatus(m_config.chargeThresholdHigh, m_config.chargeThresholdLow);
            
            // Apply to sysfs
            applyToSysfs();
            
            // Update state flags
            {
                std::lock_guard<std::mutex> lock(m_stateMutex);
                m_currentState.isLow = (m_currentState.percentage <= m_config.chargeThresholdLow);
                m_currentState.isOverheating = (m_currentState.temperature > 35.0f);
            }
        }
        
        // Sleep for simulation interval
        std::this_thread::sleep_for(milliseconds(BatteryConst::DEFAULT_SIMULATION_INTERVAL_MS));
    }
    
    Logger::getInstance().debug("Battery simulation loop ended");
}

void BatteryMaskingManager::updateBatteryDecay() {
    if (m_currentState.isCharging) {
        // Don't decay while charging
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    // Calculate decay based on CPU load
    float decayRate = calculateDecayRate();
    
    // Convert hourly rate to per-interval rate
    float intervalHours = static_cast<float>(BatteryConst::DEFAULT_SIMULATION_INTERVAL_MS) / 3600000.0f;
    float decayAmount = decayRate * intervalHours;
    
    // Apply decay
    m_currentState.percentage = std::max(0, m_currentState.percentage - static_cast<int>(std::round(decayAmount)));
    
    // Update level estimate
    m_currentState.level = m_currentState.percentage * 40; // 4000mAh total
    
    m_stats.totalDecayEvents++;
}

float BatteryMaskingManager::calculateDecayRate() const {
    // Base decay rate
    float baseRate = m_config.decayRatePerHour;
    
    if (m_config.dynamicDecayBasedOnLoad) {
        // Adjust decay based on CPU load
        float loadFactor = m_currentCPULoad.load();
        // Higher load = faster drain
        float adjustedRate = baseRate * (1.0f + loadFactor * 1.5f);
        
        // Add variance
        std::uniform_real_distribution<float> varianceDist(-m_config.decayVariance, m_config.decayVariance);
        float variance = varianceDist(m_randomEngine);
        
        return std::max(0.5f, adjustedRate + variance);
    }
    
    return baseRate;
}

void BatteryMaskingManager::updateTemperature() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    // Base temperature
    float baseTemp = m_config.baseTemperature;
    
    // Adjust based on CPU load
    float loadImpact = m_currentCPULoad.load() * m_config.loadImpactOnTemp;
    
    // Generate temperature variation
    float variation = m_temperatureDist(m_randomEngine) - 32.0f; // Centered around 0
    variation = std::clamp(variation, -2.0f, 2.0f);
    
    // Calculate new temperature
    float newTemp = baseTemp + loadImpact + variation;
    
    // Clamp to configured range
    m_currentState.temperature = std::clamp(newTemp, m_config.minTemperature, m_config.maxTemperature);
}

void BatteryMaskingManager::updateCurrent() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    if (m_currentState.isCharging) {
        // Charging current with variation
        std::uniform_int_distribution<int> currentVar(-100, 100);
        m_currentState.current = m_config.chargeCurrent + currentVar(m_randomEngine);
    } else {
        // Discharge current varies with load
        float loadFactor = m_currentCPULoad.load();
        int baseDischarge = m_config.baseCurrent;
        int maxDischarge = -1500; // Maximum discharge rate
        
        // Interpolate based on load
        int newCurrent = static_cast<int>(baseDischarge * (1.0f + loadFactor * 2.0f));
        newCurrent = std::max(newCurrent, maxDischarge);
        
        // Add small random variation
        std::uniform_int_distribution<int> currentVar(-50, 50);
        m_currentState.current = newCurrent + currentVar(m_randomEngine);
    }
}

void BatteryMaskingManager::addFluctuations() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    // Small voltage fluctuation
    m_currentState.voltage += static_cast<int>(generateFluctuation(m_config.voltageVariance));
    m_currentState.voltage = std::clamp(m_currentState.voltage, 
                                        m_config.baseVoltage - m_config.voltageVariance * 2,
                                        m_config.baseVoltage + m_config.voltageVariance * 2);
    
    // Small temperature fluctuation
    m_currentState.temperature += generateFluctuation(0.3f);
    m_currentState.temperature = std::clamp(m_currentState.temperature, 
                                            m_config.minTemperature, 
                                            m_config.maxTemperature);
}

float BatteryMaskingManager::generateFluctuation(float magnitude) {
    return m_fluctuationDist(m_randomEngine) * magnitude;
}

// ============================================================================
// State Queries
// ============================================================================

BatteryState BatteryMaskingManager::getCurrentState() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_currentState;
}

int BatteryMaskingManager::getBatteryPercentage() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_currentState.percentage;
}

float BatteryMaskingManager::getTemperature() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_currentState.temperature;
}

BatteryStatus BatteryMaskingManager::getStatus() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_currentState.status;
}

PowerSource BatteryMaskingManager::getPowerSource() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_currentState.powerSource;
}

// ============================================================================
// Sysfs Mocking
// ============================================================================

bool BatteryMaskingManager::applyToSysfs() {
    // This would write to actual sysfs on a rooted device
    // For simulation, we just update our internal state
    return true;
}

std::string BatteryMaskingManager::mockSysfsRead(const std::string& property) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    if (property == "status") {
        return statusToString(m_currentState.status);
    }
    else if (property == "capacity" || property == "level") {
        return std::to_string(m_currentState.percentage);
    }
    else if (property == "health") {
        return healthToString(m_currentState.health);
    }
    else if (property == "temp" || property == "temperature") {
        // Temperature might be in deci-degrees (e.g., 320 = 32.0°C)
        return std::to_string(static_cast<int>(m_currentState.temperature * 10));
    }
    else if (property == "voltage_now") {
        return std::to_string(m_currentState.voltage * 1000); // Convert to microvolts
    }
    else if (property == "current_now") {
        return std::to_string(m_currentState.current * 1000); // Convert to microamps
    }
    else if (property == "technology") {
        return BatteryConst::TECH_LI_ION;
    }
    else if (property == "capacity_level") {
        if (m_currentState.percentage >= 100) return "Full";
        if (m_currentState.percentage <= 5) return "Critical";
        if (m_currentState.percentage <= 20) return "Low";
        return "Normal";
    }
    
    return "";
}

std::map<std::string, std::string> BatteryMaskingManager::getAllSysfsMocks() const {
    std::map<std::string, std::string> mocks;
    
    mocks["status"] = statusToString(m_currentState.status);
    mocks["capacity"] = std::to_string(m_currentState.percentage);
    mocks["health"] = healthToString(m_currentState.health);
    mocks["temp"] = std::to_string(static_cast<int>(m_currentState.temperature * 10));
    mocks["voltage_now"] = std::to_string(m_currentState.voltage * 1000);
    mocks["current_now"] = std::to_string(m_currentState.current * 1000);
    mocks["technology"] = BatteryConst::TECH_LI_ION;
    mocks["capacity_level"] = mockSysfsRead("capacity_level");
    
    return mocks;
}

bool BatteryMaskingManager::findBatteryPath() {
    // Try common battery paths
    std::vector<std::string> possiblePaths = {
        BatteryConst::DEFAULT_POWER_SUPPLY_PATH + "/battery",
        BatteryConst::DEFAULT_POWER_SUPPLY_PATH + "/battery_power_supply",
        BatteryConst::DEFAULT_POWER_SUPPLY_PATH + "/main-battery",
        BatteryConst::DEFAULT_POWER_SUPPLY_PATH + "/sc27xx-battery",
        BatteryConst::DEFAULT_POWER_SUPPLY_PATH + "/bq27500-0",
    };
    
    for (const auto& path : possiblePaths) {
        if (sysfsExists(path)) {
            m_sysfsPaths.batteryPath = path;
            m_sysfsPathFound = true;
            
            // Update other paths
            m_sysfsPaths.statusPath = path + "/status";
            m_sysfsPaths.capacityPath = path + "/capacity";
            m_sysfsPaths.healthPath = path + "/health";
            m_sysfsPaths.tempPath = path + "/temp";
            m_sysfsPaths.voltagePath = path + "/voltage_now";
            m_sysfsPaths.currentPath = path + "/current_now";
            m_sysfsPaths.technologyPath = path + "/technology";
            
            return true;
        }
    }
    
    // On desktop/without root, we can't find real sysfs
    m_sysfsPathFound = false;
    Logger::getInstance().warning("Battery sysfs path not found - running in simulation mode");
    
    return false;
}

// ============================================================================
// Status State Management
// ============================================================================

void BatteryMaskingManager::enterChargingState() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = BatteryStatus::CHARGING;
    m_currentState.isCharging = true;
    m_currentState.isFull = false;
    m_currentState.powerSource = PowerSource::USB;
    m_currentState.current = m_config.chargeCurrent;
}

void BatteryMaskingManager::enterDischargingState() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = BatteryStatus::DISCHARGING;
    m_currentState.isCharging = false;
    m_currentState.isFull = false;
    m_currentState.powerSource = PowerSource::BATTERY;
    m_currentState.current = m_config.baseCurrent;
}

void BatteryMaskingManager::enterFullState() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = BatteryStatus::FULL;
    m_currentState.isCharging = false;
    m_currentState.isFull = true;
    m_currentState.percentage = 100;
    m_currentState.level = 4000;
}

void BatteryMaskingManager::enterNotChargingState() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = BatteryStatus::NOT_CHARGING;
    m_currentState.isCharging = false;
    m_currentState.isFull = false;
}

void BatteryMaskingManager::autoManageStatus(int thresholdFull, int thresholdLow) {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    
    int percentage = m_currentState.percentage;
    
    // Don't change status if explicitly in charging
    if (m_currentState.status == BatteryStatus::CHARGING) {
        if (percentage >= thresholdFull) {
            m_currentState.status = BatteryStatus::FULL;
            m_currentState.isFull = true;
            m_currentState.isCharging = false;
        }
        return;
    }
    
    // Auto-manage based on percentage
    if (percentage >= thresholdFull && !m_currentState.isCharging) {
        m_currentState.status = BatteryStatus::FULL;
        m_currentState.isFull = true;
    } else if (percentage <= thresholdLow) {
        // Low battery - still discharging unless plugged in
        m_currentState.isLow = true;
    }
}

// ============================================================================
// Presets
// ============================================================================

void BatteryMaskingManager::presetFullyCharged() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.percentage = 100;
    m_currentState.level = 4000;
    m_currentState.status = BatteryStatus::FULL;
    m_currentState.isCharging = false;
    m_currentState.isFull = true;
    m_currentState.temperature = 30.0f; // Cool when full
    m_currentState.current = 0;
    m_currentState.voltage = 4350; // Full voltage
}

void BatteryMaskingManager::presetNewPhone() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.percentage = 90;
    m_currentState.level = 3600;
    m_currentState.status = BatteryStatus::DISCHARGING;
    m_currentState.isCharging = false;
    m_currentState.isFull = false;
    m_currentState.health = BatteryHealth::GOOD;
    m_currentState.temperature = 28.0f;
    m_currentState.current = -300;
    m_currentState.voltage = 4200;
}

void BatteryMaskingManager::presetLowBattery() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.percentage = 15;
    m_currentState.level = 600;
    m_currentState.status = BatteryStatus::DISCHARGING;
    m_currentState.isCharging = false;
    m_currentState.isFull = false;
    m_currentState.isLow = true;
    m_currentState.temperature = 30.0f;
    m_currentState.current = -800;
    m_currentState.voltage = 3700;
}

void BatteryMaskingManager::presetCharging() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = BatteryStatus::CHARGING;
    m_currentState.isCharging = true;
    m_currentState.isFull = false;
    m_currentState.powerSource = PowerSource::USB;
    m_currentState.current = m_config.chargeCurrent;
    m_currentState.temperature = 33.0f; // Slightly warm while charging
}

void BatteryMaskingManager::presetStandby() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    m_currentState.status = BatteryStatus::DISCHARGING;
    m_currentState.isCharging = false;
    m_currentState.temperature = 25.0f; // Cool when idle
    m_currentState.current = -100; // Minimal drain
    
    // Slower decay in standby
    m_config.decayRatePerHour = 1.0f;
    m_cpuConfig.baseLoad = 0.05f;
}

// ============================================================================
// Raw Sysfs Access
// ============================================================================

bool BatteryMaskingManager::writeSysfs(const std::string& path, const std::string& value) {
    if (!m_sysfsPathFound) {
        Logger::getInstance().warning("Cannot write to sysfs - path not found: " + path);
        return false;
    }
    
    std::ofstream file(path);
    if (!file.is_open()) {
        Logger::getInstance().error("Failed to open sysfs for write: " + path);
        return false;
    }
    
    file << value;
    file.close();
    
    return true;
}

std::string BatteryMaskingManager::readSysfs(const std::string& path) const {
    if (!m_sysfsPathFound) {
        return "";
    }
    
    std::ifstream file(path);
    if (!file.is_open()) {
        return "";
    }
    
    std::string value;
    std::getline(file, value);
    file.close();
    
    return value;
}

bool BatteryMaskingManager::sysfsExists(const std::string& path) const {
    struct stat st;
    return (stat(path.c_str(), &st) == 0 && S_ISDIR(st.st_mode));
}

} // namespace AntiDetect