# tests/emul/CMakeLists.txt
#
# Unlike tests/unit, this suite links the app's REAL sensor code -- bme280.c,
# unmodified -- plus the emulator. Bosch's driver is in the loop too. The only
# thing that differs from a build for a real board is app.overlay.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(climate_emul)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sensors/bme280.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sensors/bme280_emul.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sensors/climate_logic.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/sensors
)
