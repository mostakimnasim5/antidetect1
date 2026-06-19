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

find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${OPENSSL_INCLUDE_DIR}
)

set(HEADERS
    include/AntiDetectCore.hpp
    include/ADBManager.hpp
    include/DeviceFingerprint.hpp
    include/NetworkSpoofer.hpp
    include/SystemManager.hpp
    include/ProfileManager.hpp
    include/Logger.hpp
    include/Config.hpp
)

set(SOURCES
    src/main.cpp
    src/AntiDetectCore.cpp
    src/ADBManager.cpp
    src/DeviceFingerprint.cpp
    src/NetworkSpoofer.cpp
    src/SystemManager.cpp
    src/ProfileManager.cpp
    src/Logger.cpp
    src/Config.cpp
)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})
target_link_libraries(${PROJECT_NAME} Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
target_compile_definitions(${PROJECT_NAME} PRIVATE VERSION="${PROJECT_VERSION}")

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
