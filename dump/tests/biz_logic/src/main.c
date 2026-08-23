#include <zephyr/ztest.h>
#include <zephyr/drivers/gpio/gpio_emul.h>
#include <zephyr/drivers/gpio.h>
#include "biz_logic.h"

/* Helper to get the emulator device */
#define GPIO_EMUL_LABEL DT_NODELABEL(gpio_sim)

ZTEST_SUITE(biz_logic_tests, NULL, NULL, NULL, NULL, NULL);

/**
 * Test Case 1: Default state (No magnet, No button)
 * Expectation: Interval = 500ms
 */
ZTEST(biz_logic_tests, test_default_state) {
    const struct device *g_dev = DEVICE_DT_GET(GPIO_EMUL_LABEL);
    
    biz_logic_init();

    // Set inputs to "Idle"
    zassert_ok(gpio_emul_input_set(g_dev, 12, 0), "Failed to set Hall High");
    zassert_ok(gpio_emul_input_set(g_dev, 11, 1), "Failed to set Button High (Pullup)");

    biz_logic_process();

    zassert_equal(get_current_blink_interval(), 500, "Default interval should be 500ms");
}

/**
 * Test Case 2: Magnet detected
 * Expectation: Interval = 250ms
 */
ZTEST(biz_logic_tests, test_magnet_detected) {
    const struct device *g_dev = DEVICE_DT_GET(GPIO_EMUL_LABEL);

    biz_logic_init();
    
    // Simulate Hall sensor going Active (High)
    gpio_emul_input_set(g_dev, 12, 1);
    
    biz_logic_process();

    zassert_equal(get_current_blink_interval(), 250, "Magnet interval should be 250ms");
}

/**
 * Test Case 3: Magnet + Button Pressed
 * Expectation: Interval = 100ms (Super Fast)
 */
ZTEST(biz_logic_tests, test_boost_mode) {
    const struct device *g_dev = DEVICE_DT_GET(GPIO_EMUL_LABEL);
    
    biz_logic_init();
    
    // Magnet High, Button Low (Pressed)
    gpio_emul_input_set(g_dev, 12, 1);
    gpio_emul_input_set(g_dev, 11, 0);
    
    biz_logic_process();

    zassert_equal(get_current_blink_interval(), 100, "Boost interval should be 100ms");
    
    // Also verify LED toggled (Check output pin 10)
    // Note: Since it was initialized inactive, a toggle should make it 1
    zassert_equal(gpio_emul_output_get(g_dev, 10), 1, "LED should have toggled to ON");
}