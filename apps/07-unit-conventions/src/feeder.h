// src/feeder.h
//
// A feeding schedule for a pond feeder. A handful of times of day, and one
// question: how long until the next feed?
//
// Small on purpose. The point of this app is how the tests are written, not
// how clever the module is.
//
// Zero Zephyr dependencies. Time of day arrives as a parameter rather than
// being read from a clock inside, which is what lets a test ask "what happens
// at 23:59" without waiting until 23:59.

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
 * Returning a bool rather than a sentinel is a testability decision. Zero is a
 * perfectly good answer, so there is no spare value left to mean "nothing
 * scheduled".
 */
bool feeder_next(const struct feeder *f, uint16_t now_minute, uint16_t *minutes_until);

#endif /* FEEDER_H_ */
