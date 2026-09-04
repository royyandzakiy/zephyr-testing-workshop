#include "button.h"
#include "led.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static struct gpio_callback button_cb_data;
static bool led_state = false;

static void button_pressed_cb(const struct device *dev, 
                              struct gpio_callback *cb, 
                              uint32_t pins)
{
    led_toggle();
    led_state = !led_state;
    printk("Button pressed! LED is now %s\n", led_state ? "ON" : "OFF");
}

void button_init(void)
{
    int ret;

    if (!device_is_ready(button.port)) {
        printk("Error: button device %s is not ready\n", button.port->name);
        return;
    }

    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure button\n", ret);
        return;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt\n", ret);
        return;
    }

    gpio_init_callback(&button_cb_data, button_pressed_cb, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret != 0) {
        printk("Error %d: failed to add callback\n", ret);
        return;
    }
}