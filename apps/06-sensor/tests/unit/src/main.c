// tests/unit/src/main.c

#include <zephyr/ztest.h>

#include "climate_logic.h"

ZTEST_SUITE(climate_logic, NULL, NULL, NULL, NULL, NULL);

/* ---- climate_milli(): the conversion ---- */

ZTEST(climate_logic, test_milli_whole_and_fraction)
{
	/* 25.080000 degC -> 25080 m degC */
	zassert_equal(climate_milli(25, 80000), 25080);
	/* 100.653000 kPa -> 100653 milli-kPa, which is Pa */
	zassert_equal(climate_milli(100, 653000), 100653);
	zassert_equal(climate_milli(0, 0), 0);
}

ZTEST(climate_logic, test_milli_rounds_to_nearest)
{
	/* The whole reason climate_milli() is not a one-liner. Truncating here
	 * would return 1000 for all four of these, losing up to a full
	 * milli-unit off every reading, always downward. */
	zassert_equal(climate_milli(1, 400), 1000);   /* 1.000400 -> down */
	zassert_equal(climate_milli(1, 500), 1001);   /* exactly half -> up */
	zassert_equal(climate_milli(1, 600), 1001);   /* 1.000600 -> up   */
	zassert_equal(climate_milli(1, 1400), 1001);  /* 1.001400 -> down */
}

ZTEST(climate_logic, test_milli_negative_readings)
{
	/* Zephyr sensor_value carries the sign on BOTH fields for negatives,
	 * so rounding has to go away from zero on the negative side too --
	 * an easy thing to get backwards, and it only shows up below 0 degC. */
	zassert_equal(climate_milli(-5, -250000), -5250);
	zassert_equal(climate_milli(0, -500), -1);
	zassert_equal(climate_milli(0, -400), 0);
	zassert_equal(climate_milli(-40, 0), -40000);
}

/* ---- climate_alarm(): the decision ---- */

ZTEST(climate_logic, test_alarm_trips_on_either_channel)
{
	/* Comfortable room: nothing doing. */
	zassert_false(climate_alarm(22000, 45000, false));

	/* Temperature alone is enough, and so is humidity alone. */
	zassert_true(climate_alarm(CLIMATE_TEMP_ON_MC, 45000, false));
	zassert_true(climate_alarm(22000, CLIMATE_HUM_ON_MRH, false));

	/* One milli-unit below either trip point is still off. */
	zassert_false(climate_alarm(CLIMATE_TEMP_ON_MC - 1,
				    CLIMATE_HUM_ON_MRH - 1, false));
}

ZTEST(climate_logic, test_alarm_hysteresis_latches)
{
	/* The band between the OFF and ON points is where hysteresis lives:
	 * the same reading gives a DIFFERENT answer depending on where the
	 * alarm already is. Without that, a reading parked on the threshold
	 * chatters the alarm on every sample. */
	const int32_t in_band = (CLIMATE_TEMP_ON_MC + CLIMATE_TEMP_OFF_MC) / 2;

	zassert_false(climate_alarm(in_band, 45000, false),
		      "in-band reading must not trip an alarm that is off");
	zassert_true(climate_alarm(in_band, 45000, true),
		     "in-band reading must not release an alarm that is on");
}

ZTEST(climate_logic, test_alarm_releases_only_below_both_off_points)
{
	/* Latched on. Temperature has come down but humidity has not. */
	zassert_true(climate_alarm(20000, CLIMATE_HUM_OFF_MRH + 1, true));

	/* Both back under their release points: now it clears. */
	zassert_false(climate_alarm(CLIMATE_TEMP_OFF_MC, CLIMATE_HUM_OFF_MRH, true));
}

ZTEST(climate_logic, test_alarm_sweep_up_and_down_is_asymmetric)
{
	/* Walk temperature up past the trip point and back down, carrying the
	 * state forward the way the application does, and record where it
	 * switches each way. If the two switch points are the same there is no
	 * hysteresis, whatever the constants claim. */
	int32_t turned_on_at = 0;
	int32_t turned_off_at = 0;
	bool alarm = false;

	for (int32_t t = 20000; t <= 35000; t += 100) {
		bool next = climate_alarm(t, 40000, alarm);

		if (next && !alarm) {
			turned_on_at = t;
		}
		alarm = next;
	}

	for (int32_t t = 35000; t >= 20000; t -= 100) {
		bool next = climate_alarm(t, 40000, alarm);

		if (!next && alarm) {
			turned_off_at = t;
		}
		alarm = next;
	}

	TC_PRINT("on at %d mC, off at %d mC\n", turned_on_at, turned_off_at);

	zassert_true(turned_on_at > 0, "alarm never turned on during the sweep");
	zassert_true(turned_off_at > 0, "alarm never turned off during the sweep");
	zassert_true(turned_off_at < turned_on_at,
		     "no hysteresis: switched on at %d and off at %d",
		     turned_on_at, turned_off_at);
}
