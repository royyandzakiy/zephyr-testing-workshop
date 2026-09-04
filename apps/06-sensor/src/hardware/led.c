#include "led.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void led_init(void)
{
    int ret;

    if (led.port == NULL || !device_is_ready(led.port)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure LED\n", ret);
        return;
    }
}

void led_toggle(void)
{
    gpio_pin_toggle_dt(&led);
}

void led_set(bool state)
{
    gpio_pin_set_dt(&led, state ? 1 : 0);
}