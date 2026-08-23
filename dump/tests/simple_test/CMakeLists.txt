cmake_minimum_required(VERSION 3.20)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(simple_test)

target_sources(app PRIVATE 
    simple_test.c
)