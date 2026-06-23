#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <random>
#include <chrono>
#include <thread>
#include <mutex>

namespace AntiDetect {

struct SensorData {
    double x;
    double y;
    double z;
    long long timestamp;
    std::string sensorType;
};

struct SensorConfig {
    bool enabled;
    double baseNoiseLevel;
    double movementAmplitude;
    double driftRate;
    int updateFrequencyHz;
};

enum class SensorType {
    ACCELEROMETER,
    GYROSCOPE,
    MAGNETOMETER,
    PROXIMITY,
    LIGHT,
    PRESSURE,
    HUMIDITY,
    TEMPERATURE,
    STEP_COUNTER,
    ROTATION_VECTOR
};

struct SensorSpoofResult {
    bool success;
    std::string message;
    std::string error;
    int valuesApplied;
};

class SensorSpoofer {
public:
    static SensorSpoofer& getInstance();
    
    SensorSpoofer();
    ~SensorSpoofer();
    
    bool initialize();
    bool isInitialized() const;
    void shutdown();
    
    // Core sensor spoofing
    SensorSpoofResult enableSensorSpoofing(SensorType type);
    SensorSpoofResult disableSensorSpoofing(SensorType type);
    SensorSpoofResult enableAllSensors();
    SensorSpoofResult disableAllSensors();
    
    // Individual sensor control
    SensorSpoofResult spoofAccelerometer(double x, double y, double z);
    SensorSpoofResult spoofGyroscope(double x, double y, double z);
    SensorSpoofResult spoofMagnetometer(double x, double y, double z);
    SensorSpoofResult spoofProximity(double distance);
    SensorSpoofResult spoofLight(int lux);
    SensorSpoofResult spoofPressure(double hPa);
    SensorSpoofResult spoofHumidity(double percent);
    SensorSpoofResult spoofTemperature(double celsius);
    bool pushSensorData(SensorType type, const SensorData& data);
    SensorSpoofResult spoofStepCounter(int steps);
    SensorSpoofResult spoofRotationVector(double pitch, double roll, double azimuth);
    
    // Advanced: Natural movement simulation
    SensorSpoofResult enableNaturalMovement();
    SensorSpoofResult disableNaturalMovement();
    SensorSpoofResult setMovementPattern(const std::string& pattern);
    
    // Configuration
    void setNoiseLevel(double level);
    void setUpdateFrequency(int hz);
    void setBaseValues(double x, double y, double z);
    
    // Status
    bool isSensorSpoofingEnabled(SensorType type) const;
    std::map<std::string, std::string> getSensorStatus();
    SensorData getCurrentSensorData(SensorType type);
    
private:
    void sensorLoop();
    void generateNaturalNoise(SensorData& data, SensorType type);
    void applyMovementPattern(SensorData& data, SensorType type, double time);
    
    double generateGaussianNoise(std::mt19937& gen, double mean, double stddev);
    double generatePinkNoise(std::mt19937& gen);
    
    std::string sensorTypeToString(SensorType type) const;
    std::string sensorTypeToAndroidName(SensorType type) const;
    
    bool m_initialized;
    bool m_running;
    std::thread m_sensorThread;
    std::mutex m_mutex;
    
    std::map<SensorType, bool> m_enabledSensors;
    std::map<SensorType, SensorConfig> m_sensorConfigs;
    std::map<SensorType, SensorData> m_currentData;
    std::map<SensorType, std::vector<double>> m_movementHistory;
    
    bool m_naturalMovementEnabled;
    std::string m_movementPattern;
    double m_movementTime;
    
    std::mt19937 m_randomGenerator;
    std::uniform_real_distribution<double> m_noiseDistribution;
};

}
