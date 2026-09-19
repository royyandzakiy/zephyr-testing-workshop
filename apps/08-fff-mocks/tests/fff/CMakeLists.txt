# tests/fff/CMakeLists.txt
#
# Read the target_sources list, then read what is missing from it.
#
# Linked:     climate_service.c, climate_logic.c
# NOT linked: sensor_port_bme280.c, sensor_port_sim.c, alarm_port_led.c
#
# Those three files are the only definitions of sensor_port_ready(),
# sensor_port_read() and alarm_port_set() in the tree. Leave them out and the
# symbols are undefined -- until src/main.c in this directory defines them with
# FAKE_VALUE_FUNC and FAKE_VOID_FUNC, and the linker is perfectly happy.
#
# That is the entire mechanism. No --wrap, no weak symbols, no #ifdef TEST in
# production code.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(climate_fff)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/climate_service.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/climate_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
