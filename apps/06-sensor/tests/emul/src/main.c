// The app's sensor code, the Bosch driver and the Zephyr I2C stack, unmodified,
// over an emulated chip. Tests assert properties (range, monotonicity, channel
// independence) rather than exact values, which would mean reimplementing the
// driver's compensation math. The one exception is the datasheet example.

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>

#include "bme280.h"
#include "bme280_emul.h"

/* BME280 datasheet operating ranges. A reading outside them means the
 * emulator's calibration blob is wrong; a bad blob still yields numbers. */
#define TEMP_MIN_MC   (-40000)
#define TEMP_MAX_MC     85000
#define PRESS_MIN_PA    30000  /*  300 hPa */
#define PRESS_MAX_PA   110000  /* 1100 hPa */
#define HUM_MIN_MRH         0
#define HUM_MAX_MRH    100000

static void emul_before(void *fixture)
{
	ARG_UNUSED(fixture);

	/* Restore the default codes so tests do not depend on run order.
	 * Setting these registers fires no callbacks, unlike gpio_emul. */
	bme280_emul_set_raw(BME280_EMUL_ADC_TEMP_DEFAULT,
			    BME280_EMUL_ADC_PRESS_DEFAULT,
			    BME280_EMUL_ADC_HUM_DEFAULT);
}

ZTEST_SUITE(climate_emul, NULL, NULL, emul_before, NULL, NULL);

ZTEST(climate_emul, test_reading_is_plausible)
{
	struct climate_reading r;

	zassert_ok(bme280_read_once(&r), "fetch through the emulated bus failed");

	TC_PRINT("T: %d mC | P: %d Pa | H: %d m%%RH\n",
		 r.temp_mc, r.press_mpa, r.hum_mrh);

	zassert_between_inclusive(r.temp_mc, TEMP_MIN_MC, TEMP_MAX_MC,
				  "temperature %d mC is outside the BME280 range",
				  r.temp_mc);
	zassert_between_inclusive(r.press_mpa, PRESS_MIN_PA, PRESS_MAX_PA,
				  "pressure %d Pa is outside the BME280 range",
				  r.press_mpa);
	zassert_between_inclusive(r.hum_mrh, HUM_MIN_MRH, HUM_MAX_MRH,
				  "humidity %d m%%RH is outside 0-100 %%RH",
				  r.hum_mrh);
}

ZTEST(climate_emul, test_temperature_is_monotonic_in_the_raw_code)
{
	/* A larger raw code must not report a colder room. Unlike the range
	 * check, this catches a sign inversion or swapped byte pair that still
	 * leaves every individual reading inside the sensor's range. */
	static const int32_t codes[] = {300000, 400000, 519888, 600000, 700000};
	int32_t prev_mc = INT32_MIN;

	for (int i = 0; i < ARRAY_SIZE(codes); i++) {
		struct climate_reading r;

		bme280_emul_set_raw(codes[i], BME280_EMUL_ADC_PRESS_DEFAULT,
				    BME280_EMUL_ADC_HUM_DEFAULT);

		zassert_ok(bme280_read_once(&r));
		TC_PRINT("adc_temp %d -> %d mC\n", codes[i], r.temp_mc);

		zassert_true(r.temp_mc > prev_mc,
			     "adc_temp %d reported %d mC, not warmer than the previous %d mC",
			     codes[i], r.temp_mc, prev_mc);
		prev_mc = r.temp_mc;
	}
}

ZTEST(climate_emul, test_humidity_does_not_move_temperature)
{
	/* The 8-byte burst carries pressure, temperature and humidity back to
	 * back, so an off-by-one in the emulator's packing bleeds one channel
	 * into another. */
	struct climate_reading a, b;

	zassert_ok(bme280_read_once(&a));

	bme280_emul_set_raw(BME280_EMUL_ADC_TEMP_DEFAULT,
			    BME280_EMUL_ADC_PRESS_DEFAULT,
			    BME280_EMUL_ADC_HUM_DEFAULT / 2);

	zassert_ok(bme280_read_once(&b));

	zassert_not_equal(a.hum_mrh, b.hum_mrh, "humidity did not change at all");
	zassert_equal(a.temp_mc, b.temp_mc,
		      "changing humidity moved temperature from %d to %d mC",
		      a.temp_mc, b.temp_mc);
	zassert_equal(a.press_mpa, b.press_mpa,
		      "changing humidity moved pressure from %d to %d Pa",
		      a.press_mpa, b.press_mpa);
}

ZTEST(climate_emul, test_datasheet_worked_example)
{
	/* The published result of the Bosch datasheet worked example (section
	 * 4.2.3), for the default codes and calibration blob the emulator
	 * serves. Packing, calibration encoding, driver math and climate_milli()
	 * all have to be right to land here. Humidity has no published example.
	 */
	struct climate_reading r;

	zassert_ok(bme280_read_once(&r));

	zassert_equal(r.temp_mc, 25080, "expected 25080 mC, got %d", r.temp_mc);
	zassert_equal(r.press_mpa, 100653, "expected 100653 Pa, got %d", r.press_mpa);
}
