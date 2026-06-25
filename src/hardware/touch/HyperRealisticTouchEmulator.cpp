/**
 * HyperRealisticTouchEmulator - Enterprise-Grade Touch Simulation Implementation
 */

#include "hardware/touch/HyperRealisticTouchEmulator.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

namespace VirtualPhonePro {

// ============================================
// HyperRealisticTouchEmulator Implementation
// ============================================

HyperRealisticTouchEmulator::HyperRealisticTouchEmulator()
    : m_rd(), m_gen(m_rd()),
      m_gaussianDist(0.0f, 1.0f),
      m_uniform_dist(0.0f, 1.0f),
      m_profile(TouchProfile::NATURAL),
      m_screenWidth(1080),
      m_screenHeight(2400),
      m_screenDPI(480),
      m_manufacturer("Samsung"),
      m_model("SM-G998B"),
      m_learningSampleCount(0) {
    
    // Default touch characteristics
    m_characteristics.avgPressure = 0.5f;
    m_characteristics.pressureVariance = 0.15f;
    m_characteristics.avgSwipeSpeed = 0.8f;
    m_characteristics.swipeSpeedVariance = 0.2f;
    m_characteristics.avgTouchSize = 8.0f;
    m_characteristics.touchSizeVariance = 2.0f;
    m_characteristics.tapInterval = 150.0f;
    m_characteristics.gestureAccuracy = 0.95f;
    m_characteristics.palmRejectionRate = 0.85f;
    m_characteristics.multiTouchCoordination = 0.9f;
}

HyperRealisticTouchEmulator::~HyperRealisticTouchEmulator() {}

HyperRealisticTouchEmulator& HyperRealisticTouchEmulator::getInstance() {
    static HyperRealisticTouchEmulator instance;
    return instance;
}

void HyperRealisticTouchEmulator::setProfile(TouchProfile profile) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_profile = profile;
    
    // Adjust characteristics based on profile
    switch (profile) {
        case TouchProfile::NATURAL:
            m_characteristics.avgPressure = 0.5f;
            m_characteristics.pressureVariance = 0.15f;
            m_characteristics.avgSwipeSpeed = 0.8f;
            m_characteristics.swipeSpeedVariance = 0.2f;
            m_characteristics.gestureAccuracy = 0.95f;
            break;
            
        case TouchProfile::CAREFUL:
            m_characteristics.avgPressure = 0.3f;
            m_characteristics.pressureVariance = 0.1f;
            m_characteristics.avgSwipeSpeed = 0.5f;
            m_characteristics.swipeSpeedVariance = 0.1f;
            m_characteristics.gestureAccuracy = 0.98f;
            break;
            
        case TouchProfile::AGGRESSIVE:
            m_characteristics.avgPressure = 0.7f;
            m_characteristics.pressureVariance = 0.2f;
            m_characteristics.avgSwipeSpeed = 1.2f;
            m_characteristics.swipeSpeedVariance = 0.3f;
            m_characteristics.gestureAccuracy = 0.85f;
            break;
            
        case TouchProfile::ELDERLY:
            m_characteristics.avgPressure = 0.4f;
            m_characteristics.pressureVariance = 0.25f;  // More variance due to tremors
            m_characteristics.avgSwipeSpeed = 0.4f;
            m_characteristics.swipeSpeedVariance = 0.15f;
            m_characteristics.gestureAccuracy = 0.8f;
            break;
            
        case TouchProfile::POWER_USER:
            m_characteristics.avgPressure = 0.6f;
            m_characteristics.pressureVariance = 0.1f;
            m_characteristics.avgSwipeSpeed = 1.0f;
            m_characteristics.swipeSpeedVariance = 0.15f;
            m_characteristics.gestureAccuracy = 0.97f;
            break;
            
        case TouchProfile::GESTURE_MASTER:
            m_characteristics.avgPressure = 0.5f;
            m_characteristics.pressureVariance = 0.08f;
            m_characteristics.avgSwipeSpeed = 0.9f;
            m_characteristics.swipeSpeedVariance = 0.05f;
            m_characteristics.gestureAccuracy = 0.99f;
            break;
    }
}

void HyperRealisticTouchEmulator::configureTouchCharacteristics(const TouchCharacteristics& chars) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_characteristics = chars;
}

void HyperRealisticTouchEmulator::setScreenSize(int width, int height, int dpi) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_screenWidth = width;
    m_screenHeight = height;
    m_screenDPI = dpi;
}

void HyperRealisticTouchEmulator::setDeviceModel(const std::string& manufacturer, const std::string& model) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_manufacturer = manufacturer;
    m_model = model;
}

TouchPoint HyperRealisticTouchEmulator::generateTouchPoint(int id, float x, float y, TouchAction action) {
    std::lock_guard<std::mutex> lock(m_mutex);
    
    TouchPoint point;
    point.id = id;
    point.x = x;
    point.y = y;
    point.pressure = generatePressure(x, y, action);
    point.size = generateTouchSize(point.pressure);
    
    auto tilt = generateTilt(x, y);
    point.tiltX = tilt.first;
    point.tiltY = tilt.second;
    
    point.orientation = 0.0f;
    point.action = action;
    point.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    return addJitter(point, 0.3f);
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateTap(float x, float y, int tapCount) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TouchPoint> points;
    
    for (int tap = 0; tap < tapCount; tap++) {
        uint64_t baseTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now().time_since_epoch()
        ).count();
        
        // Touch down
        TouchPoint down = generateTouchPoint(0, x, y, TouchAction::DOWN);
        down.timestamp = baseTime;
        points.push_back(down);
        
        // Small movement during tap (natural)
        float jitterX = gaussianRandom(0, 2.0f);
        float jitterY = gaussianRandom(0, 2.0f);
        
        // Touch move
        TouchPoint move = generateTouchPoint(0, x + jitterX, y + jitterY, TouchAction::MOVE);
        move.timestamp = baseTime + 10;
        move.pressure = m_characteristics.avgPressure * 0.9f;
        points.push_back(move);
        
        // Touch up
        TouchPoint up = generateTouchPoint(0, x + jitterX * 0.5f, y + jitterY * 0.5f, TouchAction::UP);
        up.timestamp = baseTime + 50 + gaussianRandom(0, 10);
        up.pressure = m_characteristics.avgPressure * 0.8f;
        points.push_back(up);
        
        // Interval between taps
        if (tap < tapCount - 1) {
            baseTime += 150 + gaussianRandom(0, 30);  // Natural tap interval
        }
    }
    
    return points;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateSwipe(GestureType direction, 
                                                                  float startX, float startY,
                                                                  float distance, bool withCurve) {
    float endX = startX, endY = startY;
    
    switch (direction) {
        case GestureType::SWIPE_RIGHT:
            endX = startX + distance;
            break;
        case GestureType::SWIPE_LEFT:
            endX = startX - distance;
            break;
        case GestureType::SWIPE_UP:
            endY = startY - distance;
            break;
        case GestureType::SWIPE_DOWN:
            endY = startY + distance;
            break;
        default:
            break;
    }
    
    return generateNaturalFingerPath(startX, startY, endX, endY, withCurve);
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateNaturalFingerPath(float startX, float startY,
                                                                              float endX, float endY,
                                                                              bool useCurve) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TouchPoint> points;
    
    // Generate base number of points based on distance
    float dist = calculateDistance(startX, startY, endX, endY);
    int numPoints = static_cast<int>(dist / 5) + 10;  // ~5px between points
    numPoints = std::min(numPoints, 200);  // Cap at 200 points
    numPoints = std::max(numPoints, 20);   // Minimum 20 points
    
    // Calculate duration based on swipe speed
    float speed = m_characteristics.avgSwipeSpeed + gaussianRandom(0, m_characteristics.swipeSpeedVariance);
    speed = std::max(0.3f, std::min(speed, 2.0f));
    uint64_t duration = static_cast<uint64_t>(dist / speed);
    
    uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    float lastX = startX, lastY = startY;
    
    for (int i = 0; i <= numPoints; i++) {
        float t = static_cast<float>(i) / numPoints;
        
        // Calculate position with natural variance
        float x, y;
        
        if (useCurve) {
            // Add natural curve to the path
            float curveFactor = gaussianRandom(0, 0.1f);
            
            // Bezier-like curve
            float midX = (startX + endX) / 2 + curveFactor * dist;
            float midY = (startY + endY) / 2 + curveFactor * dist;
            
            // Catmull-Rom interpolation
            auto curvePoint = catmullRom(t,
                {startX - (endX - startX) * 0.1f, startY - (endY - startY) * 0.1f},
                {startX, startY},
                {endX, endY},
                {endX + (endX - startX) * 0.1f, endY + (endY - startY) * 0.1f}
            );
            x = curvePoint.first;
            y = curvePoint.second;
        } else {
            // Linear path with natural variance
            x = lerp(startX, endX, t);
            y = lerp(startY, endY, t);
        }
        
        // Add biomechanical jitter
        float jitterAmount = m_characteristics.gestureAccuracy < 0.9f ? 3.0f : 1.0f;
        x += gaussianRandom(0, jitterAmount);
        y += gaussianRandom(0, jitterAmount);
        
        // Ensure within bounds
        x = std::max(0.0f, std::min(x, static_cast<float>(m_screenWidth)));
        y = std::max(0.0f, std::min(y, static_cast<float>(m_screenHeight)));
        
        TouchPoint point;
        point.id = 0;
        point.x = x;
        point.y = y;
        point.pressure = generatePressure(x, y, TouchAction::MOVE);
        point.size = generateTouchSize(point.pressure);
        
        auto tilt = generateTilt(x, y);
        point.tiltX = tilt.first;
        point.tiltY = tilt.second;
        
        // Calculate orientation based on movement direction
        float dx = x - lastX;
        float dy = y - lastY;
        if (std::abs(dx) > 0.1f || std::abs(dy) > 0.1f) {
            point.orientation = std::atan2(dy, dx) * 180.0f / M_PI;
        }
        
        // Timing with natural variation
        point.timestamp = startTime + (uint64_t)(t * duration) + gaussianRandom(0, 3);
        
        // Action based on position
        if (i == 0) {
            point.action = TouchAction::DOWN;
            point.pressure = m_characteristics.avgPressure;
        } else if (i == numPoints) {
            point.action = TouchAction::UP;
            point.pressure = m_characteristics.avgPressure * 0.7f;
        } else {
            point.action = TouchAction::MOVE;
            // Natural pressure variation during swipe
            point.pressure *= (1.0f + gaussianRandom(0, 0.1f));
        }
        
        points.push_back(point);
        lastX = x;
        lastY = y;
    }
    
    return points;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generatePinch(float centerX, float centerY, float scale) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TouchPoint> points;
    
    // Calculate pinch points
    float pinchDistance = 100.0f * scale;
    float angle = 0.0f;
    
    // Two fingers starting apart
    float finger1StartX = centerX - pinchDistance;
    float finger1StartY = centerY;
    float finger2StartX = centerX + pinchDistance;
    float finger2StartY = centerY;
    
    // Two fingers ending together
    float finger1EndX = centerX - 20.0f;
    float finger1EndY = centerY;
    float finger2EndX = centerX + 20.0f;
    float finger2EndY = centerY;
    
    int numPoints = 30;
    uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    for (int i = 0; i <= numPoints; i++) {
        float t = static_cast<float>(i) / numPoints;
        
        // Interpolate finger positions
        float x1 = lerp(finger1StartX, finger1EndX, t);
        float y1 = lerp(finger1StartY, finger1EndY, t);
        float x2 = lerp(finger2StartX, finger2EndX, t);
        float y2 = lerp(finger2StartY, finger2EndY, t);
        
        // Add natural variance
        x1 += gaussianRandom(0, 1.0f);
        y1 += gaussianRandom(0, 1.0f);
        x2 += gaussianRandom(0, 1.0f);
        y2 += gaussianRandom(0, 1.0f);
        
        uint64_t timestamp = startTime + (uint64_t)(t * 500);
        
        TouchPoint p1, p2;
        p1.id = 0;
        p1.x = x1;
        p1.y = y1;
        p1.pressure = 0.5f + gaussianRandom(0, 0.1f);
        p1.size = 8.0f + gaussianRandom(0, 1.0f);
        p1.timestamp = timestamp;
        p1.action = (i == 0) ? TouchAction::DOWN : TouchAction::MOVE;
        
        p2.id = 1;
        p2.x = x2;
        p2.y = y2;
        p2.pressure = 0.5f + gaussianRandom(0, 0.1f);
        p2.size = 8.0f + gaussianRandom(0, 1.0f);
        p2.timestamp = timestamp;
        p2.action = (i == 0) ? TouchAction::DOWN : TouchAction::MOVE;
        
        points.push_back(p1);
        points.push_back(p2);
    }
    
    // Add final UP actions
    TouchPoint p1, p2;
    p1.id = 0;
    p1.action = TouchAction::UP;
    p1.timestamp = startTime + 500 + gaussianRandom(0, 5);
    points.push_back(p1);
    
    p2.id = 1;
    p2.action = TouchAction::UP;
    p2.timestamp = startTime + 500 + gaussianRandom(0, 5);
    points.push_back(p2);
    
    return points;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateSpread(float centerX, float centerY, float scale) {
    // Spread is essentially reverse pinch
    return generatePinch(centerX, centerY, -scale);
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateLongPress(float x, float y, int durationMs) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TouchPoint> points;
    
    uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    // Touch down
    TouchPoint down = generateTouchPoint(0, x, y, TouchAction::DOWN);
    down.timestamp = startTime;
    down.pressure = m_characteristics.avgPressure * 0.8f;
    points.push_back(down);
    
    // Small drift during long press
    int numUpdates = durationMs / 16;  // ~60fps
    for (int i = 1; i < numUpdates - 1; i++) {
        float driftX = gaussianRandom(0, 0.5f);
        float driftY = gaussianRandom(0, 0.5f);
        
        TouchPoint move = generateTouchPoint(0, x + driftX, y + driftY, TouchAction::MOVE);
        move.timestamp = startTime + (i * 16);
        move.pressure = m_characteristics.avgPressure * (0.8f + gaussianRandom(0, 0.05f));
        points.push_back(move);
    }
    
    // Touch up
    TouchPoint up = generateTouchPoint(0, x + gaussianRandom(0, 2.0f), 
                                       y + gaussianRandom(0, 2.0f), TouchAction::UP);
    up.timestamp = startTime + durationMs + gaussianRandom(0, 10);
    up.pressure = m_characteristics.avgPressure * 0.6f;
    points.push_back(up);
    
    return points;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateDrag(float startX, float startY, 
                                                                  float endX, float endY) {
    return generateNaturalFingerPath(startX, startY, endX, endY, true);
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateScroll(float startX, float startY,
                                                                   GestureType direction, float distance) {
    // Scroll is similar to swipe but typically with more points
    auto points = generateSwipe(direction, startX, startY, distance, true);
    
    // Add momentum effect at the end
    if (!points.empty()) {
        auto& lastPoint = points.back();
        // Natural deceleration
        float lastPressure = lastPoint.pressure * 0.5f;
        lastPoint.pressure = lastPressure;
    }
    
    return points;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateRotation(float centerX, float centerY, float angle) {
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TouchPoint> points;
    
    float radius = 100.0f;
    int numPoints = static_cast<int>(std::abs(angle) / 10.0f) + 10;
    
    uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    float startAngleRad = 0.0f;
    float angleRad = angle * M_PI / 180.0f;
    
    for (int i = 0; i <= numPoints; i++) {
        float t = static_cast<float>(i) / numPoints;
        float currentAngle = startAngleRad + angleRad * t;
        
        // Two fingers at opposite positions
        float x1 = centerX + radius * std::cos(currentAngle);
        float y1 = centerY + radius * std::sin(currentAngle);
        float x2 = centerX + radius * std::cos(currentAngle + M_PI);
        float y2 = centerY + radius * std::sin(currentAngle + M_PI);
        
        uint64_t timestamp = startTime + (uint64_t)(t * 1000);
        
        TouchPoint p1, p2;
        p1.id = 0;
        p1.x = x1 + gaussianRandom(0, 1.0f);
        p1.y = y1 + gaussianRandom(0, 1.0f);
        p1.pressure = 0.5f;
        p1.size = 8.0f;
        p1.timestamp = timestamp;
        p1.action = (i == 0) ? TouchAction::DOWN : TouchAction::MOVE;
        p1.orientation = currentAngle * 180.0f / M_PI;
        
        p2.id = 1;
        p2.x = x2 + gaussianRandom(0, 1.0f);
        p2.y = y2 + gaussianRandom(0, 1.0f);
        p2.pressure = 0.5f;
        p2.size = 8.0f;
        p2.timestamp = timestamp;
        p2.action = (i == 0) ? TouchAction::DOWN : TouchAction::MOVE;
        p2.orientation = currentAngle * 180.0f / M_PI + 180.0f;
        
        points.push_back(p1);
        points.push_back(p2);
    }
    
    // Final UP actions
    TouchPoint p1, p2;
    p1.id = 0;
    p1.action = TouchAction::UP;
    p1.timestamp = startTime + 1000;
    points.push_back(p1);
    
    p2.id = 1;
    p2.action = TouchAction::UP;
    p2.timestamp = startTime + 1000;
    points.push_back(p2);
    
    return points;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateFromPath(const std::vector<std::pair<float, float>>& path) {
    if (path.size() < 2) return {};
    
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<TouchPoint> points;
    
    uint64_t startTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    
    for (size_t i = 0; i < path.size(); i++) {
        float x = path[i].first;
        float y = path[i].second;
        
        // Add natural variance
        x += gaussianRandom(0, 1.0f);
        y += gaussianRandom(0, 1.0f);
        
        TouchPoint point;
        point.id = 0;
        point.x = x;
        point.y = y;
        point.pressure = generatePressure(x, y, TouchAction::MOVE);
        point.size = generateTouchSize(point.pressure);
        point.timestamp = startTime + (i * 16);
        
        if (i == 0) {
            point.action = TouchAction::DOWN;
        } else if (i == path.size() - 1) {
            point.action = TouchAction::UP;
        } else {
            point.action = TouchAction::MOVE;
        }
        
        points.push_back(point);
    }
    
    return points;
}

// Device-specific patterns
std::vector<TouchPoint> HyperRealisticTouchEmulator::generateSamsungPattern(float startX, float startY,
                                                                           float endX, float endY) {
    // Samsung: smoother, larger touches, curved paths
    TouchCharacteristics saved = m_characteristics;
    m_characteristics.avgPressure = 0.55f;
    m_characteristics.avgTouchSize = 9.0f;
    m_characteristics.swipeSpeedVariance = 0.15f;
    
    auto result = generateNaturalFingerPath(startX, startY, endX, endY, true);
    
    m_characteristics = saved;
    return result;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateApplePattern(float startX, float startY,
                                                                         float endX, float endY) {
    // Apple: precise, consistent pressure, minimal variance
    TouchCharacteristics saved = m_characteristics;
    m_characteristics.avgPressure = 0.5f;
    m_characteristics.pressureVariance = 0.05f;
    m_characteristics.gestureAccuracy = 0.98f;
    m_characteristics.swipeSpeedVariance = 0.05f;
    
    auto result = generateNaturalFingerPath(startX, startY, endX, endY, false);
    
    m_characteristics = saved;
    return result;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generatePixelPattern(float startX, float startY,
                                                                          float endX, float endY) {
    // Pixel: natural, balanced
    auto result = generateNaturalFingerPath(startX, startY, endX, endY, true);
    return result;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateXiaomiPattern(float startX, float startY,
                                                                           float endX, float endY) {
    // Xiaomi: aggressive, fast, less curved
    TouchCharacteristics saved = m_characteristics;
    m_characteristics.avgPressure = 0.65f;
    m_characteristics.avgSwipeSpeed = 1.1f;
    m_characteristics.swipeSpeedVariance = 0.25f;
    
    auto result = generateNaturalFingerPath(startX, startY, endX, endY, false);
    
    m_characteristics = saved;
    return result;
}

std::vector<TouchPoint> HyperRealisticTouchEmulator::generateOnePlusPattern(float startX, float startY,
                                                                            float endX, float endY) {
    // OnePlus: smooth, gesture-optimized
    TouchCharacteristics saved = m_characteristics;
    m_characteristics.avgPressure = 0.5f;
    m_characteristics.avgSwipeSpeed = 0.9f;
    m_characteristics.gestureAccuracy = 0.96f;
    
    auto result = generateNaturalFingerPath(startX, startY, endX, endY, true);
    
    m_characteristics = saved;
    return result;
}

// ============================================
// Pressure, Size, Tilt Generation
// ============================================

float HyperRealisticTouchEmulator::generatePressure(float x, float y, TouchAction action) {
    // Base pressure varies by location on screen
    float basePressure = m_characteristics.avgPressure;
    
    // Natural variance
    float pressure = basePressure + gaussianRandom(0, m_characteristics.pressureVariance);
    
    // Action-specific adjustments
    if (action == TouchAction::DOWN) {
        pressure *= 1.1f;
    } else if (action == TouchAction::UP) {
        pressure *= 0.8f;
    }
    
    // Screen edge touches often have slightly different pressure
    float edgeDistance = std::min({x, y, 
        static_cast<float>(m_screenWidth) - x, 
        static_cast<float>(m_screenHeight) - y});
    if (edgeDistance < 50.0f) {
        pressure *= 0.95f;  // Slightly lower pressure at edges
    }
    
    return std::max(0.1f, std::min(pressure, 1.0f));
}

float HyperRealisticTouchEmulator::generateTouchSize(float pressure) {
    float baseSize = m_characteristics.avgTouchSize;
    float size = baseSize + gaussianRandom(0, m_characteristics.touchSizeVariance);
    
    // Size correlates with pressure
    size *= (0.8f + pressure * 0.4f);
    
    return std::max(4.0f, std::min(size, 20.0f));
}

std::pair<float, float> HyperRealisticTouchEmulator::generateTilt(float x, float y) {
    // Tilt based on screen position and natural variance
    float tiltX = gaussianRandom(0, 5.0f);
    float tiltY = gaussianRandom(0, 5.0f);
    
    // Natural tendency based on hand position
    // Bottom of screen (thumb area) tends to have more tilt
    float yNorm = y / m_screenHeight;
    tiltY += yNorm * 10.0f;
    
    return {tiltX, tiltY};
}

float HyperRealisticTouchEmulator::generateOrientation(float x, float y, float prevX, float prevY) {
    float dx = x - prevX;
    float dy = y - prevY;
    
    if (std::abs(dx) < 0.1f && std::abs(dy) < 0.1f) {
        return 0.0f;
    }
    
    float angle = std::atan2(dy, dx) * 180.0f / M_PI;
    
    // Add natural variance
    angle += gaussianRandom(0, 2.0f);
    
    return angle;
}

// ============================================
// Gesture Recognition Evasion
// ============================================

TouchPoint HyperRealisticTouchEmulator::addJitter(const TouchPoint& point, float jitterAmount) {
    TouchPoint jittered = point;
    
    // Position jitter
    jittered.x += gaussianRandom(0, jitterAmount);
    jittered.y += gaussianRandom(0, jitterAmount);
    
    // Pressure jitter
    jittered.pressure *= (1.0f + gaussianRandom(0, 0.05f));
    jittered.pressure = std::max(0.1f, std::min(jittered.pressure, 1.0f));
    
    // Size jitter
    jittered.size *= (1.0f + gaussianRandom(0, 0.05f));
    
    // Timestamp jitter
    jittered.timestamp += gaussianRandom(0, 2);
    
    return jittered;
}

float HyperRealisticTouchEmulator::addVelocityVariation(float baseVelocity) {
    float variation = gaussianRandom(0, baseVelocity * 0.1f);
    return std::max(0.1f, baseVelocity + variation);
}

uint64_t HyperRealisticTouchEmulator::addTimingIrregularity(uint64_t baseTime) {
    return baseTime + gaussianRandom(0, 3);
}

float HyperRealisticTouchEmulator::addPressureDrift(float basePressure, int pointIndex) {
    // Slow drift over time
    float drift = gaussianRandom(0, 0.01f) * (pointIndex % 50);
    return std::max(0.1f, std::min(basePressure + drift, 1.0f));
}

// ============================================
// Utility Methods
// ============================================

GestureResult HyperRealisticTouchEmulator::calculateGestureMetrics(const std::vector<TouchPoint>& points) {
    GestureResult result;
    result.points = points;
    result.type = detectGestureType(points);
    
    if (points.empty()) return result;
    
    result.startX = points.front().x;
    result.startY = points.front().y;
    result.endX = points.back().x;
    result.endY = points.back().y;
    
    result.startTime = points.front().timestamp;
    result.endTime = points.back().timestamp;
    result.duration = result.endTime - result.startTime;
    
    // Calculate total distance
    float totalDistance = 0;
    for (size_t i = 1; i < points.size(); i++) {
        totalDistance += calculateDistance(
            points[i-1].x, points[i-1].y,
            points[i].x, points[i].y
        );
    }
    result.distance = totalDistance;
    
    // Calculate velocity and acceleration
    if (result.duration > 0) {
        result.velocity = result.distance / result.duration;
    }
    
    return result;
}

bool HyperRealisticTouchEmulator::validateTouchSequence(const std::vector<TouchPoint>& points) {
    if (points.empty()) return false;
    
    // Check for valid action sequence (DOWN before MOVE/UP)
    bool hasDown = false;
    for (const auto& p : points) {
        if (p.action == TouchAction::DOWN) {
            hasDown = true;
        } else if (!hasDown && p.action != TouchAction::DOWN) {
            return false;  // MOVE/UP without DOWN
        }
    }
    
    // Check timestamp ordering
    for (size_t i = 1; i < points.size(); i++) {
        if (points[i].timestamp < points[i-1].timestamp) {
            return false;
        }
    }
    
    return true;
}

GestureType HyperRealisticTouchEmulator::detectGestureType(const std::vector<TouchPoint>& points) {
    if (points.empty()) return GestureType::TAP;
    
    float totalDistance = 0;
    uint64_t duration = points.back().timestamp - points.front().timestamp;
    
    for (size_t i = 1; i < points.size(); i++) {
        totalDistance += calculateDistance(
            points[i-1].x, points[i-1].y,
            points[i].x, points[i].y
        );
    }
    
    // Simple heuristic for gesture detection
    if (totalDistance < 50) {
        return GestureType::TAP;
    } else if (duration > 500 && totalDistance < 100) {
        return GestureType::LONG_PRESS;
    } else {
        // Determine direction
        float dx = points.back().x - points.front().x;
        float dy = points.back().y - points.front().y;
        
        if (std::abs(dx) > std::abs(dy)) {
            return dx > 0 ? GestureType::SWIPE_RIGHT : GestureType::SWIPE_LEFT;
        } else {
            return dy > 0 ? GestureType::SWIPE_DOWN : GestureType::SWIPE_UP;
        }
    }
}

// ============================================
// Mathematical Helpers
// ============================================

float HyperRealisticTouchEmulator::calculateDistance(float x1, float y1, float x2, float y2) {
    float dx = x2 - x1;
    float dy = y2 - y1;
    return std::sqrt(dx*dx + dy*dy);
}

float HyperRealisticTouchEmulator::calculateVelocity(float distance, uint64_t timeDelta) {
    if (timeDelta == 0) return 0;
    return distance / timeDelta;
}

float HyperRealisticTouchEmulator::lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

float HyperRealisticTouchEmulator::gaussianRandom(float mean, float stddev) {
    return mean + stddev * m_gaussianDist(m_gen);
}

float HyperRealisticTouchEmulator::uniformRandom(float min, float max) {
    return m_uniform_dist(m_gen) * (max - min) + min;
}

std::pair<float, float> HyperRealisticTouchEmulator::bezierPoint(float t,
    float x0, float y0, float x1, float y1,
    float x2, float y2, float x3, float y3) {
    
    float u = 1.0f - t;
    float tt = t * t;
    float uu = u * u;
    float uuu = uu * u;
    float ttt = tt * t;
    
    float x = uuu * x0 + 3 * uu * t * x1 + 3 * u * tt * x2 + ttt * x3;
    float y = uuu * y0 + 3 * uu * t * y1 + 3 * u * tt * y2 + ttt * y3;
    
    return {x, y};
}

std::pair<float, float> HyperRealisticTouchEmulator::catmullRom(float t,
    const std::pair<float, float>& p0,
    const std::pair<float, float>& p1,
    const std::pair<float, float>& p2,
    const std::pair<float, float>& p3) {
    
    float t2 = t * t;
    float t3 = t2 * t;
    
    float x = 0.5f * ((2.0f * p1.first) + 
                       (-p0.first + p2.first) * t +
                       (2.0f * p0.first - 5.0f * p1.first + 4.0f * p2.first - p3.first) * t2 +
                       (-p0.first + 3.0f * p1.first - 3.0f * p2.first + p3.first) * t3);
    
    float y = 0.5f * ((2.0f * p1.second) + 
                       (-p0.second + p2.second) * t +
                       (2.0f * p0.second - 5.0f * p1.second + 4.0f * p2.second - p3.second) * t2 +
                       (-p0.second + 3.0f * p1.second - 3.0f * p2.second + p3.second) * t3);
    
    return {x, y};
}

// ============================================
// Touch Characteristics
// ============================================

TouchCharacteristics HyperRealisticTouchEmulator::getCharacteristics() {
    std::lock_guard<std::mutex> lock(m_mutex);
    return m_characteristics;
}

TouchCharacteristics HyperRealisticTouchEmulator::analyzeTouchSequence(const std::vector<TouchPoint>& points) {
    if (points.empty()) return m_characteristics;
    
    float totalPressure = 0;
    float totalVelocity = 0;
    float totalSize = 0;
    int count = 0;
    
    for (size_t i = 1; i < points.size(); i++) {
        totalPressure += points[i].pressure;
        float dist = calculateDistance(points[i-1].x, points[i-1].y,
                                       points[i].x, points[i].y);
        uint64_t dt = points[i].timestamp - points[i-1].timestamp;
        if (dt > 0) {
            totalVelocity += dist / dt;
        }
        totalSize += points[i].size;
        count++;
    }
    
    if (count > 0) {
        TouchCharacteristics analyzed;
        analyzed.avgPressure = totalPressure / count;
        analyzed.avgSwipeSpeed = totalVelocity / count;
        analyzed.avgTouchSize = totalSize / count;
        return analyzed;
    }
    
    return m_characteristics;
}

void HyperRealisticTouchEmulator::learnFromUser(const std::vector<TouchPoint>& gesture) {
    TouchCharacteristics analyzed = analyzeTouchSequence(gesture);
    m_learnedCharacteristics.push_back(analyzed);
    m_learningSampleCount++;
    
    // Update base characteristics with learned data
    if (m_learnedCharacteristics.size() >= 5) {
        float sumPressure = 0, sumVelocity = 0, sumSize = 0;
        for (const auto& c : m_learnedCharacteristics) {
            sumPressure += c.avgPressure;
            sumVelocity += c.avgSwipeSpeed;
            sumSize += c.avgTouchSize;
        }
        
        size_t n = m_learnedCharacteristics.size();
        m_characteristics.avgPressure = (m_characteristics.avgPressure * 0.7f) + (sumPressure / n * 0.3f);
        m_characteristics.avgSwipeSpeed = (m_characteristics.avgSwipeSpeed * 0.7f) + (sumVelocity / n * 0.3f);
        m_characteristics.avgTouchSize = (m_characteristics.avgTouchSize * 0.7f) + (sumSize / n * 0.3f);
        
        // Keep only recent samples
        if (m_learnedCharacteristics.size() > 20) {
            m_learnedCharacteristics.erase(m_learnedCharacteristics.begin());
        }
    }
}

// ============================================
// Touch Biometrics Analyzer Implementation
// ============================================

TouchBiometricsAnalyzer::TouchBiometricsAnalyzer()
    : m_rd(), m_gen(m_rd()) {}

std::map<std::string, float> TouchBiometricsAnalyzer::extractBiometrics(const std::vector<TouchPoint>& points) {
    std::map<std::string, float> biometrics;
    
    if (points.empty()) return biometrics;
    
    // Velocity profile
    auto velocities = getVelocityProfile(points);
    if (!velocities.empty()) {
        float avgVel = std::accumulate(velocities.begin(), velocities.end(), 0.0f) / velocities.size();
        biometrics["avg_velocity"] = avgVel;
        
        // Velocity variance
        float variance = 0;
        for (float v : velocities) {
            variance += (v - avgVel) * (v - avgVel);
        }
        biometrics["velocity_variance"] = variance / velocities.size();
    }
    
    // Pressure profile
    auto pressures = getPressureProfile(points);
    if (!pressures.empty()) {
        float avgPressure = std::accumulate(pressures.begin(), pressures.end(), 0.0f) / pressures.size();
        biometrics["avg_pressure"] = avgPressure;
    }
    
    // Gesture metrics
    float totalDistance = 0;
    for (size_t i = 1; i < points.size(); i++) {
        float dx = points[i].x - points[i-1].x;
        float dy = points[i].y - points[i-1].y;
        totalDistance += std::sqrt(dx*dx + dy*dy);
    }
    biometrics["total_distance"] = totalDistance;
    
    uint64_t duration = points.back().timestamp - points.front().timestamp;
    biometrics["duration"] = duration;
    
    return biometrics;
}

std::vector<float> TouchBiometricsAnalyzer::getVelocityProfile(const std::vector<TouchPoint>& points) {
    std::vector<float> velocities;
    
    for (size_t i = 1; i < points.size(); i++) {
        float dx = points[i].x - points[i-1].x;
        float dy = points[i].y - points[i-1].y;
        float dist = std::sqrt(dx*dx + dy*dy);
        uint64_t dt = points[i].timestamp - points[i-1].timestamp;
        if (dt > 0) {
            velocities.push_back(dist / dt);
        }
    }
    
    return velocities;
}

std::vector<float> TouchBiometricsAnalyzer::getAccelerationProfile(const std::vector<TouchPoint>& points) {
    auto velocities = getVelocityProfile(points);
    std::vector<float> accelerations;
    
    for (size_t i = 1; i < velocities.size(); i++) {
        float dv = velocities[i] - velocities[i-1];
        uint64_t dt = points[i+1].timestamp - points[i].timestamp;
        if (dt > 0) {
            accelerations.push_back(dv / dt);
        }
    }
    
    return accelerations;
}

std::vector<float> TouchBiometricsAnalyzer::getPressureProfile(const std::vector<TouchPoint>& points) {
    std::vector<float> pressures;
    for (const auto& p : points) {
        pressures.push_back(p.pressure);
    }
    return pressures;
}

std::vector<float> TouchBiometricsAnalyzer::getCurvatureProfile(const std::vector<TouchPoint>& points) {
    std::vector<float> curvatures;
    
    for (size_t i = 1; i < points.size() - 1; i++) {
        // Calculate curvature using three points
        float x1 = points[i-1].x, y1 = points[i-1].y;
        float x2 = points[i].x, y2 = points[i].y;
        float x3 = points[i+1].x, y3 = points[i+1].y;
        
        float dx1 = x2 - x1, dy1 = y2 - y1;
        float dx2 = x3 - x2, dy2 = y3 - y2;
        
        float cross = dx1 * dy2 - dy1 * dx2;
        float dot = dx1 * dx2 + dy1 * dy2;
        float angle = std::atan2(cross, dot);
        
        curvatures.push_back(std::abs(angle));
    }
    
    return curvatures;
}

bool TouchBiometricsAnalyzer::detectAnomaly(const std::vector<TouchPoint>& points) {
    // Simple anomaly detection based on variance
    auto velocities = getVelocityProfile(points);
    
    if (velocities.empty()) return false;
    
    float mean = std::accumulate(velocities.begin(), velocities.end(), 0.0f) / velocities.size();
    float variance = 0;
    for (float v : velocities) {
        variance += (v - mean) * (v - mean);
    }
    variance /= velocities.size();
    
    float stddev = std::sqrt(variance);
    
    // Check for extreme outliers
    for (float v : velocities) {
        if (std::abs(v - mean) > 3 * stddev) {
            return true;  // Anomaly detected
        }
    }
    
    return false;
}

float TouchBiometricsAnalyzer::comparePatterns(const std::vector<TouchPoint>& pattern1,
                                             const std::vector<TouchPoint>& pattern2) {
    if (pattern1.empty() || pattern2.empty()) return 0.0f;
    
    auto biometrics1 = extractBiometrics(pattern1);
    auto biometrics2 = extractBiometrics(pattern2);
    
    float similarity = 0.0f;
    int count = 0;
    
    for (const auto& kv : biometrics1) {
        auto it = biometrics2.find(kv.first);
        if (it != biometrics2.end()) {
            float diff = std::abs(kv.second - it->second);
            float maxVal = std::max(std::abs(kv.second), std::abs(it->second));
            if (maxVal > 0) {
                similarity += 1.0f - std::min(diff / maxVal, 1.0f);
            }
            count++;
        }
    }
    
    return count > 0 ? similarity / count : 0.0f;
}

// ============================================
// Palm Rejection Simulator Implementation
// ============================================

PalmRejectionSimulator::PalmRejectionSimulator()
    : m_rd(), m_gen(m_rd()),
      m_sensitivity(0.7f), m_enabled(true) {}

bool PalmRejectionSimulator::shouldRejectAsPalm(const TouchPoint& point) {
    if (!m_enabled) return false;
    
    // Large touch size indicates palm
    if (point.size > 15.0f * m_sensitivity) {
        return true;
    }
    
    // Touch near screen edge often indicates palm
    if (point.x < 20 || point.x > 1060 ||
        point.y < 20 || point.y > 2380) {
        return true;
    }
    
    return false;
}

TouchPoint PalmRejectionSimulator::generatePalmTouch(float x, float y) {
    TouchPoint point;
    point.id = 0;
    point.x = x;
    point.y = y;
    point.size = 18.0f + uniformRandom(0, 5.0f);  // Large for palm
    point.pressure = 0.7f + uniformRandom(0, 0.2f);
    point.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()
    ).count();
    point.action = TouchAction::DOWN;
    
    return point;
}

void PalmRejectionSimulator::setSensitivity(float sensitivity) {
    m_sensitivity = std::max(0.0f, std::min(sensitivity, 1.0f));
}

void PalmRejectionSimulator::enablePalmRejection(bool enable) {
    m_enabled = enable;
}

float PalmRejectionSimulator::uniformRandom(float min, float max) {
    std::uniform_real_distribution<float> dist(min, max);
    return dist(m_gen);
}

// ============================================
// Touch Latency Simulator Implementation
// ============================================

TouchLatencySimulator::TouchLatencySimulator()
    : m_rd(), m_gen(m_rd()),
      m_baseLatency(20), m_latencyVariance(5), m_deviceType("flagship") {}

uint64_t TouchLatencySimulator::addLatency(uint64_t eventTime) {
    std::normal_distribution<float> dist(static_cast<float>(m_baseLatency), 
                                        static_cast<float>(m_latencyVariance));
    int latency = static_cast<int>(dist(m_gen));
    latency = std::max(5, std::min(latency, 100));  // Clamp to realistic range
    return eventTime + latency;
}

void TouchLatencySimulator::setLatencyProfile(const std::string& deviceType) {
    m_deviceType = deviceType;
    
    if (deviceType == "flagship") {
        m_baseLatency = 20;
        m_latencyVariance = 5;
    } else if (deviceType == "midrange") {
        m_baseLatency = 40;
        m_latencyVariance = 10;
    } else if (deviceType == "budget") {
        m_baseLatency = 60;
        m_latencyVariance = 15;
    }
}

int TouchLatencySimulator::getCurrentLatency() {
    return m_baseLatency;
}

TouchPoint TouchLatencySimulator::addLatencyToPoint(const TouchPoint& point) {
    TouchPoint result = point;
    result.timestamp = addLatency(point.timestamp);
    return result;
}

} // namespace VirtualPhonePro
