// src/dispenser.h
//
// apps/07-unit-conventions decides *when* to feed. This decides what happens
// when that moment arrives, which means calling out to a motor that can fail.
//
// No Zephyr headers, no devicetree, no driver. That is what makes the fakes in
// tests/fff possible at all: if dispenser.c called gpio_pin_set_dt() directly
// there would be nothing to substitute.

#ifndef DISPENSER_H_
#define DISPENSER_H_

#include <stdint.h>

/** Tries per portion. One retry, because most jams clear on a second turn. */
#define DISPENSER_ATTEMPTS 2

struct dispenser {
	uint32_t dispensed_g;
	uint32_t jams;
};

/** Zero the counters. */
void dispenser_init(struct dispenser *d);

/**
 * Put one portion in the water.
 *
 * @retval 0       the portion went out, possibly on the second attempt
 * @retval -EIO    every attempt jammed, nothing was dispensed
 * @retval -EINVAL @p grams was zero
 */
int dispenser_feed(struct dispenser *d, uint16_t grams);

#endif /* DISPENSER_H_ */
