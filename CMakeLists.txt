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

# Qt6 configuration (optional - for GUI)
option(BUILD_GUI "Build Qt6 GUI application" ON)
option(BUILD_CLI "Build CLI application" ON)

if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Core Gui Widgets Network Widgets CONFIG QUIET)
    if(Qt6_FOUND)
        message(STATUS "Qt6 found - GUI support enabled")
        set(QT_VERSION 6)
    else()
        find_package(Qt5 COMPONENTS Core Gui Widgets Network CONFIG QUIET)
        if(Qt5_FOUND)
            message(STATUS "Qt5 found - GUI support enabled")
            set(QT_VERSION 5)
        else()
            message(WARNING "Qt not found - GUI support disabled")
            set(BUILD_GUI OFF)
        endif()
    endif()
endif()

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
    # Core Engine
    include/CoreEngine.hpp
    include/VMManager.hpp
    include/profiles/DeviceIdentityGenerator.hpp
    include/profiles/DeviceProfileDatabase.hpp
    include/ProfileManager.hpp
    include/ResourceAllocator.hpp
    include/StorageManager.hpp
    include/SnapshotManager.hpp
    
    # Enterprise Cryptography
    include/core/CryptoUtils.hpp
    
    # Hardware Emulation
    include/HardwareEmulator.hpp
    include/CPUEmulator.hpp
    include/GPUEmulator.hpp
    include/SMBIOSEmulator.hpp
    include/SensorEmulator.hpp
    include/BootloaderEmulator.hpp
    include/hardware/AdvancedSensorSimulator.hpp
    include/hardware/AdvancedBatterySimulator.hpp
    include/hardware/touch/HyperRealisticTouchEmulator.hpp
    
    # Browser Fingerprinting (NEW)
    include/browser/BrowserFingerprinting.hpp
    
    # Network Virtualization
    include/NetworkVirtualizer.hpp
    include/MACAddressGenerator.hpp
    include/DNSResolver.hpp
    include/TCPFingerprintEmulator.hpp
    include/VPNController.hpp
    include/network/ProxyConfig.hpp
    
    # Anti-Detection Suite (from AntiDetectPro)
    include/SafetyNetBypass.hpp
    include/PlayIntegrityBypass.hpp
    include/VirtualizationHider.hpp
    include/TimingAttackPrevention.hpp
    include/security/attestation/HardwareAttestationEmulator.hpp
    include/DeviceFingerprintGenerator.hpp
    
    # Advanced Anti-Detection v2.4 (Detection-Proof Enhancement)
    include/anti_detect/TrustZoneEmulator.hpp
    include/anti_detect/ProcEmulator.hpp
    include/anti_detect/SafetyNetEmulator.hpp
    include/anti_detect/SELinuxEmulator.hpp
    include/anti_detect/AntiDetectionManager.hpp
    include/anti_detect/AdvancedAntiDetection.hpp
    include/anti_detect/CompleteAntiDetection.hpp
    
    # Profile Generation (from AntiDetectPro)
    include/RealisticProfileGenerator.hpp
    include/DeviceProfileDatabase.hpp
    include/CarrierDatabase.hpp
    include/RegionDatabase.hpp
    
    # Security
    include/EncryptionManager.hpp
    include/IsolationController.hpp
    include/SELinuxManager.hpp
    
    # Utilities
    include/Logger.hpp
    include/Config.hpp
    include/CommonTypes.hpp
)

# ============================================
# CORE SOURCES
# ============================================
set(CORE_SOURCES
    # Core Engine
    src/CoreEngine.cpp
    src/VMManager.cpp
    src/ProfileManager.cpp
    src/profiles/DeviceIdentityGenerator.cpp
    src/profiles/DeviceProfileDatabase.cpp
    src/ResourceAllocator.cpp
    src/StorageManager.cpp
    src/SnapshotManager.cpp
    
    # Enterprise Cryptography
    src/core/CryptoUtils.cpp
    
    # Hardware Emulation
    src/HardwareEmulator.cpp
    src/CPUEmulator.cpp
    src/GPUEmulator.cpp
    src/SMBIOSEmulator.cpp
    src/SensorEmulator.cpp
    src/BootloaderEmulator.cpp
    src/hardware/AdvancedSensorSimulator.cpp
    src/hardware/AdvancedBatterySimulator.cpp
    src/hardware/touch/HyperRealisticTouchEmulator.cpp
    
    # Browser Fingerprinting (NEW)
    src/browser/BrowserFingerprinting.cpp
    
    # Network Virtualization
    src/NetworkVirtualizer.cpp
    src/MACAddressGenerator.cpp
    src/DNSResolver.cpp
    src/TCPFingerprintEmulator.cpp
    src/VPNController.cpp
    src/network/ProxyConfig.cpp
    
    # Anti-Detection Suite
    src/SafetyNetBypass.cpp
    src/PlayIntegrityBypass.cpp
    src/VirtualizationHider.cpp
    src/TimingAttackPrevention.cpp
    src/security/attestation/HardwareAttestationEmulator.cpp
    src/DeviceFingerprintGenerator.cpp
    
    # Advanced Anti-Detection v2.4 (Detection-Proof Enhancement)
    src/anti_detect/TrustZoneEmulator.cpp
    src/anti_detect/ProcEmulator.cpp
    src/anti_detect/SafetyNetEmulator.cpp
    src/anti_detect/SELinuxEmulator.cpp
    src/anti_detect/AntiDetectionManager.cpp
    src/anti_detect/AdvancedAntiDetection.cpp
    src/anti_detect/CompleteAntiDetection.cpp
    
    # Profile Generation
    src/RealisticProfileGenerator.cpp
    src/DeviceProfileDatabase.cpp
    src/CarrierDatabase.cpp
    src/RegionDatabase.cpp
    
    # Security
    src/EncryptionManager.cpp
    src/IsolationController.cpp
    src/SELinuxManager.cpp
    
    # Utilities
    src/Logger.cpp
    src/Config.cpp
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
# GUI EXECUTABLE (Qt)
# ============================================
if(BUILD_GUI AND (Qt6_FOUND OR Qt5_FOUND))
    set(GUI_HEADERS
        gui/MainWindow.hpp
        gui/VMListWidget.hpp
        gui/ProfileEditor.hpp
        gui/NetworkSettings.hpp
        gui/AntiDetectionPanel.hpp
        gui/SnapshotManager.hpp
    )
    
    set(GUI_SOURCES
        gui/MainWindow.cpp
        gui/VMListWidget.cpp
        gui/ProfileEditor.cpp
        gui/NetworkSettings.cpp
        gui/AntiDetectionPanel.cpp
        gui/SnapshotManager.cpp
        gui/main.cpp
    )
    
    # Qt MOC processing
    if(Qt6_FOUND)
        set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
        qt6_wrap_cpp(GUI_MOC ${GUI_HEADERS})
    else()
        set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
        qt5_wrap_cpp(GUI_MOC ${GUI_HEADERS})
    endif()
    
    add_executable(VirtualPhonePro_GUI
        ${GUI_SOURCES}
        ${GUI_HEADERS}
        ${GUI_MOC}
        ${CORE_SOURCES}
        ${CORE_HEADERS}
    )
    
    target_link_libraries(VirtualPhonePro_GUI
        ${QT_LIBRARIES}
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
    )
    
    target_compile_definitions(VirtualPhonePro_GUI PRIVATE VERSION="${PROJECT_VERSION}")
    
    install(TARGETS VirtualPhonePro_GUI DESTINATION bin)
    
    message(STATUS "GUI executable: VirtualPhonePro_GUI")
endif()

# ============================================
# SHARED LIBRARY
# ============================================
add_library(VirtualPhonePro_Core SHARED
    ${CORE_SOURCES}
    ${CORE_HEADERS}
)

target_link_libraries(VirtualPhonePro_Core
    Threads::Threads
    OpenSSL::SSL
    OpenSSL::Crypto
)

install(TARGETS VirtualPhonePro_Core DESTINATION lib)

# ============================================
# VIRTUAL MACHINE TEMPLATES
# ============================================
install(DIRECTORY vm_configs/
    DESTINATION share/VirtualPhonePro/vm_configs
    FILES_MATCHING PATTERN "*.json"
)

install(DIRECTORY templates/
    DESTINATION share/VirtualPhonePro/templates
    FILES_MATCHING PATTERN "*.xml"
)

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
