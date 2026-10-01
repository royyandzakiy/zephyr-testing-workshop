# Zephyr test app with GoogleTest compiled into it.
#
# FetchContent_Populate, not FetchContent_MakeAvailable: the latter runs
# googletest's own CMakeLists, whose compiler and standard flags fight Zephyr's.
# Compiling the two fused sources here keeps every flag under Zephyr's control.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(feeder_gtest CXX)

# --------------------------------------------------------------------------
# GoogleTest source. Without network access, clone it once and pass the path:
#   west build ... -- -DGTEST_SRC_DIR=/path/to/googletest
# --------------------------------------------------------------------------

set(GTEST_SRC_DIR "" CACHE PATH "An existing googletest checkout, instead of downloading one")

if(NOT GTEST_SRC_DIR)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    )
    FetchContent_GetProperties(googletest)
    if(NOT googletest_POPULATED)
        FetchContent_Populate(googletest)
    endif()
    set(GTEST_SRC_DIR ${googletest_SOURCE_DIR})
endif()

message(STATUS "googletest source: ${GTEST_SRC_DIR}")

# --------------------------------------------------------------------------
# Trim GoogleTest to what a Zephyr target provides.
#
# zephyr_compile_definitions(), not a per-file define: these change the layout
# of gtest's types, so gtest and every test including its headers must agree.
#
# GTEST_HAS_FILE_SYSTEM stays on: =0 removes posix::Close, which
# gtest-death-test-internal.h still calls, so gtest's own headers fail to build.
# --------------------------------------------------------------------------

zephyr_compile_definitions(
    GTEST_HAS_PTHREAD=0      # Zephyr threads are not pthreads as far as gtest cares
)

# GTEST_HAS_DEATH_TEST is left alone: gtest-port.h defines it itself, so a
# command-line define warns "redefined", which fails Twister's -Werror build.

zephyr_include_directories(
    ${GTEST_SRC_DIR}/googletest/include
    ${GTEST_SRC_DIR}/googlemock/include
)

# --------------------------------------------------------------------------
# Everything, GoogleTest included, goes into `app`. A separate
# zephyr_library_named(googletest) builds but is never linked, because an
# application-level library is not added to ZEPHYR_LIBS.
#
# The app's src/main.cpp is left out: this directory brings its own main(),
# and MockAuger in test_dispenser.cpp is the only IAuger in this binary.
# --------------------------------------------------------------------------

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_portion.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/test_dispenser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/dispenser.cpp

    ${GTEST_SRC_DIR}/googletest/src/gtest-all.cc
    ${GTEST_SRC_DIR}/googlemock/src/gmock-all.cc
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../include
    ${GTEST_SRC_DIR}/googletest
    ${GTEST_SRC_DIR}/googlemock
)

# Third-party source, so its warnings are suppressed rather than fixed.
set_source_files_properties(
    ${GTEST_SRC_DIR}/googletest/src/gtest-all.cc
    ${GTEST_SRC_DIR}/googlemock/src/gmock-all.cc
    PROPERTIES COMPILE_OPTIONS "-w"
)
