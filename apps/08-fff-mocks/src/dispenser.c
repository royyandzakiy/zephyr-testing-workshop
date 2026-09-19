// src/dispenser.c

#include <errno.h>
#include <string.h>

#include "dispenser.h"
#include "auger_port.h"

void dispenser_init(struct dispenser *d)
{
	memset(d, 0, sizeof(*d));
}

int dispenser_feed(struct dispenser *d, uint16_t grams)
{
	int ret = -EIO;

	/* Asking for nothing is not an error worth logging a jam over, but it
	 * must not reach the motor either. A zero gram run still spins the
	 * auger up and back down, and on a solar feeder that is battery spent
	 * on nothing. There is a test named after this paragraph. */
	if (grams == 0) {
		return -EINVAL;
	}

	for (int i = 0; i < DISPENSER_ATTEMPTS; i++) {
		ret = auger_run(grams);
		if (ret == 0) {
			d->dispensed_g += grams;
			return 0;
		}
	}

	/* Count it and report it. A feeder that quietly skips a meal looks
	 * exactly like one that fed, right up until the fish are thin. */
	d->jams++;
	return ret;
}
