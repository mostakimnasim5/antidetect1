/**
 * MediaDeviceSpoofer - Browser/Media Device Identity Spoofing
 * 
 * Generates unique hardware IDs for camera, microphone, and audio.
 */

#include "anti_detect/MediaDeviceSpoofer.hpp"
#include "core/CryptoUtils.hpp"
#include <sstream>
#include <iomanip>

namespace VirtualPhonePro {
namespace AntiDetect {

MediaDeviceSpoofer& MediaDeviceSpoofer::getInstance() {
    static MediaDeviceSpoofer instance;
    return instance;
}

MediaDeviceSpoofer::MediaDeviceSpoofer()
    : m_cameraCounter(0)
    , m_micCounter(0)
    , m_audioCounter(0)
{
}

MediaDeviceSpoofer::~MediaDeviceSpoofer() = default;

bool MediaDeviceSpoofer::initialize(const std::string& deviceModel) {
    m_currentProfile = generateProfile(deviceModel);
    return true;
}

std::string MediaDeviceSpoofer::generateHexId(int length) {
    auto bytes = Crypto::SecureRandomGenerator().generateRandomBytes(length / 2 + 1);
    std::ostringstream oss;
    for (size_t i = 0; i < length / 2 && i < bytes.size(); i++) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)bytes[i];
    }
    return oss.str();
}

std::string MediaDeviceSpoofer::generateManufacturer(const std::string& deviceModel) {
    if (deviceModel.find("Samsung") != std::string::npos || 
        deviceModel.find("Galaxy") != std::string::npos) {
        return "Samsung";
    } else if (deviceModel.find("Pixel") != std::string::npos) {
        return "Google";
    } else if (deviceModel.find("Xiaomi") != std::string::npos || 
               deviceModel.find("Redmi") != std::string::npos ||
               deviceModel.find("POCO") != std::string::npos) {
        return "Xiaomi";
    } else if (deviceModel.find("OnePlus") != std::string::npos) {
        return "OnePlus";
    } else if (deviceModel.find("Huawei") != std::string::npos) {
        return "Huawei";
    } else if (deviceModel.find("OPPO") != std::string::npos) {
        return "OPPO";
    } else if (deviceModel.find("Vivo") != std::string::npos) {
        return "Vivo";
    }
    return "Generic";
}

std::string MediaDeviceSpoofer::getCameraBrandName(const std::string& deviceModel) {
    std::string mfr = generateManufacturer(deviceModel);
    
    // Real camera module brands
    std::map<std::string, std::vector<std::string>> cameraBrands = {
        {"Samsung", {"Samsung ISPCELL", "Samsung LSI", "Sony IMX", "ISOCELL"}},
        {"Google", {"Sony IMX", "Samsung ISPCELL", "OmniVision"}},
        {"Xiaomi", {"Sony IMX", "Samsung ISOCELL", "OmniVision", "Hynix"}},
        {"OnePlus", {"Sony IMX", "Samsung ISPCELL", "OmniVision"}},
        {"Huawei", {"Sony IMX", "Leica", "Samsung ISPCELL"}},
        {"Generic", {"Sony IMX", "Samsung ISOCELL", "OmniVision"}}
    };
    
    auto it = cameraBrands.find(mfr);
    if (it != cameraBrands.end()) {
        auto& brands = it->second;
        size_t idx = Crypto::SecureRandomGenerator().generateUint32() % brands.size();
        return brands[idx];
    }
    return "Sony IMX";
}

void MediaDeviceSpoofer::getCameraSpecs(const std::string& deviceModel, int& w, int& h, int& fps) {
    // High-end devices
    if (deviceModel.find("Ultra") != std::string::npos || 
        deviceModel.find("Pro") != std::string::npos) {
        w = 8160; h = 6120; fps = 30; // 108MP
        if (deviceModel.find("S24") != std::string::npos || 
            deviceModel.find("Pixel 8") != std::string::npos) {
            w = 8192; h = 6144; fps = 30; // 50MP+
        }
    }
    // Mid-range
    else if (deviceModel.find("S23") != std::string::npos || 
             deviceModel.find("Pixel 7") != std::string::npos) {
        w = 8064; h = 6048; fps = 30; // 48MP
    }
    // Standard
    else {
        w = 4032; h = 3024; fps = 30; // 12MP
    }
}

std::string MediaDeviceSpoofer::generateSerialNumber(const std::string& prefix) {
    // Real serial format varies by manufacturer
    std::ostringstream oss;
    oss << prefix;
    for (int i = 0; i < 8; i++) {
        oss << std::hex << (Crypto::SecureRandomGenerator().generateUint32() % 16);
    }
    return oss.str();
}

MediaDevice MediaDeviceSpoofer::createCamera(bool front, const std::string& model) {
    MediaDevice device;
    device.type = front ? MediaDeviceType::CAMERA_FRONT : MediaDeviceType::CAMERA_REAR;
    device.facingMode = front;
    
    // Generate unique ID
    std::string hexId = generateHexId(32);
    device.deviceId = hexId;
    
    // Group ID (front and back share)
    device.groupId = "default" + hexId.substr(0, 16);
    
    // Generate specs
    int w, h, fpsVal;
    getCameraSpecs(model, w, h, fpsVal);
    device.width = w;
    device.height = h;
    device.fps = fpsVal;
    
    // Label
    std::string cameraBrand = getCameraBrandName(model);
    if (front) {
        device.label = cameraBrand + " " + model + " Front Camera";
        device.manufacturer = cameraBrand;
        device.model = model + "_FRONT";
    } else {
        device.label = cameraBrand + " " + model + " Rear Camera";
        device.manufacturer = cameraBrand;
        device.model = model + "_REAR";
    }
    
    // Serial
    device.serialNumber = generateSerialNumber(front ? "FR" : "RR");
    
    // WebRTC fingerprint
    device.webrtcFingerprint = buildWebRTCFingerprint(device);
    
    return device;
}

MediaDevice MediaDeviceSpoofer::createMicrophone(const std::string& model) {
    MediaDevice device;
    device.type = MediaDeviceType::MICROPHONE;
    
    // Unique ID
    std::string hexId = generateHexId(32);
    device.deviceId = hexId;
    device.groupId = "mic_group_" + hexId.substr(0, 8);
    
    // Specs
    device.sampleRate = 48000;
    device.channels = 1;
    
    // Label based on manufacturer
    std::string mfr = generateManufacturer(model);
    device.label = mfr + " " + model + " Microphone";
    device.manufacturer = mfr;
    device.model = model + "_MIC";
    
    device.serialNumber = generateSerialNumber("MIC");
    device.webrtcFingerprint = buildWebRTCFingerprint(device);
    
    return device;
}

MediaDevice MediaDeviceSpoofer::createAudioOutput(const std::string& model) {
    MediaDevice device;
    device.type = MediaDeviceType::AUDIO_OUTPUT;
    
    // Unique ID
    std::string hexId = generateHexId(32);
    device.deviceId = hexId;
    device.groupId = "audio_group_" + hexId.substr(0, 8);
    
    // Specs
    device.sampleRate = 48000;
    device.channels = 2;
    
    // Label
    std::string mfr = generateManufacturer(model);
    device.label = mfr + " " + model + " Speakers";
    device.manufacturer = mfr;
    device.model = model + "_SPK";
    
    device.serialNumber = generateSerialNumber("SPK");
    device.webrtcFingerprint = buildWebRTCFingerprint(device);
    
    return device;
}

std::string MediaDeviceSpoofer::buildWebRTCFingerprint(const MediaDevice& device) {
    // Real WebRTC fingerprint is derived from DTLS certificate
    std::ostringstream data;
    data << device.deviceId << device.serialNumber << device.manufacturer;
    
    auto hash = Crypto::SecureRandomGenerator().generateHexString(40);
    return "sha-256 " + hash;
}

MediaProfile MediaDeviceSpoofer::generateProfile(const std::string& deviceModel) {
    MediaProfile profile;
    
    // Front camera
    profile.cameras.push_back(createCamera(true, deviceModel));
    
    // Rear cameras (1-3 depending on device)
    int rearCameras = 1;
    if (deviceModel.find("Ultra") != std::string::npos) {
        rearCameras = 3;
    } else if (deviceModel.find("Pro") != std::string::npos || 
               deviceModel.find("+") != std::string::npos) {
        rearCameras = 2;
    }
    
    for (int i = 0; i < rearCameras; i++) {
        profile.cameras.push_back(createCamera(false, deviceModel));
    }
    
    // Microphone
    profile.microphones.push_back(createMicrophone(deviceModel));
    // Some devices have multiple mics
    if (deviceModel.find("Ultra") != std::string::npos ||
        deviceModel.find("Pro") != std::string::npos) {
        profile.microphones.push_back(createMicrophone(deviceModel));
    }
    
    // Audio output (speakers)
    profile.audioOutputs.push_back(createAudioOutput(deviceModel));
    
    return profile;
}

std::vector<MediaDevice> MediaDeviceSpoofer::getAllDevices() {
    std::vector<MediaDevice> all;
    
    for (const auto& cam : m_currentProfile.cameras) {
        all.push_back(cam);
    }
    for (const auto& mic : m_currentProfile.microphones) {
        all.push_back(mic);
    }
    for (const auto& aud : m_currentProfile.audioOutputs) {
        all.push_back(aud);
    }
    
    return all;
}

std::vector<MediaDevice> MediaDeviceSpoofer::getCameras() {
    return m_currentProfile.cameras;
}

std::vector<MediaDevice> MediaDeviceSpoofer::getMicrophones() {
    return m_currentProfile.microphones;
}

std::vector<MediaDevice> MediaDeviceSpoofer::getAudioOutputs() {
    return m_currentProfile.audioOutputs;
}

std::string MediaDeviceSpoofer::generateJSOverride() {
    std::ostringstream js;
    
    js << "(() => { ";
    js << "const devices = " << getEnumerateDevicesOutput() << "; ";
    js << R"(
        navigator.mediaDevices.enumerateDevices = async () => {
            return devices.map(d => ({
                deviceId: d.deviceId,
                groupId: d.groupId,
                kind: d.kind,
                label: d.label
            }));
        };
        
        MediaStreamTrack.prototype.getSettings = function() {
            const self = this;
            return {
                deviceId: self.deviceId,
                groupId: self.groupId,
                width: self._width || 1920,
                height: self._height || 1080,
                frameRate: self._fps || 30,
                facingMode: self._facing || 'user'
            };
        };
    )";
    
    js << "})();";
    
    return js.str();
}

std::string MediaDeviceSpoofer::getEnumerateDevicesOutput() {
    std::ostringstream json;
    json << "[";
    
    bool first = true;
    
    for (const auto& cam : m_currentProfile.cameras) {
        if (!first) json << ",";
        first = false;
        json << "{";
        json << "\"deviceId\":\"" << cam.deviceId << "\",";
        json << "\"groupId\":\"" << cam.groupId << "\",";
        json << "\"kind\":\"videoinput\",";
        json << "\"label\":\"" << cam.label << "\"";
        json << "}";
    }
    
    for (const auto& mic : m_currentProfile.microphones) {
        if (!first) json << ",";
        first = false;
        json << "{";
        json << "\"deviceId\":\"" << mic.deviceId << "\",";
        json << "\"groupId\":\"" << mic.groupId << "\",";
        json << "\"kind\":\"audioinput\",";
        json << "\"label\":\"" << mic.label << "\"";
        json << "}";
    }
    
    for (const auto& aud : m_currentProfile.audioOutputs) {
        if (!first) json << ",";
        first = false;
        json << "{";
        json << "\"deviceId\":\"" << aud.deviceId << "\",";
        json << "\"kind\":\"audiooutput\",";
        json << "\"label\":\"" << aud.label << "\"";
        json << "}";
    }
    
    json << "]";
    return json.str();
}

std::string MediaDeviceSpoofer::getMediaStreamTrackInfo() {
    std::ostringstream info;
    info << "{";
    
    auto cameras = getCameras();
    if (!cameras.empty()) {
        auto& cam = cameras[0];
        info << "\"video\":{";
        info << "\"deviceId\":\"" << cam.deviceId << "\",";
        info << "\"width\":" << cam.width << ",";
        info << "\"height\":" << cam.height << ",";
        info << "\"frameRate\":" << cam.fps;
        info << "}";
    }
    
    auto mics = getMicrophones();
    if (!mics.empty()) {
        if (!cameras.empty()) info << ",";
        auto& mic = mics[0];
        info << "\"audio\":{";
        info << "\"deviceId\":\"" << mic.deviceId << "\",";
        info << "\"sampleRate\":" << mic.sampleRate;
        info << "}";
    }
    
    info << "}";
    return info.str();
}

std::string MediaDeviceSpoofer::generateICEUsernameFragment() {
    return generateHexId(16);
}

std::string MediaDeviceSpoofer::generateICEPassword() {
    return generateHexId(32);
}

std::string MediaDeviceSpoofer::generateMediaDeviceMAC() {
    // Real devices have manufacturer OUI + random part
    std::string mfr = generateManufacturer("default");
    
    // Manufacturer prefixes (real OUI prefixes)
    std::map<std::string, std::string> oui = {
        {"Samsung", "FC:A1:83"},
        {"Google", "F4:F5:D8"},
        {"Xiaomi", "64:09:80"},
        {"OnePlus", "2A:6B:AD"},
        {"Huawei", "00:25:9E"},
        {"Apple", "3C:06:30"},
        {"Generic", "00:1A:2B"}
    };
    
    auto it = oui.find(mfr);
    std::string prefix = (it != oui.end()) ? it->second : "00:1A:2B";
    
    std::ostringstream mac;
    mac << prefix << ":";
    for (int i = 0; i < 3; i++) {
        mac << std::hex << std::setfill('0') << std::setw(2) 
            << (int)(Crypto::SecureRandomGenerator().generateUint32() % 256);
        if (i < 2) mac << ":";
    }
    
    return mac.str();
}

std::string MediaDeviceSpoofer::getDeviceId() {
    // Used for various device identification APIs
    return generateHexId(32);
}

std::string MediaDeviceSpoofer::getAudioContextFingerprint() {
    // AudioContext fingerprint varies by hardware
    auto bytes = Crypto::SecureRandomGenerator().generateRandomBytes(16);
    std::ostringstream oss;
    oss << "0x";
    for (uint8_t b : bytes) {
        oss << std::hex << std::setfill('0') << std::setw(2) << (int)b;
    }
    return oss.str();
}

std::string MediaDeviceSpoofer::getAudioWorkletHash() {
    return generateHexId(40);
}

} // namespace AntiDetect
} // namespace VirtualPhonePro
