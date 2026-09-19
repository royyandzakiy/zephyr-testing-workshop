# tests/fff/CMakeLists.txt
#
# Read the source list, then read what is missing from it.
#
# Linked:     dispenser.c
# NOT linked: auger_port_sim.c
#
# That file holds the only definition of auger_run() in the tree. Leave it out
# and the symbol is undefined, until src/main.c in this directory defines it
# with FAKE_VALUE_FUNC and the linker is perfectly happy.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dispenser_fff)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dispenser.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
