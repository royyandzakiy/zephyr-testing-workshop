# exercises/ex2b-missing-kconfig/CMakeLists.txt
#
# The same build as tests/emul: the test source from this folder, plus the
# app's real blinky.c and blink_logic.c, unchanged. app.overlay and prj.conf
# in this folder decide which pins and which drivers that code gets.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky_ex2b)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/blinky.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/blink_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
