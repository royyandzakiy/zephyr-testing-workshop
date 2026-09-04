// tests/unit/src/main.c

#include <zephyr/ztest.h>

#include "blink_logic.h"

ZTEST_SUITE(blink_logic, NULL, NULL, NULL, NULL, NULL);

ZTEST(blink_logic, test_toggle_flips_the_state)
{
    zassert_true(blink_logic_toggle(false), "off should toggle to on");
    zassert_false(blink_logic_toggle(true), "on should toggle to off");
}

ZTEST(blink_logic, test_str_matches_what_we_print)
{
    /* These are the exact words that end up in the log line, so the string
     * the device prints is now something a test can pin down. */
    zassert_str_equal(blink_logic_str(true), "ON");
    zassert_str_equal(blink_logic_str(false), "OFF");
}

ZTEST(blink_logic, test_five_presses_from_off)
{
    /* The LED starts off, and every press toggles -- including the first. */
    static const char *const expected[] = {"ON", "OFF", "ON", "OFF", "ON"};
    bool led_on = false;

    for (int press = 0; press < ARRAY_SIZE(expected); press++) {
        led_on = blink_logic_toggle(led_on);

        TC_PRINT("press %d: LED is now %s\n", press + 1, blink_logic_str(led_on));

        zassert_str_equal(blink_logic_str(led_on), expected[press],
                          "press %d: expected %s", press + 1, expected[press]);
    }
}
