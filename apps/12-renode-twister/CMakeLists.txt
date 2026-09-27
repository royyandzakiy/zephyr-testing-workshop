cmake_minimum_required(VERSION 3.20)

# This folder is also a board root, for the nrf52840dk/nrf52840/renode variant under
# boards/nordic/nrf52840dk/. Zephyr reads BOARD_ROOT inside find_package(Zephyr), so
# appending it after that line would leave the variant unknown and the build would stop
# with "Invalid BOARD".
list(APPEND BOARD_ROOT ${CMAKE_CURRENT_SOURCE_DIR})

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(renode_twister)

target_sources(app PRIVATE
    src/main.c
)
