// Feeding schedule for a pond feeder: a few times of day, and how long until
// the next feed. No Zephyr dependencies, and the time of day is a parameter
// rather than read from a clock, so a test can ask about 23:59 without waiting.

#ifndef FEEDER_H_
#define FEEDER_H_

#include <stdbool.h>
#include <stdint.h>

#define FEEDER_SLOTS_MAX 4
#define MINUTES_PER_DAY  1440

struct feeder {
	uint16_t slots[FEEDER_SLOTS_MAX]; /* minutes since midnight, 0..1439 */
	uint8_t count;
};

/** Drop every slot. Safe on a struct that has never been used. */
void feeder_clear(struct feeder *f);

/**
 * Add a feeding time.
 *
 * @retval true  accepted
 * @retval false rejected: the schedule is full, or @p at_minute is not a real
 *               time of day
 */
bool feeder_add(struct feeder *f, uint16_t at_minute);

/** How many slots are configured, 0..FEEDER_SLOTS_MAX. */
int feeder_count(const struct feeder *f);

/**
 * Minutes from @p now_minute until the next feed, wrapping past midnight.
 *
 * @retval true  *minutes_until was written; 0 means feed now
 * @retval false the schedule is empty and *minutes_until is untouched
 *
 * Returns a bool rather than a sentinel because 0 is a valid answer, leaving
 * no spare value to mean "nothing scheduled".
 */
bool feeder_next(const struct feeder *f, uint16_t now_minute, uint16_t *minutes_until);

#endif /* FEEDER_H_ */
