/**
 * AdvancedSensorSimulator - Ultra-Realistic Sensor Simulation Implementation
 */

#include "hardware/AdvancedSensorSimulator.hpp"
#include <chrono>
#include <cmath>
#include <thread>

namespace VirtualPhonePro {

// ============================================
// AdvancedSensorSimulator Implementation
// ============================================
AdvancedSensorSimulator::AdvancedSensorSimulator()
    : m_currentTemperature(25.0f),
      m_latitude(0.0),
      m_longitude(0.0),
      m_driftAccumulator(0.0f) {
    
    m_startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    m_lastUpdateTime = m_startTime;
    
    m_config = AdvancedSensorConfig();
    m_movementState.activity = MovementState::Activity::STATIONARY;
    m_movementState.velocity = 0.0f;
    m_movementState.acceleration = 0.0f;
    m_movementState.heading = {0.0f, 0.0f, 1.0f};
    
    // Initialize base values
    m_baseValues["accelerometer"] = {0.0f, 0.0f, 9.81f};
    m_baseValues["gyroscope"] = {0.0f, 0.0f, 0.0f};
    m_baseValues["magnetometer"] = {25.0f, 0.0f, -45.0f};
    m_baseValues["gravity"] = {0.0f, 0.0f, 9.81f};
    m_baseValues["linear"] = {0.0f, 0.0f, 0.0f};
    
    // Initialize slow drift
    m_slowDrift = {0.0f, 0.0f, 0.0f};
    m_fastVariation = {0.0f, 0.0f, 0.0f};
    
    initializeCalibration();
}

AdvancedSensorSimulator::~AdvancedSensorSimulator() {}

AdvancedSensorSimulator& AdvancedSensorSimulator::getInstance() {
    static AdvancedSensorSimulator instance;
    return instance;
}

void AdvancedSensorSimulator::configure(const AdvancedSensorConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_config = config;
}

void AdvancedSensorSimulator::setActivity(MovementState::Activity activity) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_movementState.activity = activity;
}

void AdvancedSensorSimulator::setTemperature(float celsius) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_currentTemperature = celsius;
}

void AdvancedSensorSimulator::setLocation(double lat, double lon) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_latitude = lat;
    m_longitude = lon;
}

void AdvancedSensorSimulator::initializeCalibration() {
    // Initialize calibration data for each sensor
    std::vector<std::string> sensorTypes = {
        "accelerometer", "gyroscope", "magnetometer", 
        "gravity", "linear", "rotation"
    };
    
    for (const auto& type : sensorTypes) {
        SensorCalibration cal;
        cal.sensorType = type;
        cal.offset = {0.0f, 0.0f, 0.0f};
        cal.scale = {1.0f, 1.0f, 1.0f};
        cal.matrix = {1,0,0, 0,1,0, 0,0,1};
        cal.temperatureCoeff = 0.0001f;
        cal.calibrationDate = m_startTime - (uint64_t)(m_config.sensorAge * 86400000);
        cal.age = m_config.sensorAge;
        
        // Add realistic calibration offsets based on sensor type
        if (type == "accelerometer") {
            cal.offset = {
                generateRealisticNoise(0.002f),
                generateRealisticNoise(0.002f),
                generateRealisticNoise(0.003f)
            };
            cal.scale = {
                1.0f + generateRealisticNoise(0.001f),
                1.0f + generateRealisticNoise(0.001f),
                1.0f + generateRealisticNoise(0.001f)
            };
        } else if (type == "gyroscope") {
            cal.offset = {
                generateRealisticNoise(0.001f),
                generateRealisticNoise(0.001f),
                generateRealisticNoise(0.001f)
            };
        } else if (type == "magnetometer") {
            cal.offset = {
                generateRealisticNoise(0.5f),
                generateRealisticNoise(0.5f),
                generateRealisticNoise(0.5f)
            };
        }
        
        m_calibrations[type] = cal;
    }
}

SensorReading AdvancedSensorSimulator::getAccelerometer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    SensorReading reading;
    
    // Base value - gravity
    reading.x = 0.0f;
    reading.y = 0.0f;
    reading.z = 9.81f;
    
    // Add activity-based movement
    float movementScale = 0.0f;
    switch (m_movementState.activity) {
        case MovementState::Activity::WALKING:
            movementScale = 0.5f;
            break;
        case MovementState::Activity::RUNNING:
            movementScale = 1.5f;
            break;
        case MovementState::Activity::DRIVING:
            movementScale = 0.3f;
            break;
        case MovementState::Activity::IN_VEHICLE:
            movementScale = 0.2f;
            break;
        default:
            movementScale = 0.01f;
    }
    
    // Add micro-movements
    if (movementScale > 0.0f) {
        reading.x += generateRealisticNoise(movementScale * 0.1f);
        reading.y += generateRealisticNoise(movementScale * 0.1f);
        reading.z += 9.81f + generateRealisticNoise(movementScale * 0.05f);
    }
    
    reading.timestamp = now;
    reading.temperature = m_currentTemperature;
    reading.isCalibrated = m_config.enableCalibration;
    reading.confidence = 0.95f + generateRealisticNoise(0.02f);
    
    // Apply calibration
    if (m_config.enableCalibration) {
        applyCalibration(reading, "accelerometer");
    }
    
    // Apply temperature drift
    if (m_config.enableTemperatureDrift) {
        applyTemperatureDrift(reading);
    }
    
    // Apply hysteresis
    if (m_config.enableHysteresis) {
        applyHysteresis(reading, "accelerometer");
    }
    
    // Update previous reading
    m_previousReadings["accelerometer"] = reading;
    
    return reading;
}

SensorReading AdvancedSensorSimulator::getGyroscope() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    SensorReading reading;
    
    // Base gyroscope values based on activity
    switch (m_movementState.activity) {
        case MovementState::Activity::STATIONARY:
            reading = {0.001f, 0.001f, 0.001f};
            break;
        case MovementState::Activity::WALKING:
            reading = {0.05f, 0.1f, 0.02f};
            break;
        case MovementState::Activity::RUNNING:
            reading = {0.2f, 0.3f, 0.1f};
            break;
        case MovementState::Activity::DRIVING:
        case MovementState::Activity::IN_VEHICLE:
            reading = {0.01f, 0.02f, 0.01f};
            break;
        default:
            reading = {0.001f, 0.001f, 0.001f};
    }
    
    // Add realistic noise
    reading.x += generateRealisticNoise(0.002f);
    reading.y += generateRealisticNoise(0.002f);
    reading.z += generateRealisticNoise(0.002f);
    
    reading.timestamp = now;
    reading.temperature = m_currentTemperature;
    reading.isCalibrated = m_config.enableCalibration;
    reading.confidence = 0.93f + generateRealisticNoise(0.03f);
    
    if (m_config.enableCalibration) {
        applyCalibration(reading, "gyroscope");
    }
    if (m_config.enableTemperatureDrift) {
        applyTemperatureDrift(reading);
    }
    if (m_config.enableHysteresis) {
        applyHysteresis(reading, "gyroscope");
    }
    
    m_previousReadings["gyroscope"] = reading;
    return reading;
}

SensorReading AdvancedSensorSimulator::getMagnetometer() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    uint64_t now = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    
    SensorReading reading;
    
    // Base magnetic field - varies by location
    float fieldStrength = 25.0f + generateRealisticNoise(5.0f);
    reading = {
        fieldStrength * 0.3f,
        fieldStrength * 0.1f,
        -fieldStrength * 0.9f
    };
    
    // Add slow drift (magnetic declination changes slowly)
    m_slowDrift[0] += generateRealisticNoise(0.01f);
    m_slowDrift[1] += generateRealisticNoise(0.01f);
    m_slowDrift[2] += generateRealisticNoise(0.01f);
    
    reading.x += m_slowDrift[0];
    reading.y += m_slowDrift[1];
    reading.z += m_slowDrift[2];
    
    reading.timestamp = now;
    reading.temperature = m_currentTemperature;
    reading.isCalibrated = m_config.enableCalibration;
    reading.confidence = 0.88f + generateRealisticNoise(0.05f);
    
    if (m_config.enableCalibration) {
        applyCalibration(reading, "magnetometer");
    }
    if (m_config.enableTemperatureDrift) {
        applyTemperatureDrift(reading);
    }
    if (m_config.enableHysteresis) {
        applyHysteresis(reading, "magnetometer");
    }
    
    m_previousReadings["magnetometer"] = reading;
    return reading;
}

SensorReading AdvancedSensorSimulator::getGravity() {
    SensorReading accel = getAccelerometer();
    SensorReading reading;
    reading.x = 0.0f;
    reading.y = 0.0f;
    reading.z = 9.81f;
    reading.timestamp = accel.timestamp;
    reading.temperature = accel.temperature;
    reading.isCalibrated = true;
    reading.confidence = 0.98f;
    return reading;
}

SensorReading AdvancedSensorSimulator::getLinearAcceleration() {
    SensorReading accel = getAccelerometer();
    SensorReading gravity = getGravity();
    
    SensorReading reading;
    reading.x = accel.x - gravity.x;
    reading.y = accel.y - gravity.y;
    reading.z = accel.z - gravity.z;
    reading.timestamp = accel.timestamp;
    reading.temperature = accel.temperature;
    reading.isCalibrated = true;
    reading.confidence = 0.92f;
    
    return reading;
}

SensorReading AdvancedSensorSimulator::getRotationVector() {
    // Simplified rotation vector (quaternion-like)
    SensorReading reading;
    
    auto euler = getEulerAngles();
    float halfYaw = euler[0] / 2.0f;
    float halfPitch = euler[1] / 2.0f;
    float halfRoll = euler[2] / 2.0f;
    
    // Convert to quaternion representation (x,y,z,scalar)
    reading.x = sin(halfRoll) * cos(halfPitch) * cos(halfYaw) - cos(halfRoll) * sin(halfPitch) * sin(halfYaw);
    reading.y = cos(halfRoll) * sin(halfPitch) * cos(halfYaw) + sin(halfRoll) * cos(halfPitch) * sin(halfYaw);
    reading.z = cos(halfRoll) * cos(halfPitch) * sin(halfYaw) - sin(halfRoll) * sin(halfPitch) * cos(halfYaw);
    reading.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
    reading.isCalibrated = true;
    reading.confidence = 0.90f;
    
    return reading;
}

float AdvancedSensorSimulator::getPressure() {
    // Standard atmospheric pressure with variation
    float base = 1013.25f;
    float variation = 0.0f;
    
    // Weather-like variation
    variation += generateRealisticNoise(2.0f);
    
    // Altitude effect (rough approximation)
    // About 12 Pa per meter
    float altitude = getAltitude();
    variation -= altitude * 0.012f;
    
    return base + variation;
}

float AdvancedSensorSimulator::getAltitude() {
    // Simulated altitude based on pressure
    float pressure = getPressure();
    // Standard atmosphere formula
    float altitude = 44330.0f * (1.0f - powf(pressure / 1013.25f, 0.1903f));
    return altitude + generateRealisticNoise(5.0f);
}

float AdvancedSensorSimulator::getProximity() {
    // Usually 0 (far) or small value (near)
    float base = m_movementState.activity == MovementState::Activity::STATIONARY ? 0.0f : 5.0f;
    return base + generateRealisticNoise(0.5f);
}

float AdvancedSensorSimulator::getLight() {
    // Lux values: dark (0-10), indoor (100-500), outdoor (10000-100000)
    float base;
    
    if (m_movementState.activity == MovementState::Activity::IN_VEHICLE ||
        m_movementState.activity == MovementState::Activity::DRIVING) {
        base = 500.0f;  // Inside car
    } else if (m_movementState.activity == MovementState::Activity::STATIONARY) {
        base = 300.0f;  // Indoor
    } else {
        base = 15000.0f;  // Outdoor
    }
    
    // Add flicker noise (like indoor lighting)
    float flicker = generateFlickerNoise() * base * 0.05f;
    
    return base + flicker + generateRealisticNoise(base * 0.1f);
}

std::tuple<double, double, float> AdvancedSensorSimulator::getGPS() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    float accuracy = 5.0f;  // meters
    
    // Accuracy varies by activity
    switch (m_movementState.activity) {
        case MovementState::Activity::STATIONARY:
            accuracy = 3.0f;
            break;
        case MovementState::Activity::WALKING:
            accuracy = 5.0f;
            break;
        case MovementState::Activity::DRIVING:
        case MovementState::Activity::IN_VEHICLE:
            accuracy = 10.0f;
            break;
        default:
            accuracy = 5.0f;
    }
    
    // Add drift
    double latDrift = generateRealisticNoise(accuracy * 0.00001);
    double lonDrift = generateRealisticNoise(accuracy * 0.00001);
    
    // Occasional burst noise
    if (generateRealisticNoise(1.0f) > 0.98f) {
        latDrift += generateRealisticNoise(accuracy * 0.001);
        lonDrift += generateRealisticNoise(accuracy * 0.001);
    }
    
    return {
        m_latitude + latDrift,
        m_longitude + lonDrift,
        accuracy
    };
}

float AdvancedSensorSimulator::getHumidity() {
    // Typical indoor humidity 30-60%
    return 45.0f + generateRealisticNoise(10.0f);
}

float AdvancedSensorSimulator::getAmbientTemperature() {
    return m_currentTemperature + generateRealisticNoise(0.5f);
}

std::map<std::string, SensorReading> AdvancedSensorSimulator::getAllSensorReadings() {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    return {
        {"accelerometer", getAccelerometer()},
        {"gyroscope", getGyroscope()},
        {"magnetometer", getMagnetometer()},
        {"gravity", getGravity()},
        {"linear", getLinearAcceleration()},
        {"rotation", getRotationVector()}
    };
}

std::array<float, 4> AdvancedSensorSimulator::getOrientationQuaternion() {
    auto euler = getEulerAngles();
    
    float halfYaw = euler[0] / 2.0f;
    float halfPitch = euler[1] / 2.0f;
    float halfRoll = euler[2] / 2.0f;
    
    float cy = cos(halfYaw);
    float sy = sin(halfYaw);
    float cp = cos(halfPitch);
    float sp = sin(halfPitch);
    float cr = cos(halfRoll);
    float sr = sin(halfRoll);
    
    return {
        sr * cp * cy - cr * sp * sy,  // x
        cr * sp * cy + sr * cp * sy,  // y
        cr * cp * sy - sr * sp * cy,  // z
        cr * cp * cy + sr * sp * sy   // w
    };
}

std::array<float, 9> AdvancedSensorSimulator::getRotationMatrix() {
    auto quat = getOrientationQuaternion();
    
    float x = quat[0], y = quat[1], z = quat[2], w = quat[3];
    
    return {
        1.0f - 2.0f*(y*y + z*z), 2.0f*(x*y - z*w), 2.0f*(x*z + y*w),
        2.0f*(x*y + z*w), 1.0f - 2.0f*(x*x + z*z), 2.0f*(y*z - x*w),
        2.0f*(x*z - y*w), 2.0f*(y*z + x*w), 1.0f - 2.0f*(x*x + y*y)
    };
}

std::array<float, 3> AdvancedSensorSimulator::getEulerAngles() {
    // Simplified euler angles based on device orientation
    std::array<float, 3> euler = {0.0f, 0.0f, 0.0f};
    
    // Add small variations to simulate natural device tilt
    euler[0] = generateRealisticNoise(0.05f);  // Yaw (rotation around Z)
    euler[1] = generateRealisticNoise(0.03f);  // Pitch (rotation around X)
    euler[2] = generateRealisticNoise(0.02f);  // Roll (rotation around Y)
    
    return euler;
}

void AdvancedSensorSimulator::applyTemperatureDrift(SensorReading& reading) {
    if (!m_config.enableTemperatureDrift) return;
    
    float tempDiff = m_currentTemperature - 25.0f;  // Reference temperature
    float drift = tempDiff * reading.temperature * m_config.temperatureDrift;
    
    reading.x += drift;
    reading.y += drift;
    reading.z += drift;
}

void AdvancedSensorSimulator::applyCalibration(SensorReading& reading, const std::string& sensorType) {
    auto it = m_calibrations.find(sensorType);
    if (it == m_calibrations.end()) return;
    
    const auto& cal = it->second;
    
    // Apply offset
    reading.x -= cal.offset[0];
    reading.y -= cal.offset[1];
    reading.z -= cal.offset[2];
    
    // Apply scale
    reading.x *= cal.scale[0];
    reading.y *= cal.scale[1];
    reading.z *= cal.scale[2];
}

void AdvancedSensorSimulator::applyHysteresis(SensorReading& reading, const std::string& sensorType) {
    if (!m_config.enableHysteresis) return;
    
    auto it = m_previousReadings.find(sensorType);
    if (it == m_previousReadings.end()) return;
    
    const auto& prev = it->second;
    
    // Apply low-pass filter for smooth transitions
    float alpha = 0.9f;  // Smoothing factor
    reading.x = alpha * prev.x + (1.0f - alpha) * reading.x;
    reading.y = alpha * prev.y + (1.0f - alpha) * reading.y;
    reading.z = alpha * prev.z + (1.0f - alpha) * reading.z;
}

void AdvancedSensorSimulator::applyMovementCorrelation(SensorReading& reading) {
    if (!m_config.enableMovementCorrelation) return;
    
    // Accelerometer correlates with gyroscope
    auto gyro = m_previousReadings.find("gyroscope");
    if (gyro != m_previousReadings.end()) {
        reading.x += gyro->second.x * 0.01f;
        reading.y += gyro->second.y * 0.01f;
        reading.z += gyro->second.z * 0.01f;
    }
}

float AdvancedSensorSimulator::generateRealisticNoise(float stddev) {
    std::normal_distribution<float> dist(0.0f, stddev);
    return dist(m_rd);
}

float AdvancedSensorSimulator::generateFlickerNoise() {
    // 1/f noise (pink noise) - common in sensor data
    static float lastValue = 0.0f;
    float alpha = 0.95f;
    float whiteNoise = generateRealisticNoise(1.0f);
    lastValue = alpha * lastValue + (1.0f - alpha) * whiteNoise;
    return lastValue;
}

float AdvancedSensorSimulator::generateBurstNoise() {
    // Occasional spikes - like electromagnetic interference
    if (generateRealisticNoise(1.0f) > 0.995f) {
        return generateRealisticNoise(10.0f);
    }
    return 0.0f;
}

// ============================================
// RealisticDeviceProperties Implementation
// ============================================
RealisticDeviceProperties::RealisticDeviceProperties()
    : m_rd(), m_gen(m_rd()) {}

std::map<std::string, std::string> RealisticDeviceProperties::generateAllProperties() {
    std::map<std::string, std::string> props;
    
    props["ro.product.manufacturer"] = m_manufacturer;
    props["ro.product.model"] = m_model;
    props["ro.product.brand"] = m_brand;
    props["ro.product.device"] = m_device;
    props["ro.product.product"] = m_product;
    props["ro.product.board"] = m_board;
    props["ro.product.hardware"] = m_hardware;
    
    props["ro.build.version.release"] = m_androidVersion;
    props["ro.build.version.security_patch"] = m_securityPatch;
    props["ro.build.id"] = generateBuildId();
    props["ro.build.display.id"] = generateBuildNumber();
    props["ro.build.type"] = "user";
    props["ro.build.tags"] = "release-keys";
    
    props["ro.bootloader"] = generateBootloader();
    props["ro.build.fingerprint"] = generateBuildFingerprint();
    
    props["ro.serialno"] = generateSerialNumber();
    props["persist.sys.timezone"] = "Asia/Dhaka";
    props["persist.sys.locale"] = "en-BD";
    
    return props;
}

void RealisticDeviceProperties::setDeviceProfile(const std::string& manufacturer, const std::string& model) {
    m_manufacturer = manufacturer;
    m_model = model;
    m_brand = manufacturer;
    m_device = model.substr(0, 4);  // Simplified
    m_product = m_device;
    m_androidVersion = "13";
    m_securityPatch = "2024-01-01";
}

std::string RealisticDeviceProperties::generateBuildFingerprint() {
    return m_manufacturer + "/" + m_product + "/" + m_device + 
           ":" + m_androidVersion + "/" + generateBuildId() + 
           "/" + generateBuildNumber() + ":"user/release-keys";
}

std::string RealisticDeviceProperties::generateBootloader() {
    std::string prefix = m_model.substr(0, 4);
    return prefix + "U1A1" + std::to_string(m_rd() % 100);
}

std::string RealisticDeviceProperties::generateBuildId() {
    const char* prefixes[] = {"SP1A", "TP1A", "RQ3A", "PPR1"};
    std::string prefix = prefixes[m_rd() % 4];
    return prefix + ".210" + std::to_string(m_rd() % 1000) + "P";
}

std::string RealisticDeviceProperties::generateBuildNumber() {
    return std::to_string(m_rd() % 100 + 200) + ".0." + 
           std::to_string(m_rd() % 100 + 1000);
}

std::string RealisticDeviceProperties::generateSerialNumber() {
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::string serial;
    for (int i = 0; i < 12; i++) {
        serial += chars[m_rd() % 36];
    }
    return serial;
}

std::string RealisticDeviceProperties::generateIMEI() {
    std::string imei = "35" + std::to_string(m_rd() % 10) + 
                       std::to_string(m_rd() % 10) + 
                       std::to_string(m_rd() % 10) + 
                       std::to_string(m_rd() % 10) + "9" +
                       std::to_string(m_rd() % 100000000);
    
    // Add Luhn checksum
    int sum = 0;
    for (int i = 0; i < 14; i++) {
        int digit = imei[i] - '0';
        if (i % 2 == 1) digit *= 2;
        sum += digit > 9 ? digit - 9 : digit;
    }
    int checkDigit = (10 - (sum % 10)) % 10;
    imei += std::to_string(checkDigit);
    
    return imei;
}

std::string RealisticDeviceProperties::generateWiFiMAC(const std::string& manufacturer) {
    // Realistic OUI prefixes
    static const std::map<std::string, std::string> ouiMap = {
        {"Samsung", "D8:C4:E9"},
        {"Google", "3C:5A:B4"},
        {"Xiaomi", "58:44:98"},
        {"OnePlus", "2A:6B:C0"},
        {"Apple", "F0:18:98"},
        {"default", "00:1A:2B"}
    };
    
    std::string oui;
    if (manufacturer.empty()) {
        auto it = ouiMap.find(m_manufacturer);
        oui = it != ouiMap.end() ? it->second : ouiMap.at("default");
    } else {
        auto it = ouiMap.find(manufacturer);
        oui = it != ouiMap.end() ? it->second : ouiMap.at("default");
    }
    
    // Generate random NIC portion
    char buf[3];
    for (int i = 0; i < 3; i++) {
        snprintf(buf, sizeof(buf), "%02X", m_rd() % 256);
        oui += ":" + std::string(buf);
    }
    
    return oui;
}

std::string RealisticDeviceProperties::generateGSFId() {
    // Google Services Framework ID - large hex number
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int i = 0; i < 16; i++) {
        ss << std::setw(8) << (m_rd() % 0xFFFFFFFF);
    }
    return ss.str();
}

std::string RealisticDeviceProperties::generateAndroidId() {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    ss << std::setw(8) << (m_rd() % 0xFFFFFFFF);
    return ss.str();
}

std::string RealisticDeviceProperties::generateUUID() {
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    for (int i = 0; i < 4; i++) {
        ss << std::setw(8) << (m_rd() % 0xFFFFFFFF);
        if (i < 3) ss << "-";
    }
    
    return ss.str();
}

int RealisticDeviceProperties::luhnChecksum(const std::string& number) {
    int sum = 0;
    bool alternate = false;
    
    for (int i = number.length() - 1; i >= 0; i--) {
        int digit = number[i] - '0';
        if (alternate) {
            digit *= 2;
            if (digit > 9) digit -= 9;
        }
        sum += digit;
        alternate = !alternate;
    }
    
    return sum % 10;
}

// ============================================
// RealisticTimingSimulator Implementation
// ============================================
RealisticTimingSimulator::RealisticTimingSimulator()
    : m_jitterPercentage(5),
      m_timingProfile("flagship") {
    
    m_bootTime = std::chrono::duration_cast<std::chrono::seconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count() - m_rd() % 86400;  // Random boot time in last 24 hours
    
    m_baseUptime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
}

uint64_t RealisticTimingSimulator::getSystemUptime() {
    return m_baseUptime + calculateJitter(100);
}

uint64_t RealisticTimingSimulator::getRealTimeClock() {
    auto now = std::chrono::system_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

uint64_t RealisticTimingSimulator::getMonotonicClock() {
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
}

uint64_t RealisticTimingSimulator::getRDTSC() {
    #ifdef __x86_64__
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    uint64_t tsc = ((uint64_t)hi << 32) | lo;
    return tsc + calculateJitter(tsc * m_jitterPercentage / 100);
    #else
    return getMonotonicClock() * 1000000;  // Approximate
    #endif
}

void RealisticTimingSimulator::setTimingProfile(const std::string& profile) {
    m_timingProfile = profile;
    
    if (profile == "flagship") {
        m_jitterPercentage = 3;
    } else if (profile == "midrange") {
        m_jitterPercentage = 8;
    } else if (profile == "budget") {
        m_jitterPercentage = 15;
    } else if (profile == "gaming") {
        m_jitterPercentage = 5;
    }
}

void RealisticTimingSimulator::realisticSleep(int64_t milliseconds) {
    // Add realistic jitter to sleep
    int64_t jitter = calculateJitter(milliseconds * m_jitterPercentage / 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds + jitter));
}

void RealisticTimingSimulator::addJitter(int percentage) {
    m_jitterPercentage = percentage;
}

int64_t RealisticTimingSimulator::calculateJitter(int64_t baseValue) {
    std::uniform_int_distribution<int64_t> dist(-baseValue * m_jitterPercentage / 100, 
                                                  baseValue * m_jitterPercentage / 100);
    return dist(m_rd);
}

// ============================================
// BatterySimulator Implementation
// ============================================
BatterySimulator::BatterySimulator()
    : m_rd(), m_gen(m_rd()),
      m_batteryLevel(85),
      m_charging(false),
      m_health(95),
      m_temperature(28.0f),
      m_voltage(4.0f),
      m_currentNow(-500),
      m_capacity(4500),
      m_screenOn(true),
      m_wifiOn(true),
      m_gpsOn(false),
      m_bluetoothOn(false),
      m_cellularOn(true),
      m_realisticDrain(true),
      m_lastUpdate(0) {}

int BatterySimulator::getBatteryLevel() {
    return m_batteryLevel;
}

bool BatterySimulator::isCharging() {
    return m_charging;
}

int BatterySimulator::getBatteryHealth() {
    return m_health;
}

float BatterySimulator::getTemperature() {
    return m_temperature + generateRealisticNoise(0.5f);
}

float BatterySimulator::getVoltage() {
    // Voltage varies with battery level
    float baseVoltage = 3.7f + (m_batteryLevel / 100.0f) * 0.5f;
    return baseVoltage + generateRealisticNoise(0.02f);
}

int BatterySimulator::getCurrentNow() {
    if (m_charging) {
        return 1000 + m_rd() % 500;  // Positive when charging
    }
    return m_currentNow + calculateDrain();
}

int BatterySimulator::getCapacity() {
    return m_capacity;
}

void BatterySimulator::setScreenOn(bool on) {
    m_screenOn = on;
}

void BatterySimulator::setWIFIOn(bool on) {
    m_wifiOn = on;
}

void BatterySimulator::setGPSOn(bool on) {
    m_gpsOn = on;
}

void BatterySimulator::setBluetoothOn(bool on) {
    m_bluetoothOn = on;
}

void BatterySimulator::setCellularOn(bool on) {
    m_cellularOn = on;
}

void BatterySimulator::update(uint64_t deltaMs) {
    if (m_charging) {
        // Charging rate: ~1% per 2-3 minutes
        m_batteryLevel = std::min(100, m_batteryLevel + (int)(deltaMs / 180000));
        m_temperature += 0.01f;  // Slight heating
    } else {
        // Realistic battery drain
        int drain = calculateDrain();
        m_batteryLevel = std::max(0, m_batteryLevel + drain);
        
        // Temperature cooling
        m_temperature = std::max(25.0f, m_temperature - 0.001f);
    }
}

int BatterySimulator::calculateDrain() {
    int drainPerHour = 0;
    
    if (m_screenOn) drainPerHour += 300;  // Screen
    if (m_wifiOn) drainPerHour += 100;     // WiFi
    if (m_gpsOn) drainPerHour += 200;      // GPS
    if (m_bluetoothOn) drainPerHour += 50; // Bluetooth
    if (m_cellularOn) drainPerHour += 150; // Cellular
    
    // Add random variation
    drainPerHour += m_rd() % 50;
    
    // Convert to drain per update (assuming 1 minute update)
    return -drainPerHour / 60;
}

void BatterySimulator::setBatteryLevel(int level) {
    m_batteryLevel = std::max(0, std::min(100, level));
}

void BatterySimulator::setCharging(bool charging) {
    m_charging = charging;
    if (charging) {
        m_temperature = 30.0f;  // Start warm
    }
}

void BatterySimulator::setHealth(int health) {
    m_health = std::max(0, std::min(100, health));
}

float BatterySimulator::generateRealisticNoise(float stddev) {
    std::normal_distribution<float> dist(0.0f, stddev);
    return dist(m_gen);
}

int BatterySimulator::calculateDrain() {
    return calculateDrain();
}

} // namespace VirtualPhonePro
