#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "sensors/bme280.h"

int main(void)
{
	printk("System Started\n");

	bme280_start();

	/* The sensor thread does all the work from here. */
	while (1) {
		k_sleep(K_FOREVER);
	}

	return 0;
}
