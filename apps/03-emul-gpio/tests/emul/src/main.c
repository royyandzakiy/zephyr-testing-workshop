// Same five presses as tests/unit, but through the real GPIO driver, interrupt
// callback and blinky.c. Only the controller underneath is emulated.

#include <zephyr/ztest.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/gpio/gpio_emul.h>

#include "blinky.h"

/* Same aliases the application binds. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

/* The button is GPIO_ACTIVE_LOW, so "pressed" is a raw 0. Reading the devicetree
 * flags instead of hardcoding keeps the test correct if the overlay flips polarity. */
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
    /* Raw pin level. The LED is GPIO_ACTIVE_HIGH, so 1 means lit. */
    return gpio_emul_output_get(led.port, led.pin);
}

/* Runs once before the suite, not as a per-test `before` hook. gpio_emul fires
 * registered callbacks from pin_configure(), so re-running blinky_init()
 * between tests would look like a phantom button press. */
static void *emul_setup(void)
{
    zassert_ok(blinky_init(), "blinky_init() failed against the emulated pins");
    return NULL;
}

ZTEST_SUITE(blink_emul, NULL, emul_setup, NULL, NULL, NULL);

ZTEST(blink_emul, test_presses_toggle_the_led)
{
    /* Same expected states as tests/unit, read back from a pin instead of a return value. */
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
