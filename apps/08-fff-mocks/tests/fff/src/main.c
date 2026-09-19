// tests/fff/src/main.c
//
// FFF, the Fake Function Framework. One header, already vendored in Zephyr as
// <zephyr/fff.h>, so there is nothing to add to west.yml.
//
// For a function declared with FAKE_VALUE_FUNC or FAKE_VOID_FUNC you get:
//
//   <fn>_fake.call_count        how many times it was called
//   <fn>_fake.arg0_val          the first argument on the last call
//   <fn>_fake.arg0_history[i]   the first argument on call i
//   <fn>_fake.return_val        what it hands back
//   SET_RETURN_SEQ(<fn>, ...)   a different answer per call
//   RESET_FAKE(<fn>)            wipe all of the above
//
// WHAT MAKES THIS WORK: CMakeLists.txt in this directory does not link
// src/auger_port_sim.c. The fake below IS the definition of auger_run() for
// this binary. No --wrap, no weak symbols, no #ifdef TEST in production code.

#include <errno.h>

#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include "dispenser.h"
#include "auger_port.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, auger_run, uint16_t);

static struct dispenser d;

static void fff_before(void *f)
{
	ARG_UNUSED(f);

	/* The fake you forget to reset is the one that makes a test pass only
	 * when the whole suite runs in order. */
	RESET_FAKE(auger_run);
	FFF_RESET_HISTORY();

	auger_run_fake.return_val = 0;
	dispenser_init(&d);
}

ZTEST_SUITE(dispenser_fff, NULL, NULL, fff_before, NULL, NULL);

ZTEST(dispenser_fff, test_feeding_runs_the_auger_once_with_the_portion_size)
{
	zassert_ok(dispenser_feed(&d, 250));

	zassert_equal(auger_run_fake.call_count, 1,
		      "auger ran %d times", auger_run_fake.call_count);
	zassert_equal(auger_run_fake.arg0_val, 250,
		      "auger was asked for %u g", auger_run_fake.arg0_val);
	zassert_equal(d.dispensed_g, 250);
}

ZTEST(dispenser_fff, test_zero_grams_never_reaches_the_motor)
{
	zassert_equal(dispenser_feed(&d, 0), -EINVAL);

	/* The assertion you cannot make by watching a motor: not "it did not
	 * turn" but "nothing ever asked it to". */
	zassert_equal(auger_run_fake.call_count, 0,
		      "the motor was told to run for nothing");
}

ZTEST(dispenser_fff, test_a_jam_is_retried_and_the_feed_still_counts)
{
	/* One entry per call, in order. Getting a real motor to jam exactly
	 * once and then recover is a morning's work. Here it is an array. */
	static int seq[] = {-EIO, 0};

	SET_RETURN_SEQ(auger_run, seq, ARRAY_SIZE(seq));

	zassert_ok(dispenser_feed(&d, 250));

	zassert_equal(auger_run_fake.call_count, 2, "no retry after the jam");
	zassert_equal(d.dispensed_g, 250);
	zassert_equal(d.jams, 0, "a recovered jam was counted as a failure");
}

ZTEST(dispenser_fff, test_two_jams_give_up_and_dispense_nothing)
{
	auger_run_fake.return_val = -EIO;

	zassert_equal(dispenser_feed(&d, 250), -EIO);

	zassert_equal(auger_run_fake.call_count, DISPENSER_ATTEMPTS,
		      "tried %d times", auger_run_fake.call_count);
	zassert_equal(d.dispensed_g, 0, "counted pellets that never left the hopper");
	zassert_equal(d.jams, 1);
}
