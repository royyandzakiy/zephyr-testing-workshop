# Unlike tests/unit, links the app's real blinky.c unchanged. Only app.overlay
# differs: the test swaps the pins, not the code.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky_emul)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/blinky.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/blink_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
