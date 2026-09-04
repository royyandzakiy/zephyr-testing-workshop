// src/sensors/climate_logic.h
//
// THE SEAM.
//
// Everything this app *decides* about a reading, with no device, no
// devicetree, no I2C and no kernel in sight. Deliberately zero Zephyr
// dependencies -- the same rule apps/02-ztest/src/blink_logic.h follows -- so
// tests/unit needs no driver, no CONFIG_SENSOR and no board.
//
// This is where a sensor earns its place in a testing workshop. Toggling a
// GPIO is a boolean; fixed-point conversion and alarm hysteresis are real bug
// classes you can only catch by testing the arithmetic directly.

#ifndef CLIMATE_LOGIC_H_
#define CLIMATE_LOGIC_H_

#include <stdbool.h>
#include <stdint.h>

/* Alarm trips at the ON points and only releases below the OFF points. The
 * gap is the hysteresis -- without it a reading sitting exactly on the
 * threshold chatters the alarm on and off on every sample. */
#define CLIMATE_TEMP_ON_MC    30000  /*  30.0 degC */
#define CLIMATE_TEMP_OFF_MC   28000  /*  28.0 degC */
#define CLIMATE_HUM_ON_MRH    70000  /*  70.0 %RH  */
#define CLIMATE_HUM_OFF_MRH   65000  /*  65.0 %RH  */

/**
 * Collapse a Zephyr sensor_value (val1 + val2/1e6) into milli-units.
 *
 * Pass the two fields rather than the struct so this header stays free of
 * Zephyr includes. Rounds to nearest; see the note in the .c file.
 */
int32_t climate_milli(int32_t val1, int32_t val2);

/**
 * Should the alarm be on, given this reading and its own previous state?
 *
 * @param prev the alarm's last value -- passed in rather than held in a
 *             static, so the function stays pure and the hysteresis is
 *             something a test can drive directly.
 */
bool climate_alarm(int32_t temp_mc, int32_t hum_mrh, bool prev);

#endif /* CLIMATE_LOGIC_H_ */
