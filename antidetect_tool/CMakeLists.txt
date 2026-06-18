cmake_minimum_required(VERSION 3.10)
project(AntidetectTool)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(antidetect main.cpp antidetect.cpp)

# Platform-specific settings
if(UNIX AND NOT APPLE)
    target_link_libraries(antidetect pthread)
endif()

# Enable optimizations
target_compile_options(antidetect PRIVATE -O2 -Wall)