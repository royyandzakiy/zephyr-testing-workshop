// tests/simple_test/simple_test.c
#include <zephyr/ztest.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(simple_test, LOG_LEVEL_INF);

/* Test suite definition */
ZTEST_SUITE(test_simple_tests, NULL, NULL, NULL, NULL, NULL);

/* Test case */
ZTEST(test_simple_tests, test_simple_assert) {
    LOG_INF("=== Starting test_simple_assert ===");
    zassert_equal(1, 1, "Simple assertion passed");
    LOG_INF("=== Test passed! ===");
}

ZTEST(test_simple_tests, test_another_check) {
    LOG_INF("=== Starting test_another_check ===");
    int expected = 5;
    int actual = 5;
    LOG_INF("Expected: %d, Actual: %d", expected, actual);
    zassert_equal(expected, actual, "Values should match");
    LOG_INF("=== Test passed! ===");
}