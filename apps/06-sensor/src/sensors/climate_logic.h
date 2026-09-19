// src/sensors/climate_logic.h
//
// THE SEAM.
//
// One function, with no device, no devicetree, no I2C and no kernel in sight.
// Deliberately zero Zephyr dependencies, the same rule
// apps/02-ztest/src/blink_logic.h follows, so tests/unit needs no driver, no
// CONFIG_SENSOR and no board.
//
// Fixed-point conversion is where a sensor driver quietly goes wrong. The
// driver hands back a value in two pieces and every caller has to put them
// back together the same way.

#ifndef CLIMATE_LOGIC_H_
#define CLIMATE_LOGIC_H_

#include <stdint.h>

/**
 * Collapse a Zephyr sensor_value (val1 + val2/1e6) into milli-units.
 *
 * Pass the two fields rather than the struct so this header stays free of
 * Zephyr includes. Rounds to nearest; see the note in the .c file.
 */
int32_t climate_milli(int32_t val1, int32_t val2);

#endif /* CLIMATE_LOGIC_H_ */
