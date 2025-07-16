cmake_minimum_required(VERSION 3.13)

project(battery-mgr)

set(BATTERY_MGR_VERSION "1.0")

message(STATUS "Running CMake for ${CMAKE_PROJECT_NAME} v${BATTERY_MGR_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/Install" CACHE PATH "Default install in main directory" FORCE)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(yaml-cpp CONFIG REQUIRED)
find_package(PahoMqttCpp CONFIG REQUIRED)
find_package(Flatbuffers CONFIG REQUIRED)

include_directories("influxdb")

add_subdirectory(microtonome-idl)

include_directories("src")
add_subdirectory("src")

