# tests/unit/CMakeLists.txt
#
# Links the seam and nothing else. No sensor driver, no I2C, no devicetree,
# no emulator -- climate_logic.c has no Zephyr dependencies at all, so there
# is nothing here to configure.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(climate_logic_unit)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sensors/climate_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sensors
)
