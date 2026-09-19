// src/alarm_port_led.c
//
// The production implementation of alarm_port.h. An LED, through the led0
// alias, exactly as apps/01-blinky does it.

#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#include "alarm_port.h"

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

void alarm_port_set(bool on)
{
	if (!device_is_ready(led.port)) {
		printk("alarm: LED not ready, state would be %s\n", on ? "ON" : "OFF");
		return;
	}

	gpio_pin_configure_dt(&led, GPIO_OUTPUT);
	gpio_pin_set_dt(&led, on ? 1 : 0);
	printk("alarm: %s\n", on ? "ON" : "OFF");
}
