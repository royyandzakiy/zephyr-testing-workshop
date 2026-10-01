#include "feeder.h"

void feeder_clear(struct feeder *f)
{
	/* Resets only the count. Old slot values stay in memory but are
	 * unreachable once count is zero. */
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
		/* Add a day before subtracting, so a slot earlier than now counts as tomorrow.
		 * The tempting `f->slots[i] - now_minute` wraps in unsigned `gap`: 06:00 seen
		 * from 22:00 becomes 64576, never beats the initialiser, and the caller is told
		 * the next feed is a day away. The feeder silently stops feeding. */
		uint16_t gap = (f->slots[i] + MINUTES_PER_DAY - now_minute) % MINUTES_PER_DAY;

		if (gap < best) {
			best = gap;
		}
	}

	*minutes_until = best;
	return true;
}
