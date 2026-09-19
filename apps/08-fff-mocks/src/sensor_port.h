// src/sensor_port.h
//
// THE SEAM, drawn one level higher than in apps/06-sensor.
//
// app 06 faked the *chip*: an emulated BME280 on an emulated I2C bus, with
// the real Bosch driver in the middle. That is the right answer when what you
// want to test is the driver integration.
//
// This header fakes the *dependency* instead. climate_service.c knows there
// is a function called sensor_port_read() and nothing else. Whether the
// implementation behind it talks to a BME280, a thermocouple or a struct
// literal in a test file is not its problem.
//
// Both are useful and they answer different questions:
//
//   emulator  -- "does my code drive this chip correctly?"
//   fake port -- "does my code do the right thing when the chip returns
//                 -EIO three times in a row?"
//
// The second question is miserable to answer with an emulator, because you
// have to make a plausible chip misbehave in a specific way. With a fake it
// is one line.

#ifndef SENSOR_PORT_H_
#define SENSOR_PORT_H_

#include <stdbool.h>
#include <stdint.h>

struct climate_raw {
	int32_t temp_mc;
	int32_t press_pa;
	int32_t hum_mrh;
};

/** Is the sensor usable? Checked once at startup. */
bool sensor_port_ready(void);

/**
 * Read one sample.
 *
 * @retval 0        *out was filled
 * @retval negative errno from the driver; *out is undefined
 */
int sensor_port_read(struct climate_raw *out);

#endif /* SENSOR_PORT_H_ */
