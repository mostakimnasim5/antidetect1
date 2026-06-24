/**
 * HardwareEmulator - Complete Hardware Emulation Layer
 */

#include "hardware/HardwareEmulator.hpp"
#include <random>
#include <sstream>
#include <iomanip>

namespace VirtualPhonePro {

HardwareEmulator::HardwareEmulator() {
    initializeSensorNoise();
    generateUUID();
    generateSerial();
}

HardwareEmulator::~HardwareEmulator() {}

HardwareEmulator& HardwareEmulator::getInstance() {
    static HardwareEmulator instance;
    return instance;
}

bool HardwareEmulator::initialize() {
    std::cout << "[OK] Hardware Emulator initialized" << std::endl;
    return true;
}

void HardwareEmulator::shutdown() {
    std::cout << "[OK] Hardware Emulator shutdown" << std::endl;
}

void HardwareEmulator::setSMBIOSSystem(const SMBIOSSystem& sys) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_smbiosSystem = sys;
}

void HardwareEmulator::setSMBIOSProcessor(const SMBIOSProcessor& cpu) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_smbiosProcessor = cpu;
}

void HardwareEmulator::setSMBIOSBaseboard(const SMBIOSBaseboard& board) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_smbiosBaseboard = board;
}

void HardwareEmulator::setSMBIOSBIOS(const SMBIOSBIOS& bios) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_smbiosBIOS = bios;
}

SMBIOSSystem HardwareEmulator::getSMBIOSSystem() const {
    return m_smbiosSystem;
}

SMBIOSProcessor HardwareEmulator::getSMBIOSProcessor() const {
    return m_smbiosProcessor;
}

SMBIOSBaseboard HardwareEmulator::getSMBIOSBaseboard() const {
    return m_smbiosBaseboard;
}

SMBIOSBIOS HardwareEmulator::getSMBIOSBIOS() const {
    return m_smbiosBIOS;
}

void HardwareEmulator::setCPUModel(const std::string& model, int cores, int threads) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_cpuModel = model;
    m_cpuCores = cores;
    m_cpuThreads = threads;
}

std::string HardwareEmulator::getCPUModel() const {
    return m_cpuModel;
}

CPUIDResult HardwareEmulator::executeCPUID(uint32_t function) {
    CPUIDResult result = {0, 0, 0, 0};
    
    #ifdef __x86_64__
    __asm__ volatile (
        "cpuid"
        : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx)
        : "a"(function)
    );
    #endif
    
    return result;
}

void HardwareEmulator::setGPUInfo(const std::string& vendor, const std::string& renderer) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_gpuVendor = vendor;
    m_gpuRenderer = renderer;
    m_gpuVersion = "Vulkan 1.2";
}

std::string HardwareEmulator::getGPUVendor() const {
    return m_gpuVendor;
}

std::string HardwareEmulator::getGPURenderer() const {
    return m_gpuRenderer;
}

std::string HardwareEmulator::getGPUVersion() const {
    return m_gpuVersion;
}

void HardwareEmulator::setBootloaderInfo(const std::string& version, const std::string& bootloader) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_bootloaderVersion = bootloader;
}

std::string HardwareEmulator::getBootloaderVersion() const {
    return m_bootloaderVersion;
}

void HardwareEmulator::setSensorData(const std::string& sensorType, float x, float y, float z) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_sensorData[sensorType] = {x, y, z};
}

std::map<std::string, std::array<float, 3>> HardwareEmulator::getAllSensorData() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_sensorData;
}

void HardwareEmulator::setDeviceInfo(const FingerprintConfig& config) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    m_deviceProperties["ro.product.manufacturer"] = config.manufacturer;
    m_deviceProperties["ro.product.model"] = config.model;
    m_deviceProperties["ro.product.brand"] = config.brand;
    m_deviceProperties["ro.build.version.release"] = config.androidVersion;
    m_deviceProperties["ro.build.version.security_patch"] = config.securityPatch;
    m_deviceProperties["ro.build.id"] = config.buildId;
    m_deviceProperties["ro.bootloader"] = config.bootloader;
    m_deviceProperties["ro.hardware"] = config.hardware;
    m_deviceProperties["ro.product.device"] = config.device;
    m_deviceProperties["ro.product.product"] = config.product;
    m_deviceProperties["ro.build.fingerprint"] = config.fingerprint;
}

std::map<std::string, std::string> HardwareEmulator::getAllDeviceProperties() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_deviceProperties;
}

void HardwareEmulator::applyProfile(const FingerprintConfig& profile) {
    setDeviceInfo(profile);
    setGPUInfo(profile.gpuVendor, profile.gpuRenderer);
    setBootloaderInfo(profile.bootloader, profile.bootloader);
    
    // SMBIOS
    SMBIOSSystem sys;
    sys.manufacturer = profile.manufacturer;
    sys.productName = profile.model;
    sys.serialNumber = generateSerial();
    sys.uuid = m_smbiosSystem.uuid;
    setSMBIOSSystem(sys);
    
    SMBIOSProcessor cpu;
    cpu.manufacturer = profile.manufacturer;
    cpu.version = profile.cpuModel;
    cpu.coreCount = profile.cpuCores;
    cpu.threadCount = profile.cpuCores * 2;
    setSMBIOSProcessor(cpu);
}

void HardwareEmulator::generateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    
    std::stringstream ss;
    for (int i = 0; i < 16; i++) {
        ss << std::hex << std::setw(2) << std::setfill('0') << dis(gen);
        if (i == 3 || i == 5 || i == 7 || i == 9) ss << "-";
    }
    
    m_smbiosSystem.uuid = ss.str();
}

std::string HardwareEmulator::generateSerial() {
    const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 35);
    
    std::string serial;
    for (int i = 0; i < 12; i++) {
        serial += chars[dis(gen)];
    }
    
    return serial;
}

void HardwareEmulator::initializeSensorNoise() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> noise(0.0f, 0.01f);
    
    // Initialize noise for each sensor
    for (const auto& sensor : {"accelerometer", "gyroscope", "magnetometer"}) {
        m_sensorNoise[sensor] = {noise(gen), noise(gen), noise(gen)};
    }
}

// ============================================
// CPUEmulator Implementation
// ============================================
CPUEmulator::CPUEmulator() : m_jitterPercentage(0) {}

void CPUEmulator::setIntelSkylake() {
    m_vendor = "GenuineIntel";
    m_brand = "Intel(R) Core(TM) i7-10700K CPU @ 3.80GHz";
    m_family = 6;
    m_model = 165;
    m_stepping = 5;
}

void CPUEmulator::setAMDZen3() {
    m_vendor = "AuthenticAMD";
    m_brand = "AMD Ryzen 9 5900X 12-Core Processor";
    m_family = 25;
    m_model = 33;
    m_stepping = 0;
}

void CPUEmulator::setARMCortexA78() {
    m_vendor = "ARM";
    m_brand = "ARM Cortex-A78";
    m_family = 0;
    m_model = 0;
    m_stepping = 0;
}

void CPUEmulator::setCustomCPUID(const std::string& vendor, const std::string& brand) {
    m_vendor = vendor;
    m_brand = brand;
}

CPUIDResult CPUEmulator::getCPUID(uint32_t function) {
    CPUIDResult result = {0, 0, 0, 0};
    
    // Return spoofed values
    if (function == 0) {
        result.eax = 13;
        // Vendor string in ebx, edx, ecx
        std::string vendor = m_vendor;
        result.ebx = *reinterpret_cast<uint32_t*>(vendor.data());
        result.edx = *reinterpret_cast<uint32_t*>(vendor.data() + 4);
        result.ecx = *reinterpret_cast<uint32_t*>(vendor.data() + 8);
    } else if (function == 1) {
        result.eax = (m_family << 8) | (m_model << 4) | m_stepping;
        result.ebx = 0x400; // 4 cores
        result.ecx = 0x7FF << 11; // SSE, SSE2, SSE3, SSSE3, SSE4.1, SSE4.2, x2APIC
        result.edx = 0x178bfbff; // Standard features
    }
    
    return result;
}

CPUIDResult CPUEmulator::getCPUIDExt(uint32_t function, uint32_t subfunction) {
    CPUIDResult result = {0, 0, 0, 0};
    return result;
}

void CPUEmulator::setTimingProfile(const std::string& profile) {
    if (profile == "flagship") {
        m_jitterPercentage = 5;
    } else if (profile == "midrange") {
        m_jitterPercentage = 15;
    } else if (profile == "gaming") {
        m_jitterPercentage = 10;
    }
}

uint64_t CPUEmulator::getRDTSCTiming() {
    #ifdef __x86_64__
    unsigned int lo, hi;
    __asm__ __volatile__ ("rdtsc" : "=a"(lo), "=d"(hi));
    uint64_t tsc = ((uint64_t)hi << 32) | lo;
    
    // Add jitter
    if (m_jitterPercentage > 0) {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<int64_t> dist(-tsc * m_jitterPercentage / 100, 
                                                     tsc * m_jitterPercentage / 100);
        tsc += dist(gen);
    }
    
    return tsc;
    #else
    return 0;
    #endif
}

void CPUEmulator::addJitter(int percentage) {
    m_jitterPercentage = percentage;
}

// ============================================
// GPUEmulator Implementation
// ============================================
GPUEmulator::GPUEmulator() {}

void GPUEmulator::setAdreno730() {
    m_vendor = "Qualcomm";
    m_renderer = "Adreno (TM) 730";
    m_version = "3.2 Core";
    m_extensions = {"GL_OES_texture_float", "GL_OES_texture_float_linear", 
                    "GL_EXT_color_buffer_float", "GL_EXT_color_buffer_half_float"};
}

void GPUEmulator::setMaliG78() {
    m_vendor = "ARM";
    m_renderer = "Mali-G78";
    m_version = "r26p0-00dev0";
    m_extensions = {"GL_ARM_mali_shader_memory_attachment"};
}

void GPUEmulator::setMaliG710() {
    m_vendor = "ARM";
    m_renderer = "Mali-G710";
    m_version = "r32p0-00dev0";
    m_extensions = {"GL_ARM_mali_shader_memory_attachment"};
}

void GPUEmulator::setPowervr() {
    m_vendor = "Imagination Technologies";
    m_renderer = "PowerVR Rogue-XB";
    m_version = "5.0.17.2.456876";
    m_extensions = {"GL_IMG_texture_compression_pvrtc"};
}

std::string GPUEmulator::getVendorString() {
    return m_vendor;
}

std::string GPUEmulator::getRendererString() {
    return m_renderer;
}

std::string GPUEmulator::getVersionString() {
    return m_version;
}

std::string GPUEmulator::getExtensions() {
    std::string ext;
    for (const auto& e : m_extensions) {
        ext += e + " ";
    }
    return ext;
}

std::map<std::string, std::string> GPUEmulator::getWebGLParams() {
    return {
        {"vendor", m_vendor},
        {"renderer", m_renderer},
        {"version", m_version},
        {"extensions", getExtensions()}
    };
}

// ============================================
// BootloaderEmulator Implementation
// ============================================
BootloaderEmulator::BootloaderEmulator() {}

void BootloaderEmulator::setSamsungBootloader(const std::string& model) {
    std::string prefix = model.substr(0, 4);
    m_bootloader = prefix + "U1A1";
    m_radio = prefix + ".C" + std::to_string(rand() % 10 + 1) + "FT";
}

void BootloaderEmulator::setGoogleBootloader() {
    m_bootloader = "bootloader-" + std::to_string(rand() % 100 + 210) + ".0";
    m_radio = "radio-" + std::to_string(rand() % 100 + 210) + ".0";
}

void BootloaderEmulator::setXiaomiBootloader() {
    m_bootloader = "U-boot-" + std::to_string(rand() % 100 + 20) + ".0";
    m_radio = "MIX-" + std::to_string(rand() % 100 + 100) + "." + std::to_string(rand() % 10);
}

void BootloaderEmulator::setOnePlusBootloader() {
    m_bootloader = "ONEPLUS" + std::to_string(rand() % 10000 + 1000);
    m_radio = "MPSS." + std::to_string(rand() % 10) + "." + std::to_string(rand() % 100);
}

void BootloaderEmulator::setCustomBootloader(const std::string& bootloader) {
    m_bootloader = bootloader;
}

std::string BootloaderEmulator::getBootloaderVersion() {
    return m_bootloader;
}

std::string BootloaderEmulator::getRadioVersion() {
    return m_radio;
}

std::string BootloaderEmulator::generateBootSignature() {
    // Generate random boot signature
    const char* chars = "ABCDEF0123456789";
    std::string sig;
    for (int i = 0; i < 32; i++) {
        sig += chars[rand() % 16];
    }
    return sig;
}

// ============================================
// SensorEmulator Implementation
// ============================================
SensorEmulator::SensorEmulator() 
    : m_noiseEnabled(true), m_noiseProfile("stationary"),
      m_barometer(1013.25f), m_proximity(5.0f), m_light(500.0f),
      m_gpsLat(0), m_gpsLon(0), m_gpsAccuracy(1.0f) {
    m_accelerometer = {0, 0, 9.8f};
    m_gyroscope = {0, 0, 0};
    m_magnetometer = {25, 0, -45};
}

void SensorEmulator::setAccelerometer(float x, float y, float z) {
    m_accelerometer = {x, y, z};
}

std::array<float, 3> SensorEmulator::getAccelerometer() {
    if (m_noiseEnabled) addNoise(m_accelerometer);
    return m_accelerometer;
}

void SensorEmulator::setGyroscope(float x, float y, float z) {
    m_gyroscope = {x, y, z};
}

std::array<float, 3> SensorEmulator::getGyroscope() {
    if (m_noiseEnabled) addNoise(m_gyroscope);
    return m_gyroscope;
}

void SensorEmulator::setMagnetometer(float x, float y, float z) {
    m_magnetometer = {x, y, z};
}

std::array<float, 3> SensorEmulator::getMagnetometer() {
    if (m_noiseEnabled) addNoise(m_magnetometer);
    return m_magnetometer;
}

void SensorEmulator::setBarometer(float pressure) {
    m_barometer = pressure;
}

float SensorEmulator::getBarometer() {
    if (m_noiseEnabled) {
        m_barometer += generateGaussianNoise(0, 0.1f);
    }
    return m_barometer;
}

void SensorEmulator::setProximity(float distance) {
    m_proximity = distance;
}

float SensorEmulator::getProximity() {
    return m_proximity;
}

void SensorEmulator::setLight(float lux) {
    m_light = lux;
}

float SensorEmulator::getLight() {
    if (m_noiseEnabled) {
        m_light += generateGaussianNoise(0, 5.0f);
    }
    return m_light;
}

void SensorEmulator::setGPS(double lat, double lon, float accuracy) {
    m_gpsLat = lat;
    m_gpsLon = lon;
    m_gpsAccuracy = accuracy;
}

std::tuple<double, double, float> SensorEmulator::getGPS() {
    if (m_noiseEnabled) {
        m_gpsLat += generateGaussianNoise(0, 0.00001);
        m_gpsLon += generateGaussianNoise(0, 0.00001);
    }
    return {m_gpsLat, m_gpsLon, m_gpsAccuracy};
}

void SensorEmulator::enableNaturalNoise(bool enable) {
    m_noiseEnabled = enable;
}

void SensorEmulator::setNoiseProfile(const std::string& profile) {
    m_noiseProfile = profile;
}

void SensorEmulator::addNoise(std::array<float, 3>& data) {
    float stddev = 0.01f;
    if (m_noiseProfile == "walking") stddev = 0.5f;
    if (m_noiseProfile == "driving") stddev = 2.0f;
    
    data[0] += generateGaussianNoise(0, stddev);
    data[1] += generateGaussianNoise(0, stddev);
    data[2] += generateGaussianNoise(0, stddev);
}

float SensorEmulator::generateGaussianNoise(float mean, float stddev) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<float> dist(mean, stddev);
    return dist(gen);
}

} // namespace VirtualPhonePro
