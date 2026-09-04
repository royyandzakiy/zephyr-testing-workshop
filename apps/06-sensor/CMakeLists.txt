cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(sensor)

target_sources(app PRIVATE
    src/main.c
    src/sensors/bme280.c
    src/sensors/climate_logic.c
    src/hardware/button.c
    src/hardware/led.c
)

# The emulated BME280 is only buildable where the emulation framework exists,
# which in practice means native_sim. On a real board CONFIG_EMUL is unset and
# this file is simply not compiled -- the same source tree, talking to a real
# chip over a real bus.
target_sources_ifdef(CONFIG_EMUL app PRIVATE src/sensors/bme280_emul.c)
