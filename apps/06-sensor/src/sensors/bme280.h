#ifndef BME280_H
#define BME280_H

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
 * Separate from the printing thread so tests/emul can call it directly.
 */
int bme280_read_once(struct climate_reading *out);

/** Start the thread that reads and prints every 2 seconds. */
void bme280_start(void);

#endif /* BME280_H */
