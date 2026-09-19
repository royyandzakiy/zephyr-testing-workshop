cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(fff_mocks)

target_sources(app PRIVATE
    src/main.c
    src/climate_service.c
    src/climate_logic.c
    src/alarm_port_led.c
)

# One header, two implementations, and CMake picks. Add a third for a
# different part and nothing above this line changes.
if(CONFIG_BME280)
    target_sources(app PRIVATE src/sensor_port_bme280.c)
else()
    target_sources(app PRIVATE src/sensor_port_sim.c)
endif()

target_include_directories(app PRIVATE src)
