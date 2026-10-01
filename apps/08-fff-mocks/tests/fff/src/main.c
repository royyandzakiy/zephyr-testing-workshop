// FFF, the Fake Function Framework, ships with Zephyr as <zephyr/fff.h>.
// Each FAKE_VALUE_FUNC or FAKE_VOID_FUNC gives you:
//
//   <fn>_fake.call_count        how many times it was called
//   <fn>_fake.arg0_val          the first argument on the last call
//   <fn>_fake.arg0_history[i]   the first argument on call i
//   <fn>_fake.return_val        what it hands back
//   SET_RETURN_SEQ(<fn>, ...)   a different answer per call
//   RESET_FAKE(<fn>)            wipe all of the above
//
// The fake below is this binary's only auger_run(), because CMakeLists.txt does
// not link src/auger_port_sim.c. No --wrap, weak symbols or #ifdef TEST needed.

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

	/* Reset every fake before each test, or results depend on test order. */
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

	/* Proves nothing asked the motor to run, which watching a motor cannot. */
	zassert_equal(auger_run_fake.call_count, 0,
		      "the motor was told to run for nothing");
}

ZTEST(dispenser_fff, test_a_jam_is_retried_and_the_feed_still_counts)
{
	/* One return value per call: jam once, then recover. A real motor cannot
	 * be made to do that on cue. */
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
