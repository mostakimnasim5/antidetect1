/**
 * HumanTouchEmulator.cpp
 * 
 * Implementation of realistic human touch and swipe event simulation.
 * Uses Cubic Bezier Curves, micro-jitters, Gaussian delays, and pressure simulation.
 * 
 * Author: AntiDetectPro
 * License: Commercial - Elite Level
 */

#include "HumanTouchEmulator.hpp"
#include "Logger.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <sys/time.h>
#include <unistd.h>

namespace AntiDetect {

// ============================================================================
// Constants for Android input system
// ============================================================================

namespace InputConst {
    // Event types
    constexpr uint16_t EV_SYN = 0x00;
    constexpr uint16_t EV_ABS = 0x03;
    constexpr uint16_t EV_KEY = 0x01;
    
    // ABS codes for multi-touch
    constexpr uint16_t ABS_MT_POSITION_X = 0x35;
    constexpr uint16_t ABS_MT_POSITION_Y = 0x36;
    constexpr uint16_t ABS_MT_PRESSURE = 0x3a;
    constexpr uint16_t ABS_MT_TOUCH_MAJOR = 0x30;
    constexpr uint16_t ABS_MT_SLOT = 0x2f;
    
    // Key codes
    constexpr uint16_t BTN_TOUCH = 0x14a;
    
    // Synchronization
    constexpr int32_t SYN_REPORT = 0;
    constexpr int32_t SYN_MT_REPORT = 2;
}

// ============================================================================
// TouchPoint Implementation
// ============================================================================

// ============================================================================
// TouchEventTimeTracker - Helper class for timestamp management
// ============================================================================

class TouchEventTimeTracker {
public:
    TouchEventTimeTracker() : m_startTimeNs(0) {}
    
    void start() {
        m_startTimeNs = getCurrentTimeNs();
    }
    
    int64_t elapsedNs() const {
        return getCurrentTimeNs() - m_startTimeNs;
    }
    
    int64_t elapsedMs() const {
        return elapsedNs() / 1000000;
    }
    
private:
    static int64_t getCurrentTimeNs() {
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return static_cast<int64_t>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
    }
    
    int64_t m_startTimeNs;
};

// ============================================================================
// HumanTouchEmulator Implementation
// ============================================================================

HumanTouchEmulator::HumanTouchEmulator(int32_t screenWidth, int32_t screenHeight)
    : m_screenWidth(screenWidth)
    , m_screenHeight(screenHeight)
    , m_randomEngine(std::random_device{}())
    , m_gaussianDelay(12.0f, 4.0f)  // Mean=12ms, StdDev=4ms
    , m_uniform(0.0f, 1.0f)
{
    // Initialize default touch profile
    m_touchProfile = {
        .basePressure = 0.5f,
        .pressureVariance = 0.15f,
        .baseSize = 0.4f,
        .sizeVariance = 0.2f,
        .baseSpeed = 1.0f,
        .speedVariance = 0.1f,
        .jitterTendency = 0.4f
    };
    
    Logger::getInstance().debug("HumanTouchEmulator initialized: " + 
        std::to_string(screenWidth) + "x" + std::to_string(screenHeight));
}

HumanTouchEmulator::~HumanTouchEmulator() = default;

// ============================================================================
// Cubic Bezier Curve Implementation
// ============================================================================

/**
 * Calculate a point on a Cubic Bezier curve at parameter t (0 to 1)
 * 
 * Formula: B(t) = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
 */
TouchPoint HumanTouchEmulator::cubicBezierPoint(
    float t,
    const TouchPoint& p0,
    const TouchPoint& p1,
    const TouchPoint& p2,
    const TouchPoint& p3
) {
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    TouchPoint result;
    result.x = static_cast<int32_t>(uuu * p0.x + 3 * uu * t * p1.x + 3 * u * tt * p2.x + ttt * p3.x);
    result.y = static_cast<int32_t>(uuu * p0.y + 3 * uu * t * p1.y + 3 * u * tt * p2.y + ttt * p3.y);
    result.pressure = p0.pressure;  // Will be modified later
    result.size = p0.size;
    result.slot = p0.slot;
    
    return result;
}

/**
 * Generate control points for a natural-looking Bezier curve
 * 
 * The curve intensity determines how much the path curves away from
 * a straight line, mimicking natural human hand movement patterns.
 */
void HumanTouchEmulator::generateBezierControlPoints(
    int32_t startX, int32_t startY,
    int32_t endX, int32_t endY,
    float curveIntensity,
    TouchPoint& p1, TouchPoint& p2
) {
    // Calculate the distance between start and end points
    int32_t dx = endX - startX;
    int32_t dy = endY - startY;
    float distance = std::sqrt(static_cast<float>(dx * dx + dy * dy));
    
    // Control point offset based on curve intensity and distance
    float offsetFactor = curveIntensity * distance * 0.4f;
    
    // Determine primary direction and create perpendicular offset
    bool isHorizontal = std::abs(dx) > std::abs(dy);
    
    if (isHorizontal) {
        // For horizontal swipes, curve vertically
        float midY = (startY + endY) / 2.0f;
        float perpOffset = offsetFactor * (m_uniform(m_randomEngine) * 2.0f - 1.0f);
        
        p1.x = startX + static_cast<int32_t>(dx * 0.3f);
        p1.y = static_cast<int32_t>(midY + perpOffset * 0.5f);
        
        p2.x = startX + static_cast<int32_t>(dx * 0.7f);
        p2.y = static_cast<int32_t>(midY - perpOffset * 0.3f);
    } else {
        // For vertical swipes, curve horizontally
        float midX = (startX + endX) / 2.0f;
        float perpOffset = offsetFactor * (m_uniform(m_randomEngine) * 2.0f - 1.0f);
        
        p1.x = static_cast<int32_t>(midX + perpOffset * 0.5f);
        p1.y = startY + static_cast<int32_t>(dy * 0.3f);
        
        p2.x = static_cast<int32_t>(midX - perpOffset * 0.3f);
        p2.y = startY + static_cast<int32_t>(dy * 0.7f);
    }
}

// ============================================================================
// Path Generation
// ============================================================================

std::vector<TouchPoint> HumanTouchEmulator::generateSwipePath(
    int32_t startX, int32_t startY,
    int32_t endX, int32_t endY,
    const SwipeConfig& config
) {
    std::vector<TouchPoint> path;
    
    // Create the four control points for cubic Bezier
    TouchPoint p0(startX, startY);
    TouchPoint p1, p2;
    TouchPoint p3(endX, endY);
    
    // Generate natural control points
    generateBezierControlPoints(startX, startY, endX, endY, config.curveIntensity, p1, p2);
    
    // Calculate number of points based on distance and speed
    float distance = std::sqrt(
        static_cast<float>((endX - startX) * (endX - startX) + 
                          (endY - startY) * (endY - startY))
    );
    
    int numPoints = std::max(20, static_cast<int>(distance / (5.0f * config.swipeSpeedMultiplier)));
    
    // Generate points along the Bezier curve
    for (int i = 0; i <= numPoints; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numPoints);
        TouchPoint point = cubicBezierPoint(t, p0, p1, p2, p3);
        path.push_back(clampToScreen(point));
    }
    
    return path;
}

std::vector<TouchPoint> HumanTouchEmulator::generateTap(
    int32_t x, int32_t y,
    const TapConfig& config
) {
    std::vector<TouchPoint> tap;
    
    TouchPoint point(x, y);
    point.pressure = config.tapPressure;
    point.size = config.tapSize;
    
    // Decide if this is a long press
    bool isLongPress = m_uniform(m_randomEngine) < config.holdProbability;
    int duration = isLongPress ? 
        static_cast<int>(config.longPressMinMs + 
                        m_uniform(m_randomEngine) * (config.longPressMaxMs - config.longPressMinMs)) :
        config.durationMs;
    
    // Generate tap points
    int64_t baseTime = getCurrentTimestampNs();
    
    // Touch down
    point.timestamp = baseTime;
    tap.push_back(point);
    
    // Hold (for longer presses)
    if (duration > 50) {
        int holdPoints = duration / 10;
        for (int i = 1; i < holdPoints; ++i) {
            TouchPoint holdPoint = point;
            holdPoint.timestamp = baseTime + (i * 10000000LL);  // 10ms intervals
            // Slight pressure variation during hold
            holdPoint.pressure = static_cast<int32_t>(
                config.tapPressure * (0.9f + 0.2f * m_uniform(m_randomEngine))
            );
            tap.push_back(holdPoint);
        }
    }
    
    // Touch up
    TouchPoint releasePoint = point;
    releasePoint.timestamp = baseTime + (duration * 1000000LL);
    tap.push_back(releasePoint);
    
    return tap;
}

// ============================================================================
// Micro-Jitter Implementation
// ============================================================================

/**
 * Add micro-jitters to simulate natural hand trembling
 * 
 * Human hands are never perfectly steady. We add small random variations
 * in position that follow realistic patterns:
 * - Higher frequency when moving
 * - Random direction
 * - Variable amplitude
 */
std::vector<TouchPoint> HumanTouchEmulator::addMicroJitter(
    const std::vector<TouchPoint>& points,
    float maxJitter,
    float probability
) {
    if (points.empty()) return points;
    
    std::vector<TouchPoint> jittered = points;
    
    // Jitter distribution - smaller jitters are more common
    std::normal_distribution<float> jitterDist(0.0f, maxJitter / 3.0f);
    
    for (size_t i = 0; i < jittered.size(); ++i) {
        // Only apply jitter with specified probability
        if (m_uniform(m_randomEngine) < probability) {
            // Get jitter amount from normal distribution
            float jitterX = jitterDist(m_randomEngine);
            float jitterY = jitterDist(m_randomEngine);
            
            // Clamp to max
            jitterX = std::clamp(jitterX, -maxJitter, maxJitter);
            jitterY = std::clamp(jitterY, -maxJitter, maxJitter);
            
            jittered[i].x += static_cast<int32_t>(std::round(jitterX));
            jittered[i].y += static_cast<int32_t>(std::round(jitterY));
            
            // Apply to screen bounds
            jittered[i] = clampToScreen(jittered[i]);
        }
    }
    
    return jittered;
}

// ============================================================================
// Gaussian Delay Implementation
// ============================================================================

/**
 * Apply Gaussian-distributed delays to simulate human reaction time
 * 
 * Human response times follow a normal distribution, not uniform.
 * Mean ~100-300ms for simple reactions, with standard deviation ~20-50ms.
 * For touch movements, we use shorter intervals (5-25ms) with natural variation.
 */
std::vector<TouchPoint> HumanTouchEmulator::applyGaussianDelays(
    const std::vector<TouchPoint>& points,
    float meanMs,
    float stdDevMs
) {
    if (points.empty()) return points;
    
    std::vector<TouchPoint> timed = points;
    std::normal_distribution<float> delayDist(meanMs, stdDevMs);
    
    int64_t currentTime = getCurrentTimestampNs();
    
    for (size_t i = 0; i < timed.size(); ++i) {
        // Generate delay from Gaussian distribution
        float delay = delayDist(m_randomEngine);
        
        // Clamp to reasonable range
        delay = std::clamp(delay, 1.0f, 100.0f);
        
        // Accumulate time
        currentTime += static_cast<int64_t>(delay * 1000000LL);
        timed[i].timestamp = currentTime;
    }
    
    return timed;
}

// ============================================================================
// Pressure and Size Simulation
// ============================================================================

/**
 * Simulate varying pressure and touch size during gestures
 * 
 * Real human touches have natural variations:
 * - Pressure tends to be higher at touch-down
 * - Size varies with finger angle and pressure
 * - Both decrease slightly during long swipes (finger settling)
 */
std::vector<TouchPoint> HumanTouchEmulator::simulatePressureAndSize(
    const std::vector<TouchPoint>& points,
    int32_t minPressure, int32_t maxPressure,
    int32_t minSize, int32_t maxSize
) {
    if (points.empty()) return points;
    
    std::vector<TouchPoint> result = points;
    size_t numPoints = result.size();
    
    for (size_t i = 0; i < numPoints; ++i) {
        // Calculate normalized position (0 = start, 1 = end)
        float normalizedPos = static_cast<float>(i) / static_cast<float>(numPoints - 1);
        
        // Calculate base value with slight randomization
        float baseValue = 0.5f + (m_uniform(m_randomEngine) - 0.5f) * 0.1f;
        
        // Pressure profile: higher at start, settles during movement
        float pressureFactor;
        if (normalizedPos < 0.1f) {
            // Initial contact - higher pressure
            pressureFactor = 0.8f + 0.2f * (1.0f - normalizedPos * 10.0f);
        } else if (normalizedPos > 0.9f) {
            // End of gesture - slight increase
            pressureFactor = 0.7f + 0.3f * ((normalizedPos - 0.9f) * 10.0f);
        } else {
            // Middle - natural variation
            pressureFactor = 0.5f + 0.3f * m_uniform(m_randomEngine);
        }
        
        // Calculate final pressure
        int32_t pressureRange = maxPressure - minPressure;
        result[i].pressure = static_cast<int32_t>(
            minPressure + pressureRange * pressureFactor
        );
        
        // Size follows similar pattern but less pronounced
        float sizeFactor = 0.6f + 0.4f * m_uniform(m_randomEngine);
        int32_t sizeRange = maxSize - minSize;
        result[i].size = static_cast<int32_t>(
            minSize + sizeRange * sizeFactor
        );
    }
    
    return result;
}

// ============================================================================
// Natural Behavior Helpers
// ============================================================================

std::vector<TouchPoint> HumanTouchEmulator::addStartAcceleration(
    const std::vector<TouchPoint>& points,
    float accelerationFactor
) {
    if (points.size() < 3) return points;
    
    std::vector<TouchPoint> accelerated = points;
    
    // Accelerate first few points (finger moving from rest)
    int numAccelPoints = static_cast<int>(points.size() * 0.15f);
    numAccelPoints = std::max(2, std::min(numAccelPoints, 5));
    
    for (int i = 0; i < numAccelPoints; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(numAccelPoints);
        // Ease-out curve for acceleration
        float ease = 1.0f - std::pow(1.0f - t, 2.0f);
        
        // Offset points slightly backward initially then catch up
        if (i > 0 && i < numAccelPoints - 1) {
            TouchPoint& prev = accelerated[i - 1];
            TouchPoint& curr = accelerated[i];
            
            // Create slight initial delay then acceleration
            float delay = 1.0f - (ease * accelerationFactor);
            curr.x = prev.x + static_cast<int32_t>((curr.x - prev.x) * delay);
            curr.y = prev.y + static_cast<int32_t>((curr.y - prev.y) * delay);
        }
    }
    
    return accelerated;
}

std::vector<TouchPoint> HumanTouchEmulator::addEndPause(
    const std::vector<TouchPoint>& points,
    float pauseDurationMs
) {
    if (points.empty()) return points;
    
    std::vector<TouchPoint> paused = points;
    
    // Add final point with slight position drift (finger lifting)
    if (paused.size() >= 2) {
        TouchPoint lastPoint = paused.back();
        TouchPoint secondLast = paused[paused.size() - 2];
        
        // Slight drift at the end
        lastPoint.x = secondLast.x + static_cast<int32_t>((lastPoint.x - secondLast.x) * 0.9f);
        lastPoint.y = secondLast.y + static_cast<int32_t>((lastPoint.y - secondLast.y) * 0.9f);
        
        // Add pause delay
        lastPoint.timestamp += static_cast<int64_t>(pauseDurationMs * 1000000LL);
        
        paused.back() = lastPoint;
    }
    
    return paused;
}

TouchPoint HumanTouchEmulator::lerp(const TouchPoint& a, const TouchPoint& b, float t) {
    TouchPoint result;
    result.x = static_cast<int32_t>(a.x + (b.x - a.x) * t);
    result.y = static_cast<int32_t>(a.y + (b.y - a.y) * t);
    result.pressure = static_cast<int32_t>(a.pressure + (b.pressure - a.pressure) * t);
    result.size = static_cast<int32_t>(a.size + (b.size - a.size) * t);
    result.slot = a.slot;
    return result;
}

TouchPoint HumanTouchEmulator::clampToScreen(const TouchPoint& point) {
    TouchPoint clamped = point;
    clamped.x = std::clamp(clamped.x, 0, m_screenWidth - 1);
    clamped.y = std::clamp(clamped.y, 0, m_screenHeight - 1);
    return clamped;
}

// ============================================================================
// Complete Realistic Swipe Generation
// ============================================================================

std::vector<TouchPoint> HumanTouchEmulator::generateRealisticSwipe(
    int32_t startX, int32_t startY,
    int32_t endX, int32_t endY,
    const SwipeConfig& config
) {
    // Step 1: Generate base Bezier curve path
    auto path = generateSwipePath(startX, startY, endX, endY, config);
    
    // Step 2: Add start acceleration (finger moving from rest)
    if (config.accelerateAtStart) {
        path = addStartAcceleration(path, 0.7f);
    }
    
    // Step 3: Add micro-jitters (natural hand trembling)
    path = addMicroJitter(path, config.jitterMaxPixels, config.jitterProbability);
    
    // Step 4: Apply Gaussian delays (human reaction time)
    path = applyGaussianDelays(path, config.delayMeanMs, config.delayStdDevMs);
    
    // Step 5: Simulate pressure and size variation
    path = simulatePressureAndSize(
        path, 
        config.minPressure, config.maxPressure,
        config.minSize, config.maxSize
    );
    
    // Step 6: Add end pause (finger lifting off)
    if (config.addPauseAtEnd) {
        path = addEndPause(path, 50.0f);
    }
    
    return path;
}

// ============================================================================
// Multi-Touch Generation
// ============================================================================

std::vector<std::vector<TouchPoint>> HumanTouchEmulator::generateMultiTouchPath(
    const std::vector<TouchPoint>& points1,
    const std::vector<TouchPoint>& points2,
    const SwipeConfig& config
) {
    std::vector<std::vector<TouchPoint>> result(2);
    
    // Ensure both paths have the same number of points
    size_t maxPoints = std::max(points1.size(), points2.size());
    
    // Interpolate points1 if needed
    for (size_t i = 0; i < maxPoints; ++i) {
        if (i < points1.size()) {
            TouchPoint p = points1[i];
            p.slot = 0;
            result[0].push_back(p);
        }
    }
    
    // Interpolate points2 if needed
    for (size_t i = 0; i < maxPoints; ++i) {
        if (i < points2.size()) {
            TouchPoint p = points2[i];
            p.slot = 1;
            result[1].push_back(p);
        }
    }
    
    return result;
}

// ============================================================================
// Linux Input Event Generation
// ============================================================================

std::vector<InputEvent> HumanTouchEmulator::toInputEvents(
    const std::vector<TouchPoint>& points,
    int32_t slot
) {
    std::vector<InputEvent> events;
    
    if (points.empty()) return events;
    
    // Track previous positions for relative calculations
    int32_t prevX = points[0].x;
    int32_t prevY = points[0].y;
    
    for (size_t i = 0; i < points.size(); ++i) {
        const TouchPoint& pt = points[i];
        
        // Determine touch action
        TouchAction action;
        if (i == 0) {
            action = TouchAction::DOWN;
        } else if (i == points.size() - 1) {
            action = TouchAction::UP;
        } else {
            action = TouchAction::MOVE;
        }
        
        struct timeval tv;
        tv.tv_sec = pt.timestamp / 1000000000LL;
        tv.tv_usec = (pt.timestamp % 1000000000LL) / 1000LL;
        
        // Type: ABS_MT_SLOT - Set the slot
        if (action == TouchAction::DOWN) {
            events.push_back({tv, InputConst::EV_ABS, InputConst::ABS_MT_SLOT, slot});
        }
        
        // Type: ABS_MT_POSITION_X
        events.push_back({tv, InputConst::EV_ABS, InputConst::ABS_MT_POSITION_X, pt.x});
        
        // Type: ABS_MT_POSITION_Y
        events.push_back({tv, InputConst::EV_ABS, InputConst::ABS_MT_POSITION_Y, pt.y});
        
        // Type: ABS_MT_PRESSURE
        events.push_back({tv, InputConst::EV_ABS, InputConst::ABS_MT_PRESSURE, pt.pressure});
        
        // Type: ABS_MT_TOUCH_MAJOR (size)
        events.push_back({tv, InputConst::EV_ABS, InputConst::ABS_MT_TOUCH_MAJOR, pt.size});
        
        // Type: BTN_TOUCH (for compatibility)
        if (action == TouchAction::DOWN) {
            events.push_back({tv, InputConst::EV_KEY, InputConst::BTN_TOUCH, 1});
        } else if (action == TouchAction::UP) {
            events.push_back({tv, InputConst::EV_KEY, InputConst::BTN_TOUCH, 0});
        }
        
        // Type: SYN_MT_REPORT - End of touch event
        events.push_back({tv, InputConst::EV_SYN, InputConst::SYN_MT_REPORT, 0});
        
        // Type: SYN_REPORT - Synchronize
        events.push_back({tv, InputConst::EV_SYN, InputConst::SYN_REPORT, 0});
        
        prevX = pt.x;
        prevY = pt.y;
    }
    
    return events;
}

// ============================================================================
// ADB Command Generation
// ============================================================================

std::string HumanTouchEmulator::toADBCommand(
    int32_t startX, int32_t startY,
    int32_t endX, int32_t endY,
    int32_t durationMs
) {
    return ADBCommand::makeSwipeCommand(startX, startY, endX, endY, durationMs);
}

std::string HumanTouchEmulator::toADBTapCommand(int32_t x, int32_t y, int32_t durationMs) {
    return ADBCommand::makeTapCommand(x, y, durationMs);
}

// ============================================================================
// ADB Execution
// ============================================================================

bool HumanTouchEmulator::executeSwipeViaADB(
    int32_t startX, int32_t startY,
    int32_t endX, int32_t endY,
    int32_t durationMs
) {
    if (!m_adbExecutor) {
        Logger::getInstance().warning("ADB executor not set");
        return false;
    }
    
    std::string command = toADBCommand(startX, startY, endX, endY, durationMs);
    return m_adbExecutor(command);
}

bool HumanTouchEmulator::executeTapViaADB(int32_t x, int32_t y, int32_t durationMs) {
    if (!m_adbExecutor) {
        Logger::getInstance().warning("ADB executor not set");
        return false;
    }
    
    std::string command = toADBTapCommand(x, y, durationMs);
    return m_adbExecutor(command);
}

void HumanTouchEmulator::setADBExecutor(std::function<bool(const std::string&)> executor) {
    m_adbExecutor = executor;
}

// ============================================================================
// Touch Profile Management
// ============================================================================

void HumanTouchEmulator::randomizeTouchProfile() {
    // Generate random values within realistic ranges
    std::uniform_real_distribution<float> pressureDist(0.4f, 0.7f);
    std::uniform_real_distribution<float> sizeDist(0.3f, 0.5f);
    std::uniform_real_distribution<float> speedDist(0.8f, 1.2f);
    std::uniform_real_distribution<float> jitterDist(0.3f, 0.5f);
    
    m_touchProfile = {
        .basePressure = pressureDist(m_randomEngine),
        .pressureVariance = 0.1f + 0.1f * m_uniform(m_randomEngine),
        .baseSize = sizeDist(m_randomEngine),
        .sizeVariance = 0.15f + 0.1f * m_uniform(m_randomEngine),
        .baseSpeed = speedDist(m_randomEngine),
        .speedVariance = 0.05f + 0.1f * m_uniform(m_randomEngine),
        .jitterTendency = jitterDist(m_randomEngine)
    };
    
    Logger::getInstance().debug("Touch profile randomized");
}

HumanTouchEmulator::TouchProfile HumanTouchEmulator::getTouchProfile() const {
    return m_touchProfile;
}

// ============================================================================
// Utility Functions
// ============================================================================

int64_t HumanTouchEmulator::getCurrentTimestampNs() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<int64_t>(ts.tv_sec) * 1000000000LL + ts.tv_nsec;
}

// ============================================================================
// ADBCommand Static Methods
// ============================================================================

std::string ADBCommand::makeTapCommand(int x, int y, int durationMs) {
    if (durationMs > 0) {
        // Long press via swipe with same coordinates
        return "input swipe " + std::to_string(x) + " " + std::to_string(y) + 
               " " + std::to_string(x) + " " + std::to_string(y) + 
               " " + std::to_string(durationMs);
    }
    return "input tap " + std::to_string(x) + " " + std::to_string(y);
}

std::string ADBCommand::makeSwipeCommand(int x1, int y1, int x2, int y2, int durationMs) {
    return "input swipe " + std::to_string(x1) + " " + std::to_string(y1) + 
           " " + std::to_string(x2) + " " + std::to_string(y2) + 
           " " + std::to_string(durationMs);
}

std::string ADBCommand::makeMultiTouchCommand(const std::vector<TouchPoint>& points) {
    // ADB doesn't directly support multi-touch, so we use sendevent for raw input
    // This is a placeholder - actual implementation would use /dev/input/eventX
    std::stringstream ss;
    ss << "# Multi-touch requires direct input event injection" << std::endl;
    ss << "# Use toInputEvents() for Linux event format" << std::endl;
    return ss.str();
}

} // namespace AntiDetect