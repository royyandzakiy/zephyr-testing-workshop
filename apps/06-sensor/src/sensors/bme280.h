#ifndef BME280_H
#define BME280_H

#include <stdbool.h>
#include <stdint.h>

/** One reading, already collapsed to integer milli-units by climate_milli(). */
struct climate_reading {
	int32_t temp_mc;   /* milli-degrees C            */
	int32_t press_mpa; /* milli-kPa, i.e. Pa         */
	int32_t hum_mrh;   /* milli-%RH                  */
};

/**
 * Fetch one sample and convert it. 0 on success, negative errno otherwise.
 *
 * Split out from the printing thread on purpose: tests/emul calls this
 * directly. If the only way to read the sensor were to start the thread that
 * prints forever, there would be nothing a test could call.
 */
int bme280_read_once(struct climate_reading *out);

/** Current alarm state, as last computed by the sensor thread. */
bool bme280_alarm_state(void);

/** Start the thread that reads and prints every 2 seconds. */
void bme280_start(void);

#endif /* BME280_H */
