cmake_minimum_required(VERSION 3.10)
project(OSLevelAntiDetect)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Threads REQUIRED)

# Source files
set(SOURCES
    main.cpp
    core/system_fingerprint.cpp
    core/advanced_fingerprint.cpp
)

# Create executable
add_executable(antidetect ${SOURCES})

# Link libraries
target_link_libraries(antidetect PRIVATE Threads::Threads)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_compile_options(antidetect PRIVATE -Wall -Wextra -O2)
    target_link_options(antidetect PRIVATE -pthread)
endif()

# Installation
install(TARGETS antidetect DESTINATION bin)