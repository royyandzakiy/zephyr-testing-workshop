// src/main.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

int main(void)
{
    int ret;

    printk("Hello from the Zephyr testing workshop!\n");
    printk("Board: %s\n", CONFIG_BOARD_TARGET);

    if (!device_is_ready(led.port)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return 0;
    }

    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure LED\n", ret);
        return 0;
    }

    /* The tick count comes from k_sleep(), so it only advances if the emulator
     * models the RTC that the nRF52 kernel timer runs on. A printk() before the
     * loop would succeed even on an emulator that models nothing but the UART.
     */
    /* The state is tracked here, not read back with gpio_pin_get_dt(). On nRF the
     * driver disconnects a pin's input buffer when it is configured as an output
     * only, so reading it back returns the same value whatever the LED is doing.
     */
    bool led_on = false;

    for (int tick = 1;; tick++) {
        k_sleep(K_SECONDS(1));
        led_on = !led_on;
        gpio_pin_set_dt(&led, led_on);
        printk("tick %d, led0 %s\n", tick, led_on ? "on" : "off");
    }

    return 0;
}
