// tests/unit/src/main.c
//
// The conversion, on its own. No driver, no bus, no board, so this suite
// builds and runs in a second.
//
// Small on purpose. This app is about the chip and the emulator; pure-logic
// ztest is apps/02-ztest's subject and how to write it well is
// apps/07-unit-conventions'. What earns its place here is the one piece of
// arithmetic the emulated read path depends on.

#include <zephyr/ztest.h>

#include "climate_logic.h"

ZTEST_SUITE(climate_logic, NULL, NULL, NULL, NULL, NULL);

ZTEST(climate_logic, test_milli_whole_and_fraction)
{
	/* 25.080000 degC -> 25080 m degC. The same two numbers that
	 * tests/emul's test_datasheet_worked_example lands on end to end. */
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
