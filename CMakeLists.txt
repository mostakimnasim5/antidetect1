cmake_minimum_required(VERSION 3.20)
project(AntiDetectPro VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

if(MSVC)
    add_compile_options(/W4 /WX-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# Qt6 configuration (optional - for GUI)
option(BUILD_GUI "Build Qt6 GUI application" OFF)
if(BUILD_GUI)
    find_package(Qt6 COMPONENTS Core Gui Widgets Network QUIET)
    if(Qt6_FOUND)
        message(STATUS "Qt6 found - GUI support enabled")
        set(QT6_GUI ON)
    else()
        find_package(Qt5 COMPONENTS Core Gui Widgets Network QUIET)
        if(Qt5_FOUND)
            message(STATUS "Qt5 found - GUI support enabled")
            set(QT6_GUI OFF)
        else()
            message(WARNING "Qt not found - GUI support disabled")
            set(BUILD_GUI OFF)
        endif()
    endif()
endif()

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
)

# Core library sources
set(CORE_HEADERS
    include/AntiDetectCore.hpp
    include/ADBManager.hpp
    include/DeviceFingerprint.hpp
    include/NetworkSpoofer.hpp
    include/SystemManager.hpp
    include/ProfileManager.hpp
    include/Logger.hpp
    include/Config.hpp
    include/AdvancedSpoofing.hpp
    include/AntiDetectEngine.hpp
    include/HttpClient.hpp
    include/CryptoUtils.hpp
)

set(CORE_SOURCES
    src/AntiDetectCore.cpp
    src/ADBManager.cpp
    src/DeviceFingerprint.cpp
    src/NetworkSpoofer.cpp
    src/SystemManager.cpp
    src/ProfileManager.cpp
    src/Logger.cpp
    src/Config.cpp
    src/AdvancedSpoofing.cpp
    src/AntiDetectEngine.cpp
    src/HttpClient.cpp
    src/CryptoUtils.cpp
)

# CLI executable
add_executable(AntiDetectPro 
    src/main.cpp 
    ${CORE_HEADERS} 
    ${CORE_SOURCES}
)
target_link_libraries(AntiDetectPro 
    Threads::Threads 
    OpenSSL::SSL 
    OpenSSL::Crypto
)
target_compile_definitions(AntiDetectPro PRIVATE VERSION="${PROJECT_VERSION}")

# Qt GUI executable
if(BUILD_GUI AND (QT6_GUI OR Qt5_FOUND))
    if(QT6_GUI)
        set(QT_LIBRARIES Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)
        set(QT_MOC_ARGS "")
    else()
        set(QT_LIBRARIES Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
    endif()
    
    set(GUI_HEADERS
        gui/mainwindow.h
    )
    
    set(GUI_SOURCES
        gui/mainwindow.cpp
        gui/main.cpp
    )
    
    qt6_wrap_cpp(GUI_MOC ${GUI_HEADERS})
    
    add_executable(AntiDetectProGUI 
        ${GUI_SOURCES}
        ${GUI_HEADERS}
        ${GUI_MOC}
        ${CORE_HEADERS}
        ${CORE_SOURCES}
    )
    
    target_link_libraries(AntiDetectProGUI
        ${QT_LIBRARIES}
        Threads::Threads
        OpenSSL::SSL
        OpenSSL::Crypto
    )
    
    target_compile_definitions(AntiDetectProGUI PRIVATE VERSION="${PROJECT_VERSION}")
    
    if(ANDROID)
        set_target_properties(AntiDetectProGUI PROPERTIES QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android")
    endif()
    
    message(STATUS "GUI executable: AntiDetectProGUI")
endif()

# Installation
install(TARGETS AntiDetectPro DESTINATION bin)
if(BUILD_GUI AND (QT6_GUI OR Qt5_FOUND))
    install(TARGETS AntiDetectProGUI DESTINATION bin)
endif()
