# apps/09-gtest-gmock/tests/CMakeLists.txt

include(GoogleTest)

add_executable(climate_tests
    test_logic.cpp
    test_service.cpp
)

# gmock_main, not gtest_main. It pulls in gtest as well, and it installs the
# gmock listener that turns an unmet EXPECT_CALL into a test failure. Link
# gtest_main by mistake and your mocks stop verifying themselves, silently.
target_link_libraries(climate_tests PRIVATE climate GTest::gmock_main)
target_compile_options(climate_tests PRIVATE -Wall -Wextra)

if(CLIMATE_SANITIZERS)
    # Worth doing on the host, where it is nearly free. The same flags on a
    # Cortex-M are a much bigger conversation.
    target_compile_options(climate_tests PRIVATE -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(climate_tests PRIVATE -fsanitize=address,undefined)
endif()

# gtest_discover_tests runs the binary at build time and registers every TEST
# as its own ctest entry, so `ctest -R Hysteresis` works and a crash in one
# case does not take the rest down with it. add_test() alone would register
# the whole binary as a single test.
gtest_discover_tests(climate_tests)
