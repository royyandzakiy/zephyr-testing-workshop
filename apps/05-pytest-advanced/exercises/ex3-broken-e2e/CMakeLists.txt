# exercises/ex3-broken-e2e/CMakeLists.txt
#
# The same build as tests/shell_pytest, except the firmware comes from this
# folder's src/main.c instead of the app's. That copy is the file to fix. The
# backdoor, test_harness.c, is an unmodified copy of the one in
# tests/shell_pytest.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pytest_advanced_ex3)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_harness.c
)

# app_state.h still comes from the app's own src/.
target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
