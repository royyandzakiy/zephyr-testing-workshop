// src/auger_port_sim.c
//
// The shipping implementation of auger_port.h, for a laptop. Jams every third
// call so the retry in dispenser.c is visible when you run the app.
//
// Worth noticing what this is NOT: it is not a test double. It ships, it
// asserts nothing, and no test links it. On a real feeder this file is the one
// that talks to the motor driver, and nothing above it changes.

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
