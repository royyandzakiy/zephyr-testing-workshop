# tests/unit/CMakeLists.txt
#
# A test suite is just another Zephyr application. It links the *seam* --
# blink_logic.c -- and nothing else. No blinky.c, so no GPIO driver, no
# devicetree aliases, nothing to emulate.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blink_logic_unit)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/blink_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
