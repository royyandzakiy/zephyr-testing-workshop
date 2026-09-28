# exercises/ex1-write-a-test/CMakeLists.txt
#
# The same build as tests/unit: the test source from this folder, plus the
# app's real blink_logic.c. Nothing else is linked, so there is no GPIO driver
# and no devicetree alias to satisfy.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blink_logic_ex1)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/blink_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
