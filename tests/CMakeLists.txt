# ==============================================================================
# AntiDetectPro - Test Suite CMakeLists.txt
# ==============================================================================

cmake_minimum_required(VERSION 3.20)

# Enable testing
enable_testing()

# Try to find GoogleTest (optional)
find_package(GTest QUIET)
if(GTest_FOUND)
    add_executable(AntiDetectProTests test_core.cpp)
    target_link_libraries(AntiDetectProTests GTest::gtest_main Threads::Threads)
    include(GoogleTest)
    gtest_discover_tests(AntiDetectProTests)
else()
    # Simple test executable without GoogleTest
    add_executable(AntiDetectProTests test_core.cpp)
    target_link_libraries(AntiDetectProTests Threads::Threads)
    
    # Add a custom test command
    add_test(NAME AntiDetectProTests COMMAND AntiDetectProTests)
endif()
