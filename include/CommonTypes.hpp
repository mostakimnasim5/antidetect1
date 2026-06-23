#pragma once

#include <string>
#include <vector>
#include <map>

namespace AntiDetect {

// DeviceInfo is defined in ADBManager.hpp
// FingerprintProfile is defined in ProfileManager.hpp
// This header provides other common types

struct SpoofingResult {
    bool success;
    std::string message;
    std::string originalValue;
    std::string newValue;
    std::string error;
};

struct ProfileInfo {
    std::string id;
    std::string name;
    std::string description;
    std::string category;
    std::string createdAt;
    std::string modifiedAt;
};

} // namespace AntiDetect
