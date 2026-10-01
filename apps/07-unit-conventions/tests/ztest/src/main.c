// Unit-test conventions to copy, in the order they appear below. The module
// under test is kept small so the shape of the tests is the point.
//
//   1. One behaviour per test. Four unrelated assertions report one failure
//      and hide three.
//   2. A name that states the behaviour, so the summary line says what broke.
//      `test_next_2` makes you open the file.
//   3. Arrange, Act, Assert, in that order, separated by blank lines.
//   4. Assertion messages that print the offending value: "got 64576, want 480"
//      ends an investigation that "assertion failed" starts.
//   5. A `before` hook, so a test cannot inherit the previous test's state.
//   6. A table when the bodies would be copy-paste, separate tests when not.

#include <zephyr/ztest.h>

#include "feeder.h"

/* The schedule used by most tests: 06:00, 12:00, 18:00. */
#define FEED_MORNING (6 * 60)
#define FEED_MIDDAY  (12 * 60)
#define FEED_EVENING (18 * 60)

/* ------------------------------------------------------------------------ */
/* Suite 1: no fixture needed.                                              */
/*                                                                          */
/* A struct on the stack is cheaper than a fixture and cannot leak between  */
/* tests. Use a fixture when setup is long, not by reflex.                  */
/* ------------------------------------------------------------------------ */

ZTEST_SUITE(feeder_basic, NULL, NULL, NULL, NULL, NULL);

ZTEST(feeder_basic, test_a_new_schedule_has_no_slots)
{
	/* Arrange */
	struct feeder f;

	/* Act */
	feeder_clear(&f);

	/* Assert */
	zassert_equal(feeder_count(&f), 0, "new schedule reported %d slots",
		      feeder_count(&f));
}

ZTEST(feeder_basic, test_next_feed_on_an_empty_schedule_fails)
{
	struct feeder f;
	uint16_t out = 1234;

	feeder_clear(&f);

	/* Two assertions, one behaviour: the contract is "returns false and
	 * leaves *minutes_until alone". */
	zassert_false(feeder_next(&f, 0, &out), "next feed found on an empty schedule");
	zassert_equal(out, 1234, "next feed wrote %u to *out on failure", out);
}

ZTEST(feeder_basic, test_one_slot_is_its_own_next_feed)
{
	struct feeder f;
	uint16_t out;

	feeder_clear(&f);
	zassert_true(feeder_add(&f, FEED_MORNING));

	zassert_true(feeder_next(&f, FEED_MORNING, &out));

	zassert_equal(out, 0, "standing on the only feed time reported %u minutes", out);
}

/* ------------------------------------------------------------------------ */
/* Suite 2: with a fixture, and the hook that makes it safe.                */
/*                                                                          */
/* ZTEST_F builds the type name struct <suite>_fixture by token pasting,    */
/* so a mismatched name fails inside the macro rather than at your typo.    */
/* ------------------------------------------------------------------------ */

struct feeder_edit_fixture {
	struct feeder f;
	int rejected;
};

static void *edit_setup(void)
{
	/* Runs once per suite, so allocate here rather than in before(). */
	static struct feeder_edit_fixture fixture;

	return &fixture;
}

static void edit_before(void *arg)
{
	/* Runs before every test, so no test inherits the previous one's
	 * schedule. Without it the suite passes or fails depending on test
	 * order, which ztest sets by test name, not by position in the file. */
	struct feeder_edit_fixture *f = arg;

	feeder_clear(&f->f);
	f->rejected = 0;
}

static void edit_after(void *arg)
{
	/* Runs after every test, pass or fail. Release heap buffers or close
	 * files here; this suite has nothing to release. */
	ARG_UNUSED(arg);
}

ZTEST_SUITE(feeder_edit, NULL, edit_setup, edit_before, edit_after, NULL);

static void add_all(struct feeder_edit_fixture *fx, const uint16_t *times, size_t n)
{
	for (size_t i = 0; i < n; i++) {
		if (!feeder_add(&fx->f, times[i])) {
			fx->rejected++;
		}
	}
}

ZTEST_F(feeder_edit, test_adding_past_the_last_slot_is_rejected)
{
	static const uint16_t times[] = {60, 120, 180, 240, 300, 360};

	add_all(fixture, times, ARRAY_SIZE(times));

	zassert_equal(feeder_count(&fixture->f), FEEDER_SLOTS_MAX,
		      "schedule holds %d slots, want %d",
		      feeder_count(&fixture->f), FEEDER_SLOTS_MAX);
	zassert_equal(fixture->rejected, (int)ARRAY_SIZE(times) - FEEDER_SLOTS_MAX,
		      "%d adds were rejected", fixture->rejected);
}

ZTEST_F(feeder_edit, test_a_fresh_schedule_accepts_the_first_slot)
{
	zassert_true(feeder_add(&fixture->f, FEED_MORNING),
		     "a fresh schedule refused its first slot");

	zassert_equal(feeder_count(&fixture->f), 1, "schedule holds %d slots, want 1",
		      feeder_count(&fixture->f));
}

ZTEST_F(feeder_edit, test_a_time_past_midnight_is_rejected)
{
	/* 1439 (23:59) is the last legal minute of the day. */
	zassert_true(feeder_add(&fixture->f, MINUTES_PER_DAY - 1), "23:59 was refused");
	zassert_false(feeder_add(&fixture->f, MINUTES_PER_DAY), "minute 1440 was accepted");
}

/* ------------------------------------------------------------------------ */
/* Suite 3: table-driven cases.                                             */
/*                                                                          */
/* ztest has no parametrize, so a table and a loop stand in for it. The     */
/* table reports as ONE test and stops at the first failure; naming each    */
/* row recovers most of what that loses.                                    */
/* ------------------------------------------------------------------------ */

ZTEST_SUITE(feeder_timing, NULL, NULL, NULL, NULL, NULL);

static void load_three_meals(struct feeder *f)
{
	feeder_clear(f);
	feeder_add(f, FEED_MORNING);
	feeder_add(f, FEED_MIDDAY);
	feeder_add(f, FEED_EVENING);
}

ZTEST(feeder_timing, test_minutes_until_the_next_feed)
{
	static const struct {
		const char *name;
		uint16_t now;
		uint16_t want;
	} cases[] = {
		{"before the first feed",                 300,  60},
		{"standing exactly on a feed",            360,   0},
		{"between two feeds",                     400, 320},
		{"after the last feed, wraps to tomorrow", 1200, 600},
		{"one minute before midnight",           1439, 361},
		{"midnight itself",                         0, 360},
	};

	for (size_t i = 0; i < ARRAY_SIZE(cases); i++) {
		struct feeder f;
		uint16_t got;

		load_three_meals(&f);

		zassert_true(feeder_next(&f, cases[i].now, &got),
			     "case %zu (%s): no next feed found", i, cases[i].name);
		zassert_equal(got, cases[i].want,
			      "case %zu (%s): at minute %u got %u, want %u",
			      i, cases[i].name, cases[i].now, got, cases[i].want);
	}
}

ZTEST(feeder_timing, test_slots_out_of_order_still_find_the_nearest)
{
	/* Nothing sorts the slot table, so every entry must be checked. A test
	 * of its own because it is about the search, not the clock arithmetic. */
	struct feeder f;
	uint16_t got;

	feeder_clear(&f);
	feeder_add(&f, FEED_EVENING);
	feeder_add(&f, FEED_MORNING);
	feeder_add(&f, FEED_MIDDAY);

	zassert_true(feeder_next(&f, 300, &got));

	zassert_equal(got, 60, "nearest feed from 05:00 was %u minutes away, want 60", got);
}

/* ------------------------------------------------------------------------ */
/* Suite 4: a suite predicate, and skipping honestly.                       */
/* ------------------------------------------------------------------------ */

static bool schedule_holds_two_meals(const void *state)
{
	/* Decides at run time, on the device, whether the whole suite runs.
	 * platform_allow in testcase.yaml is decided by twister before the build. */
	ARG_UNUSED(state);
	return FEEDER_SLOTS_MAX >= 2;
}

ZTEST_SUITE(feeder_capacity, schedule_holds_two_meals, NULL, NULL, NULL, NULL);

ZTEST(feeder_capacity, test_the_schedule_holds_at_least_two_feeds)
{
	struct feeder f;

	feeder_clear(&f);
	zassert_true(feeder_add(&f, FEED_MORNING));
	zassert_true(feeder_add(&f, FEED_EVENING));

	zassert_equal(feeder_count(&f), 2, "schedule holds %d slots", feeder_count(&f));
}

ZTEST(feeder_capacity, test_skipped_when_the_schedule_is_odd)
{
	/* ztest_test_skip() reports SKIP, not PASS, so a test that did nothing
	 * cannot show up as green. */
	if (FEEDER_SLOTS_MAX % 2 != 0) {
		ztest_test_skip();
	}

	zassert_equal(FEEDER_SLOTS_MAX % 2, 0);
}
