#pragma once

/**
 * MediaDeviceSpoofer - Browser/Media Device Identity Spoofing
 * 
 * Generates unique hardware IDs for camera, microphone, and audio
 * that make each profile appear to be a different physical device.
 * 
 * Real phones have different:
 * - Camera IDs (front/rear)
 * - Microphone IDs
 * - Audio output device IDs
 * - Video capture device IDs
 * 
 * These are reported via:
 * - navigator.mediaDevices.enumerateDevices()
 * - WebRTC getUserMedia()
 * - MediaRecorder API
 * 
 * Protection Level: Enterprise
 */

#include "../VirtualPhonePro.hpp"
#include <vector>
#include <map>

namespace VirtualPhonePro {
namespace AntiDetect {

// Media device types
enum class MediaDeviceType {
    CAMERA_FRONT,
    CAMERA_REAR,
    MICROPHONE,
    AUDIO_OUTPUT,
    VIDEO_INPUT
};

// Individual media device
struct MediaDevice {
    MediaDeviceType type;
    std::string deviceId;      // Unique hex ID
    std::string label;         // Human readable name
    std::string groupId;       // Same device group
    std::string manufacturer;   // Camera/audio brand
    std::string model;          // Device model
    std::string serialNumber;   // Serial number
    bool facingMode;            // user/environment
    
    // Technical specs
    int width = 0;
    int height = 0;
    int fps = 30;
    int sampleRate = 48000;
    int channels = 2;
    
    // WebRTC fingerprint data
    std::string webrtcFingerprint;
};

// Complete media profile for a device
struct MediaProfile {
    std::vector<MediaDevice> cameras;
    std::vector<MediaDevice> microphones;
    std::vector<MediaDevice> audioOutputs;
    
    // Aggregated fingerprints
    std::string enumerateDevicesHash;
    std::string getUserMediaHash;
};

class MediaDeviceSpoofer {
public:
    static MediaDeviceSpoofer& getInstance();
    
    // Initialize with device profile
    bool initialize(const std::string& deviceModel);
    
    // Generate unique media devices for this profile
    MediaProfile generateProfile(const std::string& deviceModel);
    
    // Get device list (for WebRTC/navigator.mediaDevices)
    std::vector<MediaDevice> getAllDevices();
    
    // Get devices by type
    std::vector<MediaDevice> getCameras();
    std::vector<MediaDevice> getMicrophones();
    std::vector<MediaDevice> getAudioOutputs();
    
    // Generate JavaScript for browser spoofing
    std::string generateJSOverride();
    
    // Get enumerateDevices() output
    std::string getEnumerateDevicesOutput();
    
    // Get MediaStreamTrack info
    std::string getMediaStreamTrackInfo();
    
    // WebRTC ICE candidate generation
    std::string generateICEUsernameFragment();
    std::string generateICEPassword();
    
    // MAC address spoofing for media devices
    std::string generateMediaDeviceMAC();
    
    // Get device unique ID (for various APIs)
    std::string getDeviceId();
    
    // Audio context fingerprint
    std::string getAudioContextFingerprint();
    
    // Generate AudioWorklet processor hash
    std::string getAudioWorkletHash();
    
private:
    MediaDeviceSpoofer();
    ~MediaDeviceSpoofer();
    MediaDeviceSpoofer(const MediaDeviceSpoofer&) = delete;
    MediaDeviceSpoofer& operator=(const MediaDeviceSpoofer&) = delete;
    
    // Current profile
    MediaProfile m_currentProfile;
    
    // Device ID counters
    uint32_t m_cameraCounter;
    uint32_t m_micCounter;
    uint32_t m_audioCounter;
    
    // Generate random hex ID
    std::string generateHexId(int length);
    
    // Generate manufacturer
    std::string generateManufacturer(const std::string& deviceModel);
    
    // Generate camera device
    MediaDevice createCamera(bool front, const std::string& model);
    
    // Generate microphone device  
    MediaDevice createMicrophone(const std::string& model);
    
    // Generate audio output device
    MediaDevice createAudioOutput(const std::string& model);
    
    // Camera specs by device type
    void getCameraSpecs(const std::string& model, int& w, int& h, int& fps);
    
    // Brand-specific camera naming
    std::string getCameraBrandName(const std::string& deviceModel);
    
    // Generate realistic serial numbers
    std::string generateSerialNumber(const std::string& prefix);
    
    // WebRTC fingerprint components
    std::string buildWebRTCFingerprint(const MediaDevice& device);
};

} // namespace AntiDetect
} // namespace VirtualPhonePro
