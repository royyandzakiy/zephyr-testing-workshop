# Links dispenser.c but not auger_port_sim.c, the only real auger_run().
# FAKE_VALUE_FUNC in src/main.c defines that symbol for this binary instead.

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
