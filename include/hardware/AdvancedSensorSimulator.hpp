#pragma once

/**
 * AdvancedSensorSimulator - Ultra-Realistic Sensor Simulation
 * 
 * Features:
 * - Natural sensor drift and temperature compensation
 * - Sensor fusion algorithms (accelerometer + gyroscope = orientation)
 * - Hardware calibration data simulation
 * - Realistic micro-variations
 * - Movement pattern correlation
 */

#include "../VirtualPhonePro.hpp"
#include <cmath>
#include <mutex>

namespace VirtualPhonePro {

// Sensor calibration data
struct SensorCalibration {
    std::string sensorType;
    std::array<float, 3> offset;      // Calibration offset
    std::array<float, 3> scale;      // Scale factors
    std::array<float, 9> matrix;     // Transformation matrix
    float temperatureCoeff;          // Temperature drift coefficient
    uint64_t calibrationDate;        // When calibrated
    float age;                       // Days since calibration
};

// Raw vs calibrated sensor reading
struct SensorReading {
    float x, y, z;
    uint64_t timestamp;
    float temperature;
    float confidence;                 // Data quality (0-1)
    bool isCalibrated;
};

// Movement state for correlation
struct MovementState {
    enum class Activity {
        STATIONARY,
        WALKING,
        RUNNING,
        DRIVING,
        IN_VEHICLE,
        RANDOM
    };
    Activity activity;
    float velocity;                   // m/s
    float acceleration;               // m/s²
    std::array<float, 3> heading;     // Direction
};

// Advanced sensor configuration
struct AdvancedSensorConfig {
    bool enableCalibration = true;
    bool enableTemperatureDrift = true;
    bool enableSensorFusion = true;
    bool enableMovementCorrelation = true;
    bool enableNoiseProfile = true;
    bool enableHysteresis = true;
    
    float noiseLevel = 0.001f;        // Base noise level
    float temperatureDrift = 0.0001f;  // Per-second drift
    float sensorAge = 7.0f;           // Days since last calibration
    
    MovementState::Activity defaultActivity = MovementState::Activity::STATIONARY;
};

class AdvancedSensorSimulator {
public:
    static AdvancedSensorSimulator& getInstance();
    
    // Configuration
    void configure(const AdvancedSensorConfig& config);
    void setActivity(MovementState::Activity activity);
    void setTemperature(float celsius);
    void setLocation(double lat, double lon);
    
    // Sensor data retrieval with advanced simulation
    SensorReading getAccelerometer();
    SensorReading getGyroscope();
    SensorReading getMagnetometer();
    SensorReading getGravity();
    SensorReading getLinearAcceleration();
    SensorReading getRotationVector();
    
    // Barometer
    float getPressure();
    float getAltitude();
    
    // Proximity & Light
    float getProximity();
    float getLight();
    
    // GPS with realistic drift
    std::tuple<double, double, float> getGPS();
    
    // Humidity & Temperature
    float getHumidity();
    float getAmbientTemperature();
    
    // All sensors as map
    std::map<std::string, SensorReading> getAllSensorReadings();
    
    // Sensor fusion results
    std::array<float, 4> getOrientationQuaternion();
    std::array<float, 9> getRotationMatrix();
    std::array<float, 3> getEulerAngles();
    
private:
    AdvancedSensorSimulator();
    ~AdvancedSensorSimulator();
    
    // Internal methods
    void initializeCalibration();
    void updateSensorFusion();
    void applyTemperatureDrift(SensorReading& reading);
    void applyCalibration(SensorReading& reading, const std::string& sensorType);
    void applyMovementCorrelation(SensorReading& reading);
    void applyHysteresis(SensorReading& reading, const std::string& sensorType);
    float generateRealisticNoise(float stddev);
    float generateFlickerNoise();      // 1/f noise
    float generateBurstNoise();         // Occasional spikes
    
    // Calibration helpers
    SensorCalibration getCalibration(const std::string& sensorType);
    void updateCalibrationAge();
    
    // Movement simulation
    void simulateMovement();
    std::array<float, 3> calculateTilt();
    
    // State
    AdvancedSensorConfig m_config;
    MovementState m_movementState;
    std::mutex m_mutex;
    
    // Base sensor values (before noise)
    std::map<std::string, std::array<float, 3>> m_baseValues;
    
    // Previous readings for hysteresis
    std::map<std::string, SensorReading> m_previousReadings;
    
    // Calibration data
    std::map<std::string, SensorCalibration> m_calibrations;
    
    // Time tracking
    uint64_t m_startTime;
    uint64_t m_lastUpdateTime;
    float m_currentTemperature;       // Celsius
    double m_latitude, m_longitude;
    
    // Sensor fusion state
    std::array<float, 4> m_fusedOrientation;
    std::array<float, 9> m_rotationMatrix;
    std::array<float, 3> m_fusedEuler;
    
    // Natural variation parameters
    float m_driftAccumulator;
    std::array<float, 3> m_slowDrift;
    std::array<float, 3> m_fastVariation;
};

// ============================================
// Realistic Device Properties Generator
// ============================================
class RealisticDeviceProperties {
public:
    RealisticDeviceProperties();
    
    // Generate all device properties at once
    std::map<std::string, std::string> generateAllProperties();
    
    // Individual property generators
    std::string generateBuildFingerprint();
    std::string generateBootloader();
    std::string generateHardwareHash();
    std::string generateManufacturer();
    std::string generateModel();
    std::string generateBrand();
    std::string generateDevice();
    std::string generateProduct();
    std::string generateBoard();
    std::string generateHardware();
    std::string generateRadioVersion();
    std::string generateBuildId();
    std::string generateBuildNumber();
    std::string generateBuildTags();
    std::string generateBuildDescription();
    std::string generateBuildFingerprint();
    
    // Luhn-valid identifiers
    std::string generateIMEI();
    std::string generateMEID();
    std::string generateSerialNumber();
    
    // Google-specific
    std::string generateGSFId();
    std::string generateAndroidId();
    
    // WiFi MAC (realistic OUI)
    std::string generateWiFiMAC(const std::string& manufacturer = "");
    
    // Bluetooth MAC
    std::string generateBluetoothMAC();
    
    // Netflix/DRM identifiers
    std::string generateNetflixDeviceId();
    std::string generateWidevineId();
    
    // Device-specific values
    void setDeviceProfile(const std::string& manufacturer, const std::string& model);
    
private:
    std::string generateUUID();
    std::string generateRandomHex(int length);
    int luhnChecksum(const std::string& number);
    std::string randomDigitString(int length);
    
    std::string m_manufacturer;
    std::string m_model;
    std::string m_brand;
    std::string m_device;
    std::string m_product;
    std::string m_board;
    std::string m_hardware;
    std::string m_androidVersion;
    std::string m_securityPatch;
    
    std::random_device m_rd;
    std::mt19937 m_gen;
};

// ============================================
// Realistic Timing Simulator
// ============================================
class RealisticTimingSimulator {
public:
    RealisticTimingSimulator();
    
    // Get realistic timestamps
    uint64_t getSystemUptime();        // Boot time
    uint64_t getRealTimeClock();       // Wall clock
    uint64_t getMonotonicClock();      // Monotonic
    
    // CPU timing with jitter
    uint64_t getRDTSC();               // With jitter
    uint64_t getCPUTime();             // Process CPU time
    uint64_t getThreadTime();
    
    // Sleep duration with realistic jitter
    void realisticSleep(int64_t milliseconds);
    
    // Timing profile
    void setTimingProfile(const std::string& profile);
    // Profiles: "flagship", "midrange", "budget", "gaming"
    
    // Cache timing
    int64_t measureCacheLatency();
    void flushCache();
    
    // Add execution jitter
    void addJitter(int percentage);
    
    // Simulate process scheduling
    void simulateScheduling();
    
private:
    uint64_t m_baseUptime;
    uint64_t m_bootTime;
    int m_jitterPercentage;
    std::string m_timingProfile;
    
    // Platform-specific timing
    #ifdef __linux__
    uint64_t readUptimeFromProc();
    #endif
    
    // Realistic jitter calculation
    int64_t calculateJitter(int64_t baseValue);
};

// ============================================
// Battery & Power Simulation
// ============================================
class BatterySimulator {
public:
    BatterySimulator();
    
    // Battery status
    int getBatteryLevel();              // 0-100
    bool isCharging();
    int getBatteryHealth();             // 0-100
    float getTemperature();             // Celsius
    float getVoltage();                 // Volts
    int getCurrentNow();                // mA (positive=discharging)
    int getCapacity();                  // mAh
    
    // Power consumption simulation
    void setScreenOn(bool on);
    void setWIFIOn(bool on);
    void setGPSOn(bool on);
    void setBluetoothOn(bool on);
    void setCellularOn(bool on);
    
    // Update battery state
    void update(uint64_t deltaMs);
    
    // Set specific battery state
    void setBatteryLevel(int level);
    void setCharging(bool charging);
    void setHealth(int health);
    
    // Realistic battery drain patterns
    void enableRealisticDrain(bool enable);
    void setActivityProfile(const std::string& profile);
    
private:
    int m_batteryLevel;
    bool m_charging;
    int m_health;
    float m_temperature;
    float m_voltage;
    int m_currentNow;
    int m_capacity;
    
    bool m_screenOn;
    bool m_wifiOn;
    bool m_gpsOn;
    bool m_bluetoothOn;
    bool m_cellularOn;
    
    bool m_realisticDrain;
    uint64_t m_lastUpdate;
    
    std::random_device m_rd;
    std::mt19937 m_gen;
};

} // namespace VirtualPhonePro
