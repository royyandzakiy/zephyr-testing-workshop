// src/main.c
//
// Walks a clock through one day and prints how long until the next feed, so
// the app does something you can watch. The schedule below is the same one
// most of the tests use.

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "feeder.h"

int main(void)
{
	struct feeder f;
	uint16_t now = 0;

	feeder_clear(&f);
	feeder_add(&f, 6 * 60);
	feeder_add(&f, 12 * 60);
	feeder_add(&f, 18 * 60);

	printk("Pond feeder, %d slots configured\n", feeder_count(&f));

	while (1) {
		uint16_t until;

		if (feeder_next(&f, now, &until)) {
			printk("%02u:%02u  next feed in %4u min%s\n",
			       now / 60, now % 60, until,
			       until == 0 ? "   FEED NOW" : "");
		}

		now = (now + 30) % MINUTES_PER_DAY;
		k_sleep(K_MSEC(300));
	}

	return 0;
}
