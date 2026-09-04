// tests/emul/src/main.c
//
// The same five presses as tests/unit, except this time they go through the
// real GPIO driver, the real interrupt callback and the real blinky.c. What
// is fake is only the controller underneath, and app.overlay decided that.

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

#include "blinky.h"

/* The test binds the very same aliases the application binds. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

/* app.overlay wires the button GPIO_ACTIVE_LOW, so "pressed" is a raw 0.
 * Read it off the devicetree flags instead of hardcoding, and the same test
 * survives someone flipping the polarity in the overlay. */
static inline int pressed_raw_level(void)
{
    return (button.dt_flags & GPIO_ACTIVE_LOW) ? 0 : 1;
}

static void press_button(void)
{
    const int pressed = pressed_raw_level();

    gpio_emul_input_set(button.port, button.pin, pressed);
    k_sleep(K_MSEC(20));

    gpio_emul_input_set(button.port, button.pin, !pressed);
    k_sleep(K_MSEC(20));
}

static int led_level(void)
{
    /* Raw pin level. app.overlay wires the LED GPIO_ACTIVE_HIGH, so raw and
     * logical agree and 1 means lit. */
    return gpio_emul_output_get(led.port, led.pin);
}

/* Runs once, before the suite -- deliberately not a per-test `before` hook.
 * gpio_emul fires whatever callbacks are registered straight out of its
 * pin_configure(), so re-running blinky_init() between tests shows up as a
 * phantom button press. One init, and the test below owns the whole story
 * rather than leaning on the order ztest happens to run things in. */
static void *emul_setup(void)
{
    zassert_ok(blinky_init(), "blinky_init() failed against the emulated pins");
    return NULL;
}

ZTEST_SUITE(blink_emul, NULL, emul_setup, NULL, NULL, NULL);

ZTEST(blink_emul, test_presses_toggle_the_led)
{
    /* Same five presses and the same expected states as tests/unit -- but
     * read back off a pin instead of returned from a function. */
    static const int expected[] = {1, 0, 1, 0, 1};

    zassert_equal(led_level(), 0, "LED should be off before the first press");

    for (int press = 0; press < ARRAY_SIZE(expected); press++) {
        press_button();

        TC_PRINT("press %d: LED level %d\n", press + 1, led_level());

        zassert_equal(led_level(), expected[press],
                      "press %d: expected LED level %d, got %d",
                      press + 1, expected[press], led_level());
    }
}
