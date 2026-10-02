cmake_minimum_required(VERSION 3.20)
project(can_hardware_abstraction_layer
    VERSION 1.0.0
    DESCRIPTION "Modern C++17 Hardware Abstraction Layer for Heterogeneous CAN Controllers"
    HOMEPAGE_URL "https://github.com/Toya62/can-hardware-abstraction-layer"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler warning flags
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Header-only Interface Library
add_library(can_hal INTERFACE)
target_include_directories(can_hal INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/can_hal>
    $<INSTALL_INTERFACE:include>
)

# Demo Application
add_executable(can_hal_demo src/main.cpp)
target_link_libraries(can_hal_demo PRIVATE can_hal)

# Unit Testing
enable_testing()
add_executable(can_hal_tests tests/test_can_hal.cpp)
target_link_libraries(can_hal_tests PRIVATE can_hal)
add_test(NAME can_hal_unit_tests COMMAND can_hal_tests)
