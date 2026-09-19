// src/main.c
//
// The composition root: the only file that knows both which auger
// implementation exists and which dispenser uses it.
//
// tests/fff brings its own main and its own auger_run(), which is why the
// dispenser never had to know.

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "dispenser.h"

#define PORTION_G 250

int main(void)
{
	struct dispenser d;

	printk("Pond feeder dispenser starting\n");
	dispenser_init(&d);

	while (1) {
		int ret = dispenser_feed(&d, PORTION_G);

		printk("feed -> %d | total %u g | jams %u\n",
		       ret, d.dispensed_g, d.jams);

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
