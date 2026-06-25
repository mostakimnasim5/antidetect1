#pragma once

/**
 * SELinuxEmulator - Realistic SELinux Context Emulation
 * 
 * Provides realistic SELinux contexts for Android's enforced mode.
 * Critical for passing CTS Profile Match checks.
 * 
 * Android SELinux contexts follow the format:
 * user:role:type[:level]
 * 
 * Example: u:object_r:system_server:s0
 */

#include "../VirtualPhonePro.hpp"
#include <map>
#include <string>
#include <vector>

namespace VirtualPhonePro {

// SELinux enforcement state
enum class SELinuxEnforceState {
    ENFORCING,
    PERMISSIVE,
    DISABLED
};

class SELinuxEmulator {
public:
    static SELinuxEmulator& getInstance();
    
    // Initialize SELinux emulation
    void initialize();
    
    // Get SELinux status
    SELinuxEnforceState getEnforceState();
    bool isEnforcing();
    
    // Get context for a process
    std::string getProcessContext(const std::string& processName);
    
    // Get context for a file/directory
    std::string getFileContext(const std::string& path);
    
    // Get context for a socket
    std::string getSocketContext(const std::string& socketName);
    
    // Verify if a transition is allowed
    bool isTransitionAllowed(
        const std::string& sourceContext,
        const std::string& targetContext,
        const std::string& objectClass
    );
    
    // Get SELinux policy info
    std::map<std::string, std::string> getPolicyInfo();
    
    // Get all file contexts (for /file_contexts)
    std::string getFileContexts();
    
    // Get all process contexts (for /seapp_contexts)
    std::string getSeAppContexts();
    
private:
    SELinuxEmulator();
    ~SELinuxEmulator();
    
    // Process type mappings
    std::map<std::string, std::string> m_processContexts;
    
    // File type mappings
    std::map<std::string, std::string> m_fileContexts;
    
    // Socket type mappings
    std::map<std::string, std::string> m_socketContexts;
    
    // Policy version
    std::string m_policyVersion;
    std::string m_policyBuild;
    
    SELinuxEnforceState m_state;
};

} // namespace VirtualPhonePro
