cmake_minimum_required(VERSION 3.20)
project(VirtualPhonePro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Enable testing
include(CTest)

# Compiler options
if(MSVC)
    add_compile_options(/W4 /WX- /utf-8)
    add_link_options(/DYNAMICBASE /NXCOMPAT)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O3)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(CURL QUIET)

# CLI only (GUI disabled for simplicity)
option(BUILD_CLI "Build CLI application" ON)

# QEMU/KVM detection (Linux)
if(UNIX AND NOT APPLE)
    find_package(QEMU)
endif()

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
)

# ============================================
# CORE HEADERS - All modules
# ============================================
set(CORE_HEADERS
    # Core
    include/VirtualPhonePro.hpp
    
    # Core Engine
    include/core/CoreEngine.hpp
    include/core/VMManager.hpp
    include/core/ProfileManager.hpp
    include/core/CryptoUtils.hpp
    
    # Hardware Emulation
    include/hardware/HardwareEmulator.hpp
    include/hardware/AdvancedSensorSimulator.hpp
    include/hardware/AdvancedBatterySimulator.hpp
    include/hardware/touch/HyperRealisticTouchEmulator.hpp
    
    # Browser Fingerprinting
    include/browser/BrowserFingerprinting.hpp
    
    # Network Virtualization
    include/network/NetworkVirtualizer.hpp
    include/network/ProxyConfig.hpp
    
    # Profiles
    include/profiles/DeviceIdentityGenerator.hpp
    include/profiles/DeviceProfileDatabase.hpp
    
    # Anti-Detection Suite
    include/anti_detect/TrustZoneEmulator.hpp
    include/anti_detect/ProcEmulator.hpp
    include/anti_detect/SafetyNetEmulator.hpp
    include/anti_detect/SELinuxEmulator.hpp
    include/anti_detect/AntiDetectionManager.hpp
    include/anti_detect/AdvancedAntiDetection.hpp
    include/anti_detect/CompleteAntiDetection.hpp
    include/anti_detect/HypervisorBypass.hpp
    include/anti_detect/TLSFingerprint.hpp
    include/anti_detect/NetworkStackSpoofer.hpp
    include/anti_detect/SensorSpoofer.hpp
    include/anti_detect/VirtualSecurityChip.hpp
    include/anti_detect/PlayIntegrityBypass.hpp
    include/anti_detect/MediaDeviceSpoofer.hpp
    
    # Security
    include/security/attestation/HardwareAttestationEmulator.hpp
    
    # Emulator GUI (Windows/Qt)
    include/emulator/AndroidEmulator.hpp
    include/emulator/MainWindow.hpp
    include/emulator/AppStore.hpp
    
    # Scrcpy-based VM Controller (NEW)
    include/vmcontroller/VirtualBoxController.hpp
    include/vmcontroller/ADBSpoofer.hpp
    include/vmcontroller/WindowEmbedder.hpp
    include/vmcontroller/ScrcpyManager.hpp
    include/vmcontroller/VMDisplayWidget.hpp
    include/vmcontroller/VMController.hpp
    include/vmcontroller/AutoSetupManager.hpp
    include/vmcontroller/SetupWizard.hpp
    include/emulator/ScrcpyMainWindow.hpp
)

# ============================================
# CORE SOURCES
# ============================================
set(CORE_SOURCES
    # Core Engine
    src/core/CoreEngine.cpp
    src/core/VMManager.cpp
    src/core/ProfileManager.cpp
    src/core/CryptoUtils.cpp
    src/core/ResourceAllocator.cpp
    src/core/SnapshotManager.cpp
    src/core/StorageManager.cpp
    
    # Hardware Emulation
    src/hardware/HardwareEmulator.cpp
    src/hardware/AdvancedSensorSimulator.cpp
    src/hardware/AdvancedBatterySimulator.cpp
    src/hardware/touch/HyperRealisticTouchEmulator.cpp
    
    # Browser Fingerprinting
    src/browser/BrowserFingerprinting.cpp
    
    # Network Virtualization
    src/network/NetworkVirtualizer.cpp
    src/network/ProxyConfig.cpp
    
    # Profiles
    src/profiles/DeviceIdentityGenerator.cpp
    src/profiles/DeviceProfileDatabase.cpp
    
    # Anti-Detection Suite
    src/anti_detect/TrustZoneEmulator.cpp
    src/anti_detect/ProcEmulator.cpp
    src/anti_detect/SafetyNetEmulator.cpp
    src/anti_detect/SELinuxEmulator.cpp
    src/anti_detect/AntiDetectionManager.cpp
    src/anti_detect/AdvancedAntiDetection.cpp
    src/anti_detect/CompleteAntiDetection.cpp
    src/anti_detect/HypervisorBypass.cpp
    src/anti_detect/TLSFingerprint.cpp
    src/anti_detect/NetworkStackSpoofer.cpp
    src/anti_detect/SensorSpoofer.cpp
    src/anti_detect/VirtualSecurityChip.cpp
    src/anti_detect/PlayIntegrityBypass.cpp
    src/anti_detect/MediaDeviceSpoofer.cpp
    
    # Security
    src/security/attestation/HardwareAttestationEmulator.cpp
    
    # Scrcpy-based VM Controller Sources (NEW)
    src/vmcontroller/VirtualBoxController.cpp
    src/vmcontroller/ADBSpoofer.cpp
    src/vmcontroller/WindowEmbedder.cpp
    src/vmcontroller/ScrcpyManager.cpp
    src/vmcontroller/VMDisplayWidget.cpp
    src/vmcontroller/VMController.cpp
    src/vmcontroller/AutoSetupManager.cpp
    src/vmcontroller/SetupWizard.cpp
    src/emulator/ScrcpyMainWindow.cpp
)

# ============================================
# CLI EXECUTABLE
# ============================================
if(BUILD_CLI)
    add_executable(VirtualPhonePro_CLI
        src/cli/main_cli.cpp
        ${CORE_SOURCES}
        ${CORE_HEADERS}
    )
    
    target_link_libraries(VirtualPhonePro_CLI
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
    )
    
    target_compile_definitions(VirtualPhonePro_CLI PRIVATE VERSION="${PROJECT_VERSION}")
    
    install(TARGETS VirtualPhonePro_CLI DESTINATION bin)
endif()

# ============================================
# GUI EXECUTABLE (Qt-based - Windows)
# ============================================
option(BUILD_GUI "Build Qt GUI application" ON)

if(BUILD_GUI AND WIN32)
    # Find Qt6
    find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)
    
    add_executable(VirtualPhonePro_GUI WIN32
        src/emulator/MainWindow.cpp
        src/emulator/AppStore.cpp
        src/emulator/ScrcpyMainWindow.cpp
        ${CORE_SOURCES}
        ${CORE_HEADERS}
    )
    
    target_link_libraries(VirtualPhonePro_GUI
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
    )
    
    target_compile_definitions(VirtualPhonePro_GUI PRIVATE VERSION="${PROJECT_VERSION}")
    
    install(TARGETS VirtualPhonePro_GUI DESTINATION bin)
endif()

# ============================================
# SCRCPY-BASED GUI EXECUTABLE (NEW)
# ============================================
option(BUILD_SCRCPY_GUI "Build Scrcpy-based GUI application" ON)

if(BUILD_SCRCPY_GUI AND WIN32)
    # Find Qt6
    find_package(Qt6 COMPONENTS Widgets Core Gui REQUIRED)
    
    add_executable(VirtualPhonePro_Scrcpy WIN32
        src/emulator/ScrcpyMainWindow.cpp
        src/vmcontroller/VirtualBoxController.cpp
        src/vmcontroller/ADBSpoofer.cpp
        src/vmcontroller/WindowEmbedder.cpp
        src/vmcontroller/ScrcpyManager.cpp
        src/vmcontroller/VMDisplayWidget.cpp
        src/vmcontroller/VMController.cpp
        src/vmcontroller/AutoSetupManager.cpp
        src/vmcontroller/SetupWizard.cpp
        ${CORE_SOURCES}
        ${CORE_HEADERS}
    )
    
    target_link_libraries(VirtualPhonePro_Scrcpy
        Qt6::Widgets
        Qt6::Core
        Qt6::Gui
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
        CURL::libcurl
    )
    
    target_compile_definitions(VirtualPhonePro_Scrcpy PRIVATE VERSION="${PROJECT_VERSION}")
    
    install(TARGETS VirtualPhonePro_Scrcpy DESTINATION bin)
endif()

# ============================================
# SHARED LIBRARY (Optional)
# ============================================
# Uncomment if you want a shared library:
# add_library(VirtualPhonePro_Core SHARED
#     ${CORE_SOURCES}
#     ${CORE_HEADERS}
# )
# target_link_libraries(VirtualPhonePro_Core
#     Threads::Threads
#     OpenSSL::SSL
#     OpenSSL::Crypto
# )

# ============================================
# WARNINGS SUPPRESSION
# ============================================
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Wno-missing-field-initializers
        -Wno-unused-parameter
        -Wno-unused-variable
    )
endif()
