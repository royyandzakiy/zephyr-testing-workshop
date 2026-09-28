// src/main.c
//
// Same application as apps/04-shell-pytest, with one addition: it keeps a
// press counter and exposes its state through src/app_state.h. No shell
// commands are registered here. The backdoor lives entirely in the test
// build, which is the point.

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "app_state.h"

#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

static struct gpio_callback button_cb_data;

static bool led_state;
static uint32_t press_count;

bool app_led_state(void)
{
    return led_state;
}

uint32_t app_press_count(void)
{
    return press_count;
}

void app_press_count_reset(void)
{
    press_count = 0;
}

void button_pressed_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    ARG_UNUSED(dev);
    ARG_UNUSED(cb);
    ARG_UNUSED(pins);

    gpio_pin_toggle_dt(&led);
    led_state = !led_state;
    press_count++;

    printk("Button pressed! LED is now %s\n", led_state ? "ON" : "OFF");
}

int main(void)
{
    int ret;

    printk("GPIO Button + LED Toggle started\n");

    if (!device_is_ready(button.port)) {
        printk("Error: button device %s is not ready\n", button.port->name);
        return 0;
    }

    if (led.port == NULL || !device_is_ready(led.port)) {
        printk("Error: LED device is not ready\n");
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure LED\n", ret);
        return 0;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure button\n", ret);
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt\n", ret);
        return 0;
    }

    gpio_init_callback(&button_cb_data, button_pressed_cb, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret != 0) {
        printk("Error %d: failed to add callback\n", ret);
        return 0;
    }

    printk("Ready. Press the button to toggle LED.\n");

    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}
