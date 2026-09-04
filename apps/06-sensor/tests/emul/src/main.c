// tests/emul/src/main.c
//
// The application's own sensor code, the real Bosch BME280 driver, and the
// real Zephyr I2C stack -- all of it unmodified. Underneath sits an emulated
// controller and 200 lines of fake chip, and app.overlay is the only thing
// that said so.
//
// Note what is NOT asserted here: exact engineering values. The driver runs
// Bosch's fixed-point compensation over the calibration blob, so pinning an
// expected temperature would mean reimplementing that math in the test, and
// then the test would only be checking our arithmetic against itself. What is
// asserted instead are properties that hold regardless of the math:
// monotonicity, plausible range, and channel independence.

#include <zephyr/ztest.h>
#include <zephyr/kernel.h>

#include "bme280.h"
#include "bme280_emul.h"

/* BME280 datasheet operating ranges. A reading outside these means the
 * emulator's calibration blob is wrong, which is the failure mode worth
 * catching -- an invented blob still produces numbers, just absurd ones. */
#define TEMP_MIN_MC   (-40000)
#define TEMP_MAX_MC     85000
#define PRESS_MIN_PA    30000  /*  300 hPa */
#define PRESS_MAX_PA   110000  /* 1100 hPa */
#define HUM_MIN_MRH         0
#define HUM_MAX_MRH    100000

static void emul_before(void *fixture)
{
	ARG_UNUSED(fixture);

	/* Put the fake chip back to its default codes so the tests below do
	 * not depend on the order ztest happens to run them in. Unlike
	 * gpio_emul, setting registers here fires no callbacks, so a plain
	 * per-test reset is safe. */
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
	/* A real property of the compensation formula, and it needs no
	 * arithmetic to state: a larger raw code must not report a colder
	 * room. Where the range check only asks whether one number looks
	 * sane, this pins the shape of the whole transfer function -- so it
	 * still bites on a sign inversion or a swapped byte pair that happens
	 * to leave every individual reading inside the sensor's range. */
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
	/* Channel independence. The 8-byte burst carries pressure, temperature
	 * and humidity back to back, so an off-by-one in the emulator's
	 * packing shows up as one channel bleeding into another. */
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
	/* A characterization test, and worth being explicit about what that
	 * means: these two numbers are the published result of the Bosch
	 * datasheet worked example (section 4.2.3) for adc_T = 519888 and
	 * adc_P = 415148 with the calibration blob in bme280_emul.c. Because
	 * the emulator serves that exact blob and those exact codes, the whole
	 * chain -- register packing, calibration encoding, driver math,
	 * climate_milli() -- has to be right end to end to land here.
	 *
	 * Humidity is deliberately left out: the datasheet publishes no worked
	 * example for it, so any number here would be recorded from our own
	 * run and would prove nothing beyond "unchanged since last time".
	 */
	struct climate_reading r;

	zassert_ok(bme280_read_once(&r));

	zassert_equal(r.temp_mc, 25080, "expected 25080 mC, got %d", r.temp_mc);
	zassert_equal(r.press_mpa, 100653, "expected 100653 Pa, got %d", r.press_mpa);
}
