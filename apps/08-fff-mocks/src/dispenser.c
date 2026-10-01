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

	/* Zero grams never reaches the motor: a zero-length run still spins the
	 * auger up and down, which spends battery on a solar feeder. */
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

	/* Count and report the jam, or a skipped meal looks exactly like a fed one. */
	d->jams++;
	return ret;
}
