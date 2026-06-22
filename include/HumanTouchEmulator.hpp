#pragma once

/**
 * HumanTouchEmulator.hpp
 * 
 * Simulates realistic human touch and swipe events to bypass behavioral AI detection.
 * Uses Cubic Bezier Curves, micro-jitters, Gaussian delays, and pressure simulation.
 * 
 * Author: AntiDetectPro
 * License: Commercial - Elite Level
 */

#ifndef HUMAN_TOUCH_EMULATOR_HPP
#define HUMAN_TOUCH_EMULATOR_HPP

#include <cstdint>
#include <vector>
#include <random>
#include <chrono>
#include <functional>
#include <string>
#include <memory>
#include <array>
#include <optional>

namespace AntiDetect {

/**
 * Touch event types for Android input system
 */
enum class TouchAction : int32_t {
    DOWN     = 0,    // Touch down event
    UP       = 1,    // Touch up event  
    MOVE     = 2,    // Touch move event
    CANCEL   = 3     // Touch cancel event
};

/**
 * Point structure for touch coordinates with additional metadata
 */
struct TouchPoint {
    int32_t x;           // X coordinate (pixels)
    int32_t y;           // Y coordinate (pixels)
    int32_t pressure;    // Touch pressure (0-255, typically)
    int32_t size;        // Touch size area (0-255, typically)
    int64_t timestamp;   // Timestamp in nanoseconds
    int32_t slot;        // Touch slot (for multi-touch)
    
    TouchPoint() : x(0), y(0), pressure(128), size(50), timestamp(0), slot(0) {}
    TouchPoint(int32_t x_, int32_t y_) : x(x_), y(y_), pressure(128), size(50), timestamp(0), slot(0) {}
};

/**
 * Swipe gesture configuration
 */
struct SwipeConfig {
    // Bezier curve control points multiplier (0.0 to 1.0)
    // Higher values = more curved path
    float curveIntensity = 0.3f;
    
    // Micro-jitter configuration
    float jitterMaxPixels = 3.0f;      // Maximum jitter in pixels
    float jitterProbability = 0.4f;    // Probability of jitter occurring
    
    // Timing configuration
    float minDelayMs = 5.0f;           // Minimum delay between points
    float maxDelayMs = 25.0f;          // Maximum delay between points
    float delayMeanMs = 12.0f;         // Gaussian mean delay
    float delayStdDevMs = 4.0f;        // Gaussian standard deviation
    
    // Pressure configuration
    int32_t minPressure = 80;          // Minimum pressure (0-255)
    int32_t maxPressure = 200;         // Maximum pressure (0-255)
    float pressureVariation = 0.15f;   // Pressure variation factor
    
    // Size configuration
    int32_t minSize = 30;              // Minimum touch size
    int32_t maxSize = 80;              // Maximum touch size
    float sizeVariation = 0.2f;        // Size variation factor
    
    // Speed configuration
    float swipeSpeedMultiplier = 1.0f; // Overall speed multiplier
    
    // Natural behavior
    bool addPauseAtEnd = true;         // Add slight pause at swipe end
    bool accelerateAtStart = true;     // Accelerate from rest at start
};

/**
 * Configuration for tap gestures
 */
struct TapConfig {
    int32_t durationMs = 100;          // Tap duration
    int32_t holdProbability = 0.3f;    // Probability of long press
    int32_t longPressMinMs = 300;      // Minimum long press duration
    int32_t longPressMaxMs = 800;      // Maximum long press duration
    
    // Tap pressure
    int32_t tapPressure = 150;         // Average tap pressure
    int32_t tapSize = 45;              // Average tap size
};

/**
 * Raw input event for Linux event subsystem (/dev/input/eventX)
 */
struct InputEvent {
    struct timeval time;
    __u16 type;        // EV_SYN, EV_ABS, EV_KEY
    __u16 code;        // ABS_MT_* or BTN_TOUCH
    __s32 value;       // Coordinate or state value
    
    std::vector<uint8_t> serialize() const;
};

/**
 * ADB tap/swipe command format
 */
struct ADBCommand {
    std::string command;
    
    // Generate ADB shell command for tap
    static std::string makeTapCommand(int x, int y, int durationMs = 0);
    
    // Generate ADB shell command for swipe
    static std::string makeSwipeCommand(int x1, int y1, int x2, int y2, int durationMs);
    
    // Generate ADB shell command for multi-touch
    static std::string makeMultiTouchCommand(const std::vector<TouchPoint>& points);
};

/**
 * HumanTouchEmulator - Realistic touch simulation for Android automation
 * 
 * Features:
 * - Cubic Bezier curve path generation for natural swipes
 * - Micro-jitter injection to simulate hand shaking
 * - Gaussian-distributed delays for human hesitation
 * - Dynamic pressure and size variation
 * - Multi-touch support
 * - Output via Linux input events or ADB protocol
 */
class HumanTouchEmulator {
public:
    /**
     * Constructor
     * @param screenWidth Screen width in pixels
     * @param screenHeight Screen height in pixels
     */
    HumanTouchEmulator(int32_t screenWidth, int32_t screenHeight);
    
    /**
     * Destructor
     */
    ~HumanTouchEmulator();
    
    // Prevent copying
    HumanTouchEmulator(const HumanTouchEmulator&) = delete;
    HumanTouchEmulator& operator=(const HumanTouchEmulator&) = delete;
    
    /**
     * Generate a realistic swipe path using Cubic Bezier curves
     * 
     * @param startX Start X coordinate
     * @param startY Start Y coordinate  
     * @param endX End X coordinate
     * @param endY End Y coordinate
     * @param config Swipe configuration
     * @return Vector of touch points along the path
     */
    std::vector<TouchPoint> generateSwipePath(
        int32_t startX, int32_t startY,
        int32_t endX, int32_t endY,
        const SwipeConfig& config = SwipeConfig{}
    );
    
    /**
     * Generate a realistic tap gesture
     * 
     * @param x Tap X coordinate
     * @param y Tap Y coordinate
     * @param config Tap configuration
     * @return Vector of touch points for the tap
     */
    std::vector<TouchPoint> generateTap(
        int32_t x, int32_t y,
        const TapConfig& config = TapConfig{}
    );
    
    /**
     * Generate a multi-touch gesture (pinch/zoom)
     * 
     * @param points1 First finger path
     * @param points2 Second finger path
     * @param config Swipe configuration
     * @return Vector of combined touch points
     */
    std::vector<std::vector<TouchPoint>> generateMultiTouchPath(
        const std::vector<TouchPoint>& points1,
        const std::vector<TouchPoint>& points2,
        const SwipeConfig& config = SwipeConfig{}
    );
    
    /**
     * Add micro-jitter to a path for natural hand movement
     * 
     * @param points Input touch points
     * @param maxJitter Maximum jitter in pixels
     * @param probability Probability of jitter occurring (0.0-1.0)
     * @return Points with jitter added
     */
    std::vector<TouchPoint> addMicroJitter(
        const std::vector<TouchPoint>& points,
        float maxJitter,
        float probability
    );
    
    /**
     * Apply Gaussian-distributed delays to touch points
     * Simulates human reaction time and hesitation
     * 
     * @param points Input touch points
     * @param meanMs Mean delay in milliseconds
     * @param stdDevMs Standard deviation in milliseconds
     * @return Points with timestamps
     */
    std::vector<TouchPoint> applyGaussianDelays(
        const std::vector<TouchPoint>& points,
        float meanMs,
        float stdDevMs
    );
    
    /**
     * Simulate varying pressure and touch size during a gesture
     * 
     * @param points Input touch points
     * @param minPressure Minimum pressure value
     * @param maxPressure Maximum pressure value
     * @param minSize Minimum touch size
     * @param maxSize Maximum touch size
     * @return Points with simulated pressure/size
     */
    std::vector<TouchPoint> simulatePressureAndSize(
        const std::vector<TouchPoint>& points,
        int32_t minPressure, int32_t maxPressure,
        int32_t minSize, int32_t maxSize
    );
    
    /**
     * Generate complete realistic swipe with all effects
     * 
     * @param startX Start X coordinate
     * @param startY Start Y coordinate
     * @param endX End X coordinate
     * @param endY End Y coordinate
     * @param config Swipe configuration
     * @return Complete swipe gesture with all effects
     */
    std::vector<TouchPoint> generateRealisticSwipe(
        int32_t startX, int32_t startY,
        int32_t endX, int32_t endY,
        const SwipeConfig& config = SwipeConfig{}
    );
    
    /**
     * Convert touch points to Linux input events
     * Format compatible with /dev/input/eventX
     * 
     * @param points Touch points to convert
     * @param slot Touch slot for multi-touch
     * @return Vector of input events
     */
    std::vector<InputEvent> toInputEvents(
        const std::vector<TouchPoint>& points,
        int32_t slot = 0
    );
    
    /**
     * Generate ADB command string for swipe
     * 
     * @param startX Start X coordinate
     * @param startY Start Y coordinate
     * @param endX End X coordinate
     * @param endY End Y coordinate
     * @param durationMs Total swipe duration in milliseconds
     * @return ADB shell input swipe command
     */
    std::string toADBCommand(
        int32_t startX, int32_t startY,
        int32_t endX, int32_t endY,
        int32_t durationMs
    );
    
    /**
     * Generate ADB command string for tap
     * 
     * @param x Tap X coordinate
     * @param y Tap Y coordinate
     * @param durationMs Tap duration (0 for instant)
     * @return ADB shell input tap command
     */
    std::string toADBTapCommand(int32_t x, int32_t y, int32_t durationMs = 0);
    
    /**
     * Execute swipe via ADB
     * 
     * @param startX Start X coordinate
     * @param startY Start Y coordinate
     * @param endX End X coordinate
     * @param endY End Y coordinate
     * @param durationMs Total swipe duration in milliseconds
     * @return true if successful
     */
    bool executeSwipeViaADB(
        int32_t startX, int32_t startY,
        int32_t endX, int32_t endY,
        int32_t durationMs
    );
    
    /**
     * Execute tap via ADB
     * 
     * @param x Tap X coordinate
     * @param y Tap Y coordinate
     * @param durationMs Tap duration (0 for instant)
     * @return true if successful
     */
    bool executeTapViaADB(int32_t x, int32_t y, int32_t durationMs = 0);
    
    /**
     * Set callback for ADB command execution
     * If not set, commands are only generated but not executed
     */
    void setADBExecutor(std::function<bool(const std::string&)> executor);
    
    /**
     * Generate random fingerprint variation for touch behavior
     * Each device has slightly different touch characteristics
     */
    void randomizeTouchProfile();
    
    /**
     * Get current touch profile parameters
     */
    struct TouchProfile {
        float basePressure;
        float pressureVariance;
        float baseSize;
        float sizeVariance;
        float baseSpeed;
        float speedVariance;
        float jitterTendency;
    };
    TouchProfile getTouchProfile() const;
    
    /**
     * Screen dimensions
     */
    int32_t getScreenWidth() const { return m_screenWidth; }
    int32_t getScreenHeight() const { return m_screenHeight; }

private:
    int32_t m_screenWidth;
    int32_t m_screenHeight;
    
    std::mt19937 m_randomEngine;
    std::normal_distribution<float> m_gaussianDelay;
    std::uniform_real_distribution<float> m_uniform;
    
    std::function<bool(const std::string&)> m_adbExecutor;
    
    TouchProfile m_touchProfile;
    
    /**
     * Calculate Cubic Bezier point at parameter t
     */
    TouchPoint cubicBezierPoint(
        float t,
        const TouchPoint& p0,
        const TouchPoint& p1,
        const TouchPoint& p2,
        const TouchPoint& p3
    );
    
    /**
     * Generate Bezier control points for natural curve
     */
    void generateBezierControlPoints(
        int32_t startX, int32_t startY,
        int32_t endX, int32_t endY,
        float curveIntensity,
        TouchPoint& p1, TouchPoint& p2
    );
    
    /**
     * Interpolate between two points
     */
    TouchPoint lerp(const TouchPoint& a, const TouchPoint& b, float t);
    
    /**
     * Add natural acceleration at gesture start
     */
    std::vector<TouchPoint> addStartAcceleration(
        const std::vector<TouchPoint>& points,
        float accelerationFactor
    );
    
    /**
     * Add pause at gesture end
     */
    std::vector<TouchPoint> addEndPause(
        const std::vector<TouchPoint>& points,
        float pauseDurationMs
    );
    
    /**
     * Clamp coordinates to screen bounds
     */
    TouchPoint clampToScreen(const TouchPoint& point);
    
    /**
     * Get current timestamp in nanoseconds
     */
    int64_t getCurrentTimestampNs();
};

} // namespace AntiDetect

#endif // HUMAN_TOUCH_EMULATOR_HPP