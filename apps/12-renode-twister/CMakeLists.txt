cmake_minimum_required(VERSION 3.20)

# Board root for the nrf52840dk/nrf52840/renode variant. Must come before
# find_package(Zephyr), which reads BOARD_ROOT; after it the build fails with "Invalid BOARD".
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(renode_twister)

target_sources(app PRIVATE
    src/main.c
)
