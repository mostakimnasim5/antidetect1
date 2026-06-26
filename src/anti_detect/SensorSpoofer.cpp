/**
 * SensorSpoofer - Human-Like Sensor Data Generation
 * 
 * Generates realistic sensor data that mimics physical device behavior.
 */

#include "anti_detect/SensorSpoofer.hpp"
#include "core/CryptoUtils.hpp"
#include <cmath>
#include <unistd.h>

namespace VirtualPhonePro {
namespace AntiDetect {

SensorSpoofer& SensorSpoofer::getInstance() {
    static SensorSpoofer instance;
    return instance;
}

SensorSpoofer::SensorSpoofer()
    : m_activity(ActivityState::STATIONARY)
    , m_pitch(0.0f)
    , m_roll(0.0f)
    , m_yaw(0.0f)
    , m_lat(23.8103)  // Dhaka, Bangladesh
    , m_lon(90.4125)
    , m_alt(10.0)
    , m_sensorStartTime(0)
    , m_lastUpdateTime(0)
    , m_stepCount(0)
    , m_lastStepTime(0)
    , m_stepPhase(0.0f)
    , m_ambientLux(300.0f)
    , m_pressure(1013.25f)
    , m_temperature(25.0f)
    , m_noiseEngine(Crypto::SecureRandomGenerator().generateUint64())
    , m_gaussianNoise(0.0f, 0.01f)
{
    initializeSensorProfiles();
    
    // Get boot time for sensor timestamps
    struct timespec ts;
    clock_gettime(CLOCK_BOOTTIME, &ts);
    m_sensorStartTime = ts.tv_sec * 1000000LL + ts.tv_nsec / 1000LL;
    m_lastUpdateTime = m_sensorStartTime;
    
    // Initialize walking pattern
    m_walking.stepLength = 0.7f;  // 70cm
    m_walking.stepsPerMinute = 100;
    m_walking.strideVariance = 0.05f;
    m_walking.leftFoot = false;
    
    // Initialize tremor profile (natural hand tremor)
    m_tremor.amplitudeX = 0.3f;  // mm
    m_tremor.amplitudeY = 0.2f;
    m_tremor.amplitudeZ = 0.1f;
    m_tremor.frequencyX = 3.5f;   // Hz
    m_tremor.frequencyY = 4.2f;
    m_tremor.frequencyZ = 2.8f;
    m_tremor.driftRate = 0.001f;
    
    m_tremorPhaseX = 0.0f;
    m_tremorPhaseY = 0.0f;
    m_tremorPhaseZ = 0.0f;
}

SensorSpoofer::~SensorSpoofer() = default;

void SensorSpoofer::initializeSensorProfiles() {
    // Enable all sensors by default
    m_enabledSensors = {
        SensorType::ACCELEROMETER,
        SensorType::GYROSCOPE,
        SensorType::MAGNETOMETER,
        SensorType::LIGHT,
        SensorType::PROXIMITY,
        SensorType::PRESSURE,
        SensorType::GPS,
        SensorType::ROTATION,
        SensorType::GRAVITY,
        SensorType::LINEAR_ACCELERATION
    };
    
    // Sensor noise levels (RMS noise in m/s^2)
    m_noiseLevels[SensorType::ACCELEROMETER] = 0.005f;
    m_noiseLevels[SensorType::GYROSCOPE] = 0.001f;
    m_noiseLevels[SensorType::MAGNETOMETER] = 0.1f;
    m_noiseLevels[SensorType::LIGHT] = 10.0f;
    
    // Default bias
    m_sensorBias[SensorType::ACCELEROMETER] = {0.0f, 0.0f, 0.0f};
    m_sensorBias[SensorType::GYROSCOPE] = {0.0f, 0.0f, 0.0f};
}

bool SensorSpoofer::initialize(const std::string& deviceModel) {
    initializeSensorProfiles();
    return true;
}

void SensorSpoofer::setActivityState(ActivityState state) {
    m_activity = state;
}

void SensorSpoofer::setDeviceOrientation(float pitch, float roll, float yaw) {
    m_pitch = pitch;
    m_roll = roll;
    m_yaw = yaw;
}

void SensorSpoofer::setGPSLocation(double lat, double lon, double alt) {
    m_lat = lat;
    m_lon = lon;
    m_alt = alt;
}

float SensorSpoofer::addNoise(float value, float noiseLevel) {
    return value + m_gaussianNoise(m_noiseEngine) * noiseLevel;
}

SensorData SensorSpoofer::getAccelerometer(int64_t timestamp) {
    SensorData data;
    data.type = SensorType::ACCELEROMETER;
    data.timestamp = timestamp;
    data.accuracy = 3; // HIGH
    
    // Apply gravity
    float gx = 0.0f, gy = 0.0f, gz = GRAVITY;
    calculateGravityVector(gx, gy, gz);
    
    // Apply activity pattern
    if (m_activity == ActivityState::WALKING) {
        SensorData walking = calculateWalkingAcceleration(timestamp);
        data.x = gx + walking.x;
        data.y = gy + walking.y;
        data.z = gz + walking.z;
    } else if (m_activity == ActivityState::STATIONARY) {
        data.x = gx;
        data.y = gy;
        data.z = gz;
    } else if (m_activity == ActivityState::IN_VEHICLE) {
        // Vehicle has smoother, lower frequency acceleration
        data.x = addNoise(gx, 0.02f);
        data.y = addNoise(gy, 0.02f);
        data.z = addNoise(gz - 0.1f, 0.02f); // Slight deceleration
    }
    
    // Apply tremor
    SensorData tremor = applyTremor(timestamp);
    data.x += tremor.x;
    data.y += tremor.y;
    data.z += tremor.z;
    
    // Apply noise
    data.x = addNoise(data.x, m_noiseLevels[SensorType::ACCELEROMETER]);
    data.y = addNoise(data.y, m_noiseLevels[SensorType::ACCELEROMETER]);
    data.z = addNoise(data.z, m_noiseLevels[SensorType::ACCELEROMETER]);
    
    return data;
}

SensorData SensorSpoofer::getGyroscope(int64_t timestamp) {
    SensorData data;
    data.type = SensorType::GYROSCOPE;
    data.timestamp = timestamp;
    data.accuracy = 3;
    
    // Rotation rate based on activity
    if (m_activity == ActivityState::WALKING) {
        // Walking creates oscillation
        float phase = (timestamp - m_sensorStartTime) / 1000000.0f * 2.0f * M_PI;
        data.x = addNoise(sin(phase * 2.0f) * 0.5f, 0.01f);  // Forward/back sway
        data.y = addNoise(sin(phase * 3.0f) * 0.3f, 0.01f);  // Side sway
        data.z = addNoise(sin(phase * 1.5f) * 0.2f, 0.01f);  // Twist
    } else if (m_activity == ActivityState::TILTING) {
        // Slow rotation
        data.x = addNoise(0.1f, 0.05f);
        data.y = addNoise(0.05f, 0.05f);
        data.z = addNoise(0.02f, 0.02f);
    } else {
        // Stationary - very small drift (tremor)
        SensorData tremor = applyTremor(timestamp);
        data.x = addNoise(tremor.x * 0.1f, 0.001f);
        data.y = addNoise(tremor.y * 0.1f, 0.001f);
        data.z = addNoise(tremor.z * 0.1f, 0.001f);
    }
    
    return data;
}

SensorData SensorSpoofer::getMagnetometer(int64_t timestamp) {
    SensorData data;
    data.type = SensorType::MAGNETOMETER;
    data.timestamp = timestamp;
    data.accuracy = 2; // MEDIUM
    
    // Earth magnetic field (varies by location)
    // In Bangladesh, roughly 38-45 μT
    float fieldStrength = 40.0f;
    
    // Orientation affects magnetic field vector
    float cosPitch = cos(m_pitch);
    float cosRoll = cos(m_roll);
    float sinYaw = sin(m_yaw);
    
    data.x = addNoise(fieldStrength * cosPitch * sinYaw, m_noiseLevels[SensorType::MAGNETOMETER]);
    data.y = addNoise(fieldStrength * cosPitch * cosRoll, m_noiseLevels[SensorType::MAGNETOMETER]);
    data.z = addNoise(fieldStrength * sinPitch, m_noiseLevels[SensorType::MAGNETOMETER]);
    
    return data;
}

SensorData SensorSpoofer::getLightSensor(int64_t timestamp) {
    SensorData data;
    data.type = SensorType::LIGHT;
    data.timestamp = timestamp;
    data.accuracy = 3;
    
    // Ambient light (realistic: 10-1000 lux)
    float baseLux = m_ambientLux;
    
    // Add slow variation (cloud cover, shadows)
    float variation = sin((timestamp - m_sensorStartTime) / 1000000.0f * 0.1f) * 50.0f;
    
    data.x = addNoise(baseLux + variation, 5.0f); // lux in x
    data.y = 0.0f;
    data.z = 0.0f;
    
    return data;
}

SensorData SensorSpoofer::getProximitySensor(int64_t timestamp) {
    SensorData data;
    data.type = SensorType::PROXIMITY;
    data.timestamp = timestamp;
    data.accuracy = 3;
    
    // Proximity is binary: near (0) or far (5)
    if (m_activity == ActivityState::STATIONARY) {
        // Phone likely on table - far
        data.x = addNoise(5.0f, 0.1f);
    } else {
        // In hand - near
        data.x = addNoise(0.0f, 0.1f);
    }
    data.y = 0.0f;
    data.z = 0.0f;
    
    return data;
}

SensorData SensorSpoofer::getBarometer(int64_t timestamp) {
    SensorData data;
    data.type = SensorType::PRESSURE;
    data.timestamp = timestamp;
    data.accuracy = 3;
    
    // Atmospheric pressure (hPa)
    // Decreases ~12hPa per 100m altitude
    float basePressure = 1013.25f - (m_alt / 100.0f) * 12.0f;
    
    // Slow weather variation
    float variation = sin((timestamp - m_sensorStartTime) / 1000000.0f * 0.01f) * 2.0f;
    
    data.x = addNoise(basePressure + variation, 0.1f); // hPa in x
    data.y = 0.0f;
    data.z = 0.0f;
    
    return data;
}

GPSLocation SensorSpoofer::getGPS(int64_t timestamp) {
    GPSLocation loc;
    loc.latitude = m_lat;
    loc.longitude = m_lon;
    loc.altitude = m_alt;
    loc.accuracy = 5.0f;  // meters
    loc.speed = 0.0f;
    loc.bearing = 0.0f;
    loc.timestamp = timestamp;
    loc.providerAccuracy = 3.0f;
    
    if (m_activity == ActivityState::WALKING) {
        loc.speed = 1.4f; // ~5 km/h walking
        loc.bearing = m_yaw * 180.0f / M_PI;
    } else if (m_activity == ActivityState::IN_VEHICLE) {
        loc.speed = 30.0f; // ~60 km/h in vehicle
        loc.bearing = m_yaw * 180.0f / M_PI;
    }
    
    // Apply GPS drift
    updateGPSDrift(timestamp - m_lastUpdateTime);
    
    return loc;
}

void SensorSpoofer::updateGPSDrift(int64_t deltaMs) {
    // GPS has natural drift even when stationary
    if (m_activity == ActivityState::STATIONARY) {
        // Small random drift
        std::normal_distribution<float> driftDist(0.0f, 0.000001f);
        m_lat += driftDist(m_noiseEngine);
        m_lon += driftDist(m_noiseEngine);
    } else if (m_activity == ActivityState::WALKING) {
        // Move based on bearing
        float speed = 1.4f; // m/s
        float distance = speed * deltaMs / 1000.0f;
        float bearingRad = m_yaw;
        
        m_lat += (distance / 111111.0f) * cos(bearingRad);
        m_lon += (distance / (111111.0f * cos(m_lat * M_PI / 180.0f))) * sin(bearingRad);
    }
}

void SensorSpoofer::calculateGravityVector(float& gx, float& gy, float& gz) {
    // Gravity always points down in device frame
    // Based on device orientation
    float cosPitch = cos(m_pitch);
    float sinPitch = sin(m_pitch);
    float cosRoll = cos(m_roll);
    float sinRoll = sin(m_roll);
    
    // Rotation matrix application
    gx = GRAVITY * sinPitch;
    gy = -GRAVITY * sinRoll * cosPitch;
    gz = -GRAVITY * cosRoll * cosPitch;
}

SensorData SensorSpoofer::calculateWalkingAcceleration(int64_t timestamp) {
    float phase = (timestamp - m_sensorStartTime) / 1000000.0f;
    
    // Step frequency
    float stepFreq = m_walking.stepsPerMinute / 60.0f;
    float stepPhase = phase * stepFreq * 2.0f * M_PI + m_stepPhase;
    
    // Walking acceleration pattern
    float accelX = sin(stepPhase) * 0.5f;  // Forward
    float accelY = sin(stepPhase * 2.0f) * 0.3f;  // Side to side
    float accelZ = abs(sin(stepPhase)) * 2.0f - 1.0f;  // Up and down
    
    // Add variance
    accelX += addNoise(0.0f, 0.1f);
    accelY += addNoise(0.0f, 0.05f);
    accelZ += addNoise(0.0f, 0.2f);
    
    SensorData result;
    result.x = accelX;
    result.y = accelY;
    result.z = accelZ;
    
    return result;
}

SensorData SensorSpoofer::applyTremor(int64_t timestamp) {
    float time = (timestamp - m_sensorStartTime) / 1000000.0f;
    
    // Natural hand tremor frequencies
    float phaseX = time * m_tremor.frequencyX * 2.0f * M_PI + m_tremorPhaseX;
    float phaseY = time * m_tremor.frequencyY * 2.0f * M_PI + m_tremorPhaseY;
    float phaseZ = time * m_tremor.frequencyZ * 2.0f * M_PI + m_tremorPhaseZ;
    
    // Update phase slowly (tremor drifts)
    m_tremorPhaseX += m_tremor.driftRate * 0.01f;
    m_tremorPhaseY += m_tremor.driftRate * 0.01f;
    m_tremorPhaseZ += m_tremor.driftRate * 0.01f;
    
    SensorData result;
    result.x = sin(phaseX) * m_tremor.amplitudeX / 1000.0f;  // Convert mm to m
    result.y = sin(phaseY) * m_tremor.amplitudeY / 1000.0f;
    result.z = sin(phaseZ) * m_tremor.amplitudeZ / 1000.0f;
    
    return result;
}

SensorData SensorSpoofer::applyOrientationChange(int64_t timestamp) {
    SensorData result;
    float time = (timestamp - m_lastUpdateTime) / 1000000.0f;
    
    // Smooth orientation change
    result.x = m_pitch * 0.1f * time;
    result.y = m_roll * 0.1f * time;
    result.z = m_yaw * 0.1f * time;
    
    m_lastUpdateTime = timestamp;
    return result;
}

int SensorSpoofer::detectStep(int64_t timestamp) {
    if (m_activity != ActivityState::WALKING) {
        return 0;
    }
    
    int64_t timeSinceLastStep = timestamp - m_lastStepTime;
    int64_t stepIntervalUs = 60000000 / m_walking.stepsPerMinute; // μs per step
    
    if (timeSinceLastStep >= stepIntervalUs) {
        m_stepCount++;
        m_lastStepTime = timestamp;
        m_walking.leftFoot = !m_walking.leftFoot;
        return m_stepCount;
    }
    
    return 0;
}

bool SensorSpoofer::isStepDetected() {
    return detectStep(m_lastUpdateTime) > 0;
}

std::vector<SensorData> SensorSpoofer::generateSensorBatch(
    const std::vector<SensorType>& sensors,
    int64_t startTime,
    int64_t endTime,
    int samplesPerSecond) {
    
    std::vector<SensorData> batch;
    int64_t intervalUs = 1000000 / samplesPerSecond;
    
    for (int64_t t = startTime; t <= endTime; t += intervalUs) {
        for (const auto& sensor : sensors) {
            switch (sensor) {
                case SensorType::ACCELEROMETER:
                    batch.push_back(getAccelerometer(t));
                    break;
                case SensorType::GYROSCOPE:
                    batch.push_back(getGyroscope(t));
                    break;
                case SensorType::MAGNETOMETER:
                    batch.push_back(getMagnetometer(t));
                    break;
                case SensorType::LIGHT:
                    batch.push_back(getLightSensor(t));
                    break;
                case SensorType::PRESSURE:
                    batch.push_back(getBarometer(t));
                    break;
                default:
                    break;
            }
        }
    }
    
    return batch;
}

std::map<SensorType, SensorData> SensorSpoofer::getAllSensors(int64_t timestamp) {
    std::map<SensorType, SensorData> all;
    
    if (m_enabledSensors.count(SensorType::ACCELEROMETER)) {
        all[SensorType::ACCELEROMETER] = getAccelerometer(timestamp);
    }
    if (m_enabledSensors.count(SensorType::GYROSCOPE)) {
        all[SensorType::GYROSCOPE] = getGyroscope(timestamp);
    }
    if (m_enabledSensors.count(SensorType::MAGNETOMETER)) {
        all[SensorType::MAGNETOMETER] = getMagnetometer(timestamp);
    }
    if (m_enabledSensors.count(SensorType::LIGHT)) {
        all[SensorType::LIGHT] = getLightSensor(timestamp);
    }
    if (m_enabledSensors.count(SensorType::PRESSURE)) {
        all[SensorType::PRESSURE] = getBarometer(timestamp);
    }
    
    return all;
}

void SensorSpoofer::reset() {
    m_stepCount = 0;
    m_lastStepTime = 0;
    m_stepPhase = 0.0f;
    m_tremorPhaseX = 0.0f;
    m_tremorPhaseY = 0.0f;
    m_tremorPhaseZ = 0.0f;
}

void SensorSpoofer::setWalkingSpeed(float metersPerSecond) {
    m_walking.stepsPerMinute = static_cast<int>(metersPerSecond * 60 / m_walking.stepLength);
}

void SensorSpoofer::enableSensor(SensorType type) {
    m_enabledSensors.insert(type);
}

void SensorSpoofer::disableSensor(SensorType type) {
    m_enabledSensors.erase(type);
}

bool SensorSpoofer::isSensorEnabled(SensorType type) {
    return m_enabledSensors.count(type) > 0;
}

void SensorSpoofer::setAmbientLight(float lux) {
    m_ambientLux = lux;
}

void SensorSpoofer::setAltitude(float meters) {
    m_alt = meters;
}

} // namespace AntiDetect
} // namespace VirtualPhonePro
