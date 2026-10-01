cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor)

target_sources(app PRIVATE
    src/main.c
    src/sensors/bme280.c
    src/sensors/climate_logic.c
)

# The emulated BME280 needs the emulation framework (CONFIG_EMUL, native_sim only).
# On a real board it is not compiled and the same code talks to a real chip.
target_sources_ifdef(CONFIG_EMUL app PRIVATE src/sensors/bme280_emul.c)
