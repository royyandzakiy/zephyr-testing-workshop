cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(feeder_cpp CXX)

target_sources(app PRIVATE
    src/main.cpp
    src/dispenser.cpp
)

target_include_directories(app PRIVATE include src)
