// The shipping auger_run() for a laptop, not a test double: no test links it.
// Jams every third call so the dispenser's retry shows when you run the app.
// On a real feeder this file drives the motor and nothing above it changes.

#include <errno.h>

#include <zephyr/sys/printk.h>

#include "auger_port.h"

static uint32_t calls;

int auger_run(uint16_t grams)
{
	calls++;

	if (calls % 3 == 0) {
		printk("auger: JAM\n");
		return -EIO;
	}

	printk("auger: %u g\n", grams);
	return 0;
}
