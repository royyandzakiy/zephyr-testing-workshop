// src/feeder.c

#include "feeder.h"

void feeder_clear(struct feeder *f)
{
	/* Only the count. The slot values are left where they are, because
	 * nothing can read them once count is zero. That is also why a test
	 * suite without a `before` hook can see yesterday's schedule. */
	f->count = 0;
}

bool feeder_add(struct feeder *f, uint16_t at_minute)
{
	if (at_minute >= MINUTES_PER_DAY) {
		return false;
	}

	if (f->count >= FEEDER_SLOTS_MAX) {
		return false;
	}

	f->slots[f->count] = at_minute;
	f->count++;
	return true;
}

int feeder_count(const struct feeder *f)
{
	return f->count;
}

bool feeder_next(const struct feeder *f, uint16_t now_minute, uint16_t *minutes_until)
{
	uint16_t best = MINUTES_PER_DAY; /* larger than any real gap */

	if (f->count == 0) {
		return false;
	}

	for (int i = 0; i < f->count; i++) {
		/* Add a full day before subtracting, then take the remainder.
		 * A slot earlier in the day than now belongs to tomorrow, and
		 * this is the line that says so.
		 *
		 * The tempting version is `f->slots[i] - now_minute`. These are
		 * unsigned, so 06:00 seen from 22:00 does not come out negative,
		 * it comes out as 64576. That never beats the initialiser below,
		 * so the function returns 1440 and the caller is told the next
		 * feed is a whole day away. Nothing looks wrong. The feeder just
		 * stops feeding. */
		uint16_t gap = (f->slots[i] + MINUTES_PER_DAY - now_minute) % MINUTES_PER_DAY;

		if (gap < best) {
			best = gap;
		}
	}

	*minutes_until = best;
	return true;
}
