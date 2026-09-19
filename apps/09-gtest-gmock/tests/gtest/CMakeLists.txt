# tests/gtest/CMakeLists.txt
#
# A normal Zephyr test application, with GoogleTest compiled into it.
#
# GoogleTest is not a Zephyr module and west knows nothing about it, so the
# source has to come from somewhere. FetchContent downloads it once. We
# deliberately do NOT call FetchContent_MakeAvailable: that would run
# googletest's own CMakeLists, which sets its own compiler and standard flags
# and fights Zephyr's. Populating the source and compiling the two fused
# translation units ourselves keeps every flag under Zephyr's control.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(feeder_gtest CXX)

# --------------------------------------------------------------------------
# GoogleTest source.
#
# Offline, or on a runner with no network, clone it once and pass the path:
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
# Trim GoogleTest down to what a Zephyr target can provide.
#
# zephyr_compile_definitions() rather than a per-file define, because these
# change the layout of gtest's own types. GoogleTest and the tests that include
# its headers have to agree, or you get a link that succeeds and a binary that
# misbehaves.
#
# Not GTEST_HAS_FILE_SYSTEM=0, however tempting. Turning it off removes
# posix::Close from gtest-port.h while gtest-death-test-internal.h still calls
# it, and the build fails inside GoogleTest's own headers. native_sim is a
# Linux process with a real filesystem underneath it, so leave it on.
# --------------------------------------------------------------------------

zephyr_compile_definitions(
    GTEST_HAS_PTHREAD=0      # Zephyr threads are not pthreads as far as gtest cares
)

# Not GTEST_HAS_DEATH_TEST=0 either. gtest-port.h defines that symbol itself
# from its own OS detection, so defining it on the command line produces a
# "redefined" warning. Twister builds with -Werror, so the warning is a build
# failure and the suite errors out before it runs.

zephyr_include_directories(
    ${GTEST_SRC_DIR}/googletest/include
    ${GTEST_SRC_DIR}/googlemock/include
)

# --------------------------------------------------------------------------
# Everything goes into `app`, GoogleTest included.
#
# The tidy-looking alternative is zephyr_library_named(googletest) with its own
# zephyr_library_sources(). It builds libgoogletest.a and then never links it:
# a library declared in an application CMakeLists does not end up in
# ZEPHYR_LIBS, so the final image has hundreds of undefined references to
# testing::. One target has no such problem.
#
# src/main.cpp of the APP is not here. This directory brings its own main, and
# MockAuger in test_dispenser.cpp is the only IAuger in this binary.
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

# Third-party source, so its warnings are not ours to read.
set_source_files_properties(
    ${GTEST_SRC_DIR}/googletest/src/gtest-all.cc
    ${GTEST_SRC_DIR}/googlemock/src/gmock-all.cc
    PROPERTIES COMPILE_OPTIONS "-w"
)
