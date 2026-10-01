# The app's own main.c, unmodified, plus the test-only shell backdoor,
# the same shape as apps/04-shell-pytest.

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
