#pragma once

/**
 * SensorSpoofer - Human-Like Sensor Data Generation
 * 
 * Generates realistic sensor data that mimics physical device:
 * 
 * Sensors Covered:
 * - Accelerometer (human walking/movement patterns)
 * - Gyroscope (natural hand tremor and rotation)
 * - Magnetometer (earth magnetic field)
 * - Proximity sensor
 * - Light sensor (ambient brightness changes)
 * - Barometer (atmospheric pressure)
 * - GPS (location with realistic drift)
 * 
 * Behavioral Patterns:
 * - Walking patterns (step detection)
 * - Vehicle motion (driving detection)
 * - Stationary patterns (phone on table)
 * - Hand tremor (natural jitter)
 * - Orientation changes (rotation)
 * 
 * Protection Level: Enterprise
 */

#include "../VirtualPhonePro.hpp"
#include <chrono>
#include <cmath>

namespace VirtualPhonePro {
namespace AntiDetect {

// Sensor types
enum class SensorType {
    ACCELEROMETER,
    GYROSCOPE,
    MAGNETOMETER,
    PROXIMITY,
    LIGHT,
    PRESSURE,
    GPS,
    ROTATION,
    GRAVITY,
    LINEAR_ACCELERATION,
    STEP_COUNTER,
    HEART_RATE
};

// Activity state
enum class ActivityState {
    STATIONARY,     // Phone sitting still
    WALKING,        // Human walking
    RUNNING,        // Human running
    IN_VEHICLE,     // In car/bus
    ON_BICYCLE,     // Cycling
    TILTING,        // Phone tilting
    UNKNOWN
};

// Sensor data structure
struct SensorData {
    SensorType type;
    float x, y, z;           // Primary values
    float accuracy;          // Sensor accuracy
    int64_t timestamp;       // Microseconds since boot
    float accuracy_high;
    float accuracy_low;
};

// GPS Location
struct GPSLocation {
    double latitude;
    double longitude;
    double altitude;
    float accuracy;
    float speed;
    float bearing;
    int64_t timestamp;
    float providerAccuracy;
};

// Walking pattern
struct WalkingPattern {
    float stepLength;        // meters
    int stepsPerMinute;      // cadence
    float strideVariance;    // variation in step length
    bool leftFoot;
};

// Hand tremor characteristics
struct TremorProfile {
    float amplitudeX;        // mm
    float amplitudeY;
    float amplitudeZ;
    float frequencyX;        // Hz
    float frequencyY;
    float frequencyZ;
    float driftRate;
};

class SensorSpoofer {
public:
    static SensorSpoofer& getInstance();
    
    // Initialize with device profile
    bool initialize(const std::string& deviceModel);
    
    // Set activity state
    void setActivityState(ActivityState state);
    
    // Set orientation
    void setDeviceOrientation(float pitch, float roll, float yaw);
    
    // Set location
    void setGPSLocation(double lat, double lon, double alt);
    
    // Get sensor reading
    SensorData getAccelerometer(int64_t timestamp);
    SensorData getGyroscope(int64_t timestamp);
    SensorData getMagnetometer(int64_t timestamp);
    SensorData getLightSensor(int64_t timestamp);
    SensorData getProximitySensor(int64_t timestamp);
    SensorData getBarometer(int64_t timestamp);
    GPSLocation getGPS(int64_t timestamp);
    
    // Generate batch of sensor readings
    std::vector<SensorData> generateSensorBatch(
        const std::vector<SensorType>& sensors,
        int64_t startTime,
        int64_t endTime,
        int samplesPerSecond);
    
    // Walking detection
    int detectStep(int64_t timestamp);
    bool isStepDetected();
    
    // Natural movement patterns
    SensorData applyWalkingPattern(int64_t timestamp);
    SensorData applyTremor(int64_t timestamp);
    SensorData applyOrientationChange(int64_t timestamp);
    
    // Environmental factors
    void setAmbientLight(float lux);
    void setAltitude(float meters);
    
    // Get all sensor data at once
    std::map<SensorType, SensorData> getAllSensors(int64_t timestamp);
    
    // Reset sensor state
    void reset();
    
    // Set walking speed
    void setWalkingSpeed(float metersPerSecond);
    
    // Enable/disable specific sensors
    void enableSensor(SensorType type);
    void disableSensor(SensorType type);
    bool isSensorEnabled(SensorType type);
    
private:
    SensorSpoofer();
    ~SensorSpoofer();
    SensorSpoofer(const SensorSpoofer&) = delete;
    SensorSpoofer& operator=(const SensorSpoofer&) = delete;
    
    // Current state
    ActivityState m_activity;
    float m_pitch, m_roll, m_yaw;
    double m_lat, m_lon, m_alt;
    
    // Time tracking
    int64_t m_sensorStartTime;
    int64_t m_lastUpdateTime;
    
    // Physical constants
    const float GRAVITY = 9.81f;
    const float EARTH_MAGNETIC_FIELD = 45.0f; // μT
    
    // Walking state
    WalkingPattern m_walking;
    int m_stepCount;
    int64_t m_lastStepTime;
    float m_stepPhase;
    
    // Tremor profile
    TremorProfile m_tremor;
    float m_tremorPhaseX, m_tremorPhaseY, m_tremorPhaseZ;
    
    // Environment
    float m_ambientLux;
    float m_pressure; // hPa
    float m_temperature; // Celsius
    
    // Natural noise generators
    std::mt19937 m_noiseEngine;
    std::normal_distribution<float> m_gaussianNoise;
    
    // Device-specific calibration
    std::map<SensorType, std::tuple<float, float, float>> m_sensorBias;
    std::map<SensorType, std::tuple<float, float, float>> m_sensorScale;
    
    // Sensor noise levels (per device)
    std::map<SensorType, float> m_noiseLevels;
    
    // Enabled sensors
    std::set<SensorType> m_enabledSensors;
    
    // Initialize sensor characteristics
    void initializeSensorProfiles();
    
    // Apply sensor noise
    float addNoise(float value, float noiseLevel);
    
    // Generate realistic GPS drift
    void updateGPSDrift(int64_t deltaMs);
    
    // Calculate walking acceleration
    SensorData calculateWalkingAcceleration(int64_t timestamp);
    
    // Calculate gravity vector
    void calculateGravityVector(float& gx, float& gy, float& gz);
};

} // namespace AntiDetect
} // namespace VirtualPhonePro
