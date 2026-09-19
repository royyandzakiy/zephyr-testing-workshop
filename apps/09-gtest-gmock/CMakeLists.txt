# apps/09-gtest-gmock/CMakeLists.txt
#
# Read the first two lines and notice what is NOT there: no
# find_package(Zephyr), no west, no board. This is an ordinary host CMake
# project, and `west twister -T apps/` walks straight past it because there is
# no testcase.yaml anywhere underneath.
#
# That is the deal this app is offering. You give up the kernel, the
# devicetree and any claim about real hardware. You get a two-second
# edit-build-test loop, gdb without a probe, gmock, sanitizers that work
# properly, and coverage that does not need instrumenting a target.
#
# Build and run:
#   cmake -S apps/09-gtest-gmock -B apps/09-gtest-gmock/build
#   cmake --build apps/09-gtest-gmock/build -j
#   ctest --test-dir apps/09-gtest-gmock/build --output-on-failure

cmake_minimum_required(VERSION 3.20)
project(climate_gtest CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE)
endif()

option(CLIMATE_SANITIZERS "Build the tests with ASan and UBSan" OFF)

# --------------------------------------------------------------------------
# The library under test. No main(), so both the demo binary and the test
# binaries can link it.
# --------------------------------------------------------------------------

add_library(climate STATIC
    src/logic.cpp
    src/service.cpp
)
target_include_directories(climate PUBLIC include)
target_compile_options(climate PRIVATE -Wall -Wextra -Wpedantic)

# --------------------------------------------------------------------------
# The demo binary. Not required by the tests; it exists so this folder is a
# project you can run.
# --------------------------------------------------------------------------

add_executable(climate_demo src/main.cpp)
target_include_directories(climate_demo PRIVATE src)
target_link_libraries(climate_demo PRIVATE climate)

# --------------------------------------------------------------------------
# GoogleTest.
#
# FetchContent downloads it at configure time, which needs network exactly
# once. If you are offline, or your CI runner is, clone googletest somewhere
# and point at it:
#
#   cmake -S apps/09-gtest-gmock -B build -DGTEST_LOCAL_DIR=/path/to/googletest
# --------------------------------------------------------------------------

set(GTEST_LOCAL_DIR "" CACHE PATH "Path to a googletest checkout, instead of downloading one")

if(GTEST_LOCAL_DIR)
    message(STATUS "Using googletest from ${GTEST_LOCAL_DIR}")
    add_subdirectory(${GTEST_LOCAL_DIR} ${CMAKE_BINARY_DIR}/googletest EXCLUDE_FROM_ALL)
else()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        # Pinned, not a branch. A test framework that changes under you turns
        # an unrelated red build into an afternoon.
        GIT_TAG v1.15.2
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

enable_testing()
add_subdirectory(tests)
