// The conversion on its own, with no driver, bus or board. Kept small: pure
// logic tests are covered in apps/02-ztest and apps/07-unit-conventions.

#include <zephyr/ztest.h>

#include "climate_logic.h"

ZTEST_SUITE(climate_logic, NULL, NULL, NULL, NULL, NULL);

ZTEST(climate_logic, test_milli_whole_and_fraction)
{
	/* 25.080000 degC -> 25080 m degC, the value tests/emul's
	 * test_datasheet_worked_example reaches end to end. */
	zassert_equal(climate_milli(25, 80000), 25080);
	/* 100.653000 kPa -> 100653 milli-kPa, which is Pa */
	zassert_equal(climate_milli(100, 653000), 100653);
	zassert_equal(climate_milli(0, 0), 0);
}

ZTEST(climate_logic, test_milli_rounds_to_nearest)
{
	/* Truncating would turn 1.000500 and 1.000600 into 1000, losing up
	 * to a milli-unit off every reading, always downward. */
	zassert_equal(climate_milli(1, 400), 1000);   /* 1.000400 -> down */
	zassert_equal(climate_milli(1, 500), 1001);   /* exactly half -> up */
	zassert_equal(climate_milli(1, 600), 1001);   /* 1.000600 -> up   */
	zassert_equal(climate_milli(1, 1400), 1001);  /* 1.001400 -> down */
}
