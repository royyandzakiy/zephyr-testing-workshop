// src/blinky.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "blinky.h"
#include "blink_logic.h"

/* Works for BOTH physical and emulated pins -- whatever the devicetree
 * points these two aliases at is what this code drives. */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(led0), gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(DT_ALIAS(sw0), gpios);

static struct gpio_callback button_cb_data;
static bool led_state;

static void button_pressed_cb(const struct device *dev, struct gpio_callback *cb,
                              uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    led_state = blink_logic_toggle(led_state);
    gpio_pin_set_dt(&led, led_state);

    printk("Button pressed! LED is now %s\n", blink_logic_str(led_state));
}

int blinky_init(void)
{
    int ret;

    if (!device_is_ready(led.port)) {
        printk("Error: LED device not ready\n");
        return -ENODEV;
    }

    if (!device_is_ready(button.port)) {
        printk("Error: button device not ready\n");
        return -ENODEV;
    }

    led_state = false;

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure LED\n", ret);
        return ret;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure button\n", ret);
        return ret;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt\n", ret);
        return ret;
    }

    gpio_init_callback(&button_cb_data, button_pressed_cb, BIT(button.pin));

    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret != 0) {
        printk("Error %d: failed to add callback\n", ret);
        return ret;
    }

    return 0;
}
