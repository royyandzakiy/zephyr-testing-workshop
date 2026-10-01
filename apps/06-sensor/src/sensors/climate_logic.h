// The seam: fixed-point conversion with no Zephyr dependencies, like
// apps/02-ztest/src/blink_logic.h, so tests/unit needs no driver or board.

#ifndef CLIMATE_LOGIC_H_
#define CLIMATE_LOGIC_H_

#include <stdint.h>

/**
 * Collapse a Zephyr sensor_value (val1 + val2/1e6) into milli-units.
 *
 * Takes the two fields rather than the struct so this header needs no Zephyr
 * includes. Rounds to nearest; see climate_logic.c for why.
 */
int32_t climate_milli(int32_t val1, int32_t val2);

#endif /* CLIMATE_LOGIC_H_ */
