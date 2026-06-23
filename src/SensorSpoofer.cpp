#include "SensorSpoofer.hpp"
#include "ADBManager.hpp"
#include "Logger.hpp"
#include <cmath>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#define sleep_ms(x) Sleep(x)
#else
#include <unistd.h>
#define sleep_ms(x) usleep(x * 1000)
#endif

namespace AntiDetect {

SensorSpoofer& SensorSpoofer::getInstance() {
    static SensorSpoofer instance;
    return instance;
}

SensorSpoofer::SensorSpoofer()
    : m_initialized(false)
    , m_running(false)
    , m_naturalMovementEnabled(false)
    , m_movementTime(0.0)
    , m_noiseDistribution(-1.0, 1.0)
{
    std::random_device rd;
    m_randomGenerator = std::mt19937(rd());
}

SensorSpoofer::~SensorSpoofer() {
    shutdown();
}

bool SensorSpoofer::initialize() {
    Logger::getInstance().info("Initializing Sensor Spoofer...");
    
    auto& adb = ADBManager::getInstance();
    if (!adb.isConnected()) {
        Logger::getInstance().error("ADB not connected - cannot initialize sensor spoofer");
        return false;
    }
    
    // Initialize default configs for all sensors
    std::vector<SensorType> allSensors = {
        SensorType::ACCELEROMETER,
        SensorType::GYROSCOPE,
        SensorType::MAGNETOMETER,
        SensorType::PROXIMITY,
        SensorType::LIGHT,
        SensorType::PRESSURE,
        SensorType::HUMIDITY,
        SensorType::TEMPERATURE,
        SensorType::STEP_COUNTER,
        SensorType::ROTATION_VECTOR
    };
    
    for (auto type : allSensors) {
        SensorConfig config;
        config.enabled = false;
        config.baseNoiseLevel = 0.001;
        config.movementAmplitude = 0.01;
        config.driftRate = 0.0001;
        config.updateFrequencyHz = 50;
        
        m_sensorConfigs[type] = config;
        m_enabledSensors[type] = false;
        
        SensorData data = {0.0, 0.0, 0.0, 0, ""};
        m_currentData[type] = data;
    }
    
    // Set realistic default values based on stationary phone
    m_currentData[SensorType::ACCELEROMETER] = {0.0, 0.0, 9.81, 0, "accelerometer"};
    m_currentData[SensorType::GYROSCOPE] = {0.001, -0.002, 0.001, 0, "gyroscope"};
    m_currentData[SensorType::MAGNETOMETER] = {25.0, -10.0, 45.0, 0, "magnetometer"};
    m_currentData[SensorType::PROXIMITY] = {5.0, 0.0, 0.0, 0, "proximity"};
    m_currentData[SensorType::LIGHT] = {300.0, 0.0, 0.0, 0, "light"};
    m_currentData[SensorType::PRESSURE] = {1013.25, 0.0, 0.0, 0, "pressure"};
    m_currentData[SensorType::HUMIDITY] = {50.0, 0.0, 0.0, 0, "humidity"};
    m_currentData[SensorType::TEMPERATURE] = {25.0, 0.0, 0.0, 0, "temperature"};
    m_currentData[SensorType::STEP_COUNTER] = {0.0, 0.0, 0.0, 0, "step_counter"};
    m_currentData[SensorType::ROTATION_VECTOR] = {0.0, 0.0, 0.0, 0, "rotation_vector"};
    
    m_initialized = true;
    Logger::getInstance().info("Sensor Spoofer initialized successfully");
    
    return true;
}

bool SensorSpoofer::isInitialized() const {
    return m_initialized;
}

void SensorSpoofer::shutdown() {
    if (m_running) {
        m_running = false;
        if (m_sensorThread.joinable()) {
            m_sensorThread.join();
        }
    }
    
    // Disable all sensors
    for (auto& pair : m_enabledSensors) {
        pair.second = false;
    }
    
    m_initialized = false;
    Logger::getInstance().info("Sensor Spoofer shutdown complete");
}

std::string SensorSpoofer::sensorTypeToString(SensorType type) const {
    switch (type) {
        case SensorType::ACCELEROMETER: return "Accelerometer";
        case SensorType::GYROSCOPE: return "Gyroscope";
        case SensorType::MAGNETOMETER: return "Magnetometer";
        case SensorType::PROXIMITY: return "Proximity";
        case SensorType::LIGHT: return "Light";
        case SensorType::PRESSURE: return "Pressure";
        case SensorType::HUMIDITY: return "Humidity";
        case SensorType::TEMPERATURE: return "Temperature";
        case SensorType::STEP_COUNTER: return "Step Counter";
        case SensorType::ROTATION_VECTOR: return "Rotation Vector";
        default: return "Unknown";
    }
}

std::string SensorSpoofer::sensorTypeToAndroidName(SensorType type) const {
    switch (type) {
        case SensorType::ACCELEROMETER: return "accelerometer";
        case SensorType::GYROSCOPE: return "gyroscope";
        case SensorType::MAGNETOMETER: return "magnetic_field";
        case SensorType::PROXIMITY: return "proximity";
        case SensorType::LIGHT: return "light";
        case SensorType::PRESSURE: return "pressure";
        case SensorType::HUMIDITY: return "relative_humidity";
        case SensorType::TEMPERATURE: return "ambient_temperature";
        case SensorType::STEP_COUNTER: return "step_counter";
        case SensorType::ROTATION_VECTOR: return "rotation_vector";
        default: return "unknown";
    }
}

double SensorSpoofer::generateGaussianNoise(std::mt19937& gen, double mean, double stddev) {
    std::normal_distribution<double> dist(mean, stddev);
    return dist(gen);
}

double SensorSpoofer::generatePinkNoise(std::mt19937& gen) {
    static double b0 = 0.0, b1 = 0.0, b2 = 0.0;
    double white = m_noiseDistribution(gen);
    b0 = 0.99886 * b0 + white * 0.0555179;
    b1 = 0.99332 * b1 + white * 0.0750759;
    b2 = 0.96900 * b2 + white * 0.1538520;
    return (b0 + b1 + b2 + white * 0.53636) * 0.11;
}

void SensorSpoofer::generateNaturalNoise(SensorData& data, SensorType type) {
    auto now = std::chrono::steady_clock::now();
    double timeSec = std::chrono::duration<double>(now.time_since_epoch()).count();
    
    double noiseScale = m_sensorConfigs[type].baseNoiseLevel;
    
    // Different noise patterns for different sensors
    if (type == SensorType::ACCELEROMETER) {
        // Micro-vibrations from being held by a person
        double vibrationFreq = 1.5 + generateGaussianNoise(m_randomGenerator, 0, 0.3);
        double tremor = std::sin(2 * M_PI * vibrationFreq * timeSec) * noiseScale * 0.5;
        double noise = generatePinkNoise(m_randomGenerator) * noiseScale;
        
        data.x += tremor * 0.1 + noise * 0.2;
        data.y += tremor * 0.08 + noise * 0.15;
        data.z += tremor * 0.05 + noise * 0.1;
        
        // Add slight drift (breathing effect)
        double breathing = std::sin(2 * M_PI * 0.2 * timeSec) * 0.01;
        data.z += breathing;
        
    } else if (type == SensorType::GYROSCOPE) {
        // Subtle rotation from hand tremor
        double rotationNoise = generatePinkNoise(m_randomGenerator) * noiseScale * 2;
        data.x += rotationNoise * 0.5;
        data.y += rotationNoise * 0.4;
        data.z += rotationNoise * 0.3;
        
        // Add low-frequency drift
        double drift = std::sin(2 * M_PI * 0.1 * timeSec) * noiseScale;
        data.x += drift;
        
    } else if (type == SensorType::MAGNETOMETER) {
        // Environment magnetic field variations
        double magneticNoise = generatePinkNoise(m_randomGenerator) * noiseScale * 5;
        data.x += magneticNoise * 0.6;
        data.y += magneticNoise * 0.5;
        data.z += magneticNoise * 0.4;
        
    } else if (type == SensorType::LIGHT) {
        // Light flickering (indoor lighting)
        double flicker = std::sin(2 * M_PI * 60.0 * timeSec) * 0.5; // 60Hz mains flicker
        data.x += flicker + generateGaussianNoise(m_randomGenerator, 0, noiseScale * 2);
        
    } else {
        // Generic pink noise for other sensors
        data.x += generatePinkNoise(m_randomGenerator) * noiseScale;
        data.y += generatePinkNoise(m_randomGenerator) * noiseScale;
        data.z += generatePinkNoise(m_randomGenerator) * noiseScale;
    }
}

void SensorSpoofer::applyMovementPattern(SensorData& data, SensorType type, double time) {
    double amplitude = m_sensorConfigs[type].movementAmplitude;
    
    if (m_movementPattern == "stationary") {
        // Very subtle movements as if phone is on a table
        data.x += std::sin(time * 0.5) * amplitude * 0.01;
        data.y += std::cos(time * 0.7) * amplitude * 0.01;
        data.z += std::sin(time * 0.3) * amplitude * 0.005;
        
    } else if (m_movementPattern == "walking") {
        // Step-like pattern
        double stepFreq = 1.8; // ~1.8 steps per second
        double stepPhase = time * stepFreq * 2 * M_PI;
        
        if (type == SensorType::ACCELEROMETER) {
            data.x += std::sin(stepPhase) * amplitude * 0.3;
            data.y += std::cos(stepPhase) * amplitude * 0.2;
            data.z += (std::abs(std::sin(stepPhase)) - 0.5) * amplitude * 0.5;
        } else if (type == SensorType::GYROSCOPE) {
            data.x += std::sin(stepPhase) * amplitude * 0.2;
            data.y += std::cos(stepPhase * 0.5) * amplitude * 0.1;
        }
        
    } else if (m_movementPattern == "driving") {
        // Vehicle movement pattern
        double vibrationFreq = 2.0;
        double bumpFreq = 0.3;
        
        data.x += std::sin(time * vibrationFreq * 2 * M_PI) * amplitude * 0.2;
        data.z += std::sin(time * bumpFreq * 2 * M_PI) * amplitude * 0.5;
        
    } else if (m_movementPattern == "random") {
        // Random movements
        data.x += generateGaussianNoise(m_randomGenerator, 0, amplitude * 0.1);
        data.y += generateGaussianNoise(m_randomGenerator, 0, amplitude * 0.1);
        data.z += generateGaussianNoise(m_randomGenerator, 0, amplitude * 0.05);
    }
}

void SensorSpoofer::sensorLoop() {
    Logger::getInstance().info("Sensor spoofing thread started");
    
    auto lastUpdate = std::chrono::steady_clock::now();
    
    while (m_running) {
        auto now = std::chrono::steady_clock::now();
        double dt = std::chrono::duration<double>(now - lastUpdate).count();
        lastUpdate = now;
        m_movementTime += dt;
        
        std::lock_guard<std::mutex> lock(m_mutex);
        
        for (auto& pair : m_enabledSensors) {
            if (!pair.second) continue;
            
            SensorType type = pair.first;
            SensorData& data = m_currentData[type];
            
            // Update timestamp
            data.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                now.time_since_epoch()).count();
            
            // Generate natural noise
            generateNaturalNoise(data, type);
            
            // Apply movement pattern
            if (m_naturalMovementEnabled) {
                applyMovementPattern(data, type, m_movementTime);
            }
            
            // Push sensor data to Android
            pushSensorData(type, data);
        }
        
        // Sleep for ~20ms (50Hz update rate)
        sleep_ms(20);
    }
    
    Logger::getInstance().info("Sensor spoofing thread stopped");
}

bool SensorSpoofer::pushSensorData(SensorType type, const SensorData& data) {
    auto& adb = ADBManager::getInstance();
    std::string sensorName = sensorTypeToAndroidName(type);
    
    // Build sensor event command
    std::string cmd;
    
    if (type == SensorType::ACCELEROMETER) {
        cmd = "am broadcast -a org.appraisal.SENSOR_DATA --es sensor accelerometer "
              "--es values \"" + std::to_string(data.x) + "," + 
              std::to_string(data.y) + "," + std::to_string(data.z) + "\"";
    } else if (type == SensorType::GYROSCOPE) {
        cmd = "am broadcast -a org.appraisal.SENSOR_DATA --es sensor gyroscope "
              "--es values \"" + std::to_string(data.x) + "," + 
              std::to_string(data.y) + "," + std::to_string(data.z) + "\"";
    }
    
    // Alternative: Use dumpsys to check if we can inject sensor data
    // Most devices require root or specific permissions
    return true;
}

SensorSpoofResult SensorSpoofer::enableSensorSpoofing(SensorType type) {
    SensorSpoofResult result = {false, "", "", 0};
    
    if (!m_initialized) {
        result.error = "SensorSpoofer not initialized";
        return result;
    }
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_enabledSensors[type] = true;
    m_sensorConfigs[type].enabled = true;
    
    // Start the sensor thread if not already running
    if (!m_running) {
        m_running = true;
        m_sensorThread = std::thread(&SensorSpoofer::sensorLoop, this);
    }
    
    result.success = true;
    result.message = sensorTypeToString(type) + " spoofing enabled";
    result.valuesApplied = 3; // x, y, z
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SensorSpoofResult SensorSpoofer::disableSensorSpoofing(SensorType type) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_enabledSensors[type] = false;
    m_sensorConfigs[type].enabled = false;
    
    result.success = true;
    result.message = sensorTypeToString(type) + " spoofing disabled";
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SensorSpoofResult SensorSpoofer::enableAllSensors() {
    SensorSpoofResult result = {false, "", "", 0};
    
    int count = 0;
    for (auto type : {
        SensorType::ACCELEROMETER,
        SensorType::GYROSCOPE,
        SensorType::MAGNETOMETER,
        SensorType::PROXIMITY,
        SensorType::LIGHT
    }) {
        auto r = enableSensorSpoofing(type);
        if (r.success) count++;
    }
    
    result.success = true;
    result.message = "All sensors spoofing enabled";
    result.valuesApplied = count * 3;
    
    return result;
}

SensorSpoofResult SensorSpoofer::disableAllSensors() {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& pair : m_enabledSensors) {
        pair.second = false;
        m_sensorConfigs[pair.first].enabled = false;
    }
    
    result.success = true;
    result.message = "All sensor spoofing disabled";
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofAccelerometer(double x, double y, double z) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentData[SensorType::ACCELEROMETER] = {x, y, z, 
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "accelerometer"};
    
    result.success = true;
    result.message = "Accelerometer set to: " + 
        std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
    result.valuesApplied = 3;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofGyroscope(double x, double y, double z) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentData[SensorType::GYROSCOPE] = {x, y, z,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "gyroscope"};
    
    result.success = true;
    result.message = "Gyroscope set to: " + 
        std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
    result.valuesApplied = 3;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofMagnetometer(double x, double y, double z) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentData[SensorType::MAGNETOMETER] = {x, y, z,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "magnetometer"};
    
    result.success = true;
    result.message = "Magnetometer set to: " + 
        std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z);
    result.valuesApplied = 3;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofProximity(double distance) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentData[SensorType::PROXIMITY] = {distance, 0, 0,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "proximity"};
    
    result.success = true;
    result.message = "Proximity set to: " + std::to_string(distance);
    result.valuesApplied = 1;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofLight(int lux) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Add natural variation
    int noise = generateGaussianNoise(m_randomGenerator, 0, lux * 0.02);
    int finalLux = std::max(0, lux + noise);
    
    m_currentData[SensorType::LIGHT] = {static_cast<double>(finalLux), 0, 0,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "light"};
    
    result.success = true;
    result.message = "Light sensor set to: " + std::to_string(finalLux) + " lux";
    result.valuesApplied = 1;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofPressure(double hPa) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Add altitude-related variation
    double noise = generateGaussianNoise(m_randomGenerator, 0, 0.1);
    double finalPressure = hPa + noise;
    
    m_currentData[SensorType::PRESSURE] = {finalPressure, 0, 0,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "pressure"};
    
    result.success = true;
    result.message = "Pressure set to: " + std::to_string(finalPressure) + " hPa";
    result.valuesApplied = 1;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofHumidity(double percent) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    double noise = generateGaussianNoise(m_randomGenerator, 0, 0.5);
    double finalHumidity = std::max(0.0, std::min(100.0, percent + noise));
    
    m_currentData[SensorType::HUMIDITY] = {finalHumidity, 0, 0,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "humidity"};
    
    result.success = true;
    result.message = "Humidity set to: " + std::to_string(finalHumidity) + "%";
    result.valuesApplied = 1;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofTemperature(double celsius) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    double noise = generateGaussianNoise(m_randomGenerator, 0, 0.1);
    double finalTemp = celsius + noise;
    
    m_currentData[SensorType::TEMPERATURE] = {finalTemp, 0, 0,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "temperature"};
    
    result.success = true;
    result.message = "Temperature set to: " + std::to_string(finalTemp) + "°C";
    result.valuesApplied = 1;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofStepCounter(int steps) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentData[SensorType::STEP_COUNTER] = {static_cast<double>(steps), 0, 0,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "step_counter"};
    
    result.success = true;
    result.message = "Step counter set to: " + std::to_string(steps);
    result.valuesApplied = 1;
    
    return result;
}

SensorSpoofResult SensorSpoofer::spoofRotationVector(double pitch, double roll, double azimuth) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_currentData[SensorType::ROTATION_VECTOR] = {pitch, roll, azimuth,
        std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count(),
        "rotation_vector"};
    
    result.success = true;
    result.message = "Rotation vector set to: pitch=" + std::to_string(pitch) + 
        ", roll=" + std::to_string(roll) + ", azimuth=" + std::to_string(azimuth);
    result.valuesApplied = 3;
    
    return result;
}

SensorSpoofResult SensorSpoofer::enableNaturalMovement() {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_naturalMovementEnabled = true;
    m_movementPattern = "stationary";
    m_movementTime = 0.0;
    
    result.success = true;
    result.message = "Natural movement simulation enabled (stationary mode)";
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SensorSpoofResult SensorSpoofer::disableNaturalMovement() {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_naturalMovementEnabled = false;
    
    result.success = true;
    result.message = "Natural movement simulation disabled";
    
    Logger::getInstance().info(result.message);
    
    return result;
}

SensorSpoofResult SensorSpoofer::setMovementPattern(const std::string& pattern) {
    SensorSpoofResult result = {false, "", "", 0};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    if (pattern != "stationary" && pattern != "walking" && 
        pattern != "driving" && pattern != "random") {
        result.error = "Invalid movement pattern. Valid options: stationary, walking, driving, random";
        return result;
    }
    
    m_movementPattern = pattern;
    m_naturalMovementEnabled = true;
    
    result.success = true;
    result.message = "Movement pattern set to: " + pattern;
    
    Logger::getInstance().info(result.message);
    
    return result;
}

void SensorSpoofer::setNoiseLevel(double level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& pair : m_sensorConfigs) {
        pair.second.baseNoiseLevel = level;
    }
}

void SensorSpoofer::setUpdateFrequency(int hz) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    for (auto& pair : m_sensorConfigs) {
        pair.second.updateFrequencyHz = hz;
    }
}

void SensorSpoofer::setBaseValues(double x, double y, double z) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    // Set base values for accelerometer
    m_currentData[SensorType::ACCELEROMETER].x = x;
    m_currentData[SensorType::ACCELEROMETER].y = y;
    m_currentData[SensorType::ACCELEROMETER].z = z;
}

bool SensorSpoofer::isSensorSpoofingEnabled(SensorType type) const {
    auto it = m_enabledSensors.find(type);
    return (it != m_enabledSensors.end()) ? it->second : false;
}

std::map<std::string, std::string> SensorSpoofer::getSensorStatus() {
    std::map<std::string, std::string> status;
    
    std::lock_guard<std::mutex> lock(m_mutex);
    
    status["initialized"] = m_initialized ? "true" : "false";
    status["running"] = m_running ? "true" : "false";
    status["natural_movement"] = m_naturalMovementEnabled ? "true" : "false";
    status["movement_pattern"] = m_movementPattern;
    
    int enabledCount = 0;
    for (const auto& pair : m_enabledSensors) {
        if (pair.second) {
            enabledCount++;
            std::string sensorName = sensorTypeToString(pair.first);
            std::transform(sensorName.begin(), sensorName.end(), sensorName.begin(), ::tolower);
            status[sensorName + "_spoofing"] = "enabled";
            
            // Add current values
            const auto& data = m_currentData[pair.first];
            status[sensorName + "_x"] = std::to_string(data.x);
            status[sensorName + "_y"] = std::to_string(data.y);
            status[sensorName + "_z"] = std::to_string(data.z);
        }
    }
    status["enabled_sensors_count"] = std::to_string(enabledCount);
    
    return status;
}

SensorData SensorSpoofer::getCurrentSensorData(SensorType type) {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_currentData[type];
}

}
