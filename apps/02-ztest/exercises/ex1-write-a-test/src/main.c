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
    /* TODO: write this test.
     *
     * The LED starts off, and every press toggles it, including the first.
     * Press the button 5 times starting from off. After each press the LED
     * should read, in order: ON, OFF, ON, OFF, ON.
     *
     * Use blink_logic_toggle() to get the next state after a press, and
     * blink_logic_str() to turn a state into "ON" or "OFF". Check each press
     * with zassert_str_equal(), and put the press number in the assertion
     * message so a failure tells you which press went wrong.
     *
     * Delete the ztest_test_skip() line below once the test is written.
     *
     * Stuck? See ../solution/main.c
     */
    ztest_test_skip();
}
