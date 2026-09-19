# tests/shell_pytest/CMakeLists.txt
#
# The application's own main.c, unmodified, plus the backdoor. Exactly the
# shape apps/04-shell-pytest uses -- the difference in this app is all on the
# Python side.

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(pytest_advanced_shell)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/test_harness.c
)

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src
)
