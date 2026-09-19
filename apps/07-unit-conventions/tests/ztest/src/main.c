// tests/ztest/src/main.c
//
// THE CONVENTIONS FILE.
//
// The module under test is deliberately small. What is worth copying from this
// file is the shape of the tests, not the cleverness of the code they cover.
//
// The habits, in the order they show up below:
//
//   1. One behaviour per test. A test with four assertions about four
//      different things reports one failure and hides three.
//   2. A name that states the behaviour. `test_next_feed_wraps_past_midnight`
//      tells you what broke from the summary line alone. `test_next_2` makes
//      you open the file.
//   3. Arrange, Act, Assert, in that order, with a blank line between. Once
//      every test in a repo has that shape you stop reading them and start
//      scanning them.
//   4. Assertion messages that print the offending value. "assertion failed"
//      is the start of an investigation; "got 64576, want 480" ends it.
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
/* A struct on the stack is cheaper than a fixture and impossible to leak    */
/* between tests. Reach for a fixture when setup is long, not as a reflex.   */
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

	/* Two assertions, one behaviour: the contract is "returns false AND
	 * leaves *minutes_until alone". Splitting them would split a sentence
	 * in half. */
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
/* The type MUST be named struct <suite>_fixture. ZTEST_F builds that name   */
/* by token pasting, so a mismatch gives you an error pointing at the macro  */
/* rather than at your typo.                                                 */
/* ------------------------------------------------------------------------ */

struct feeder_edit_fixture {
	struct feeder f;
	int rejected;
};

static void *edit_setup(void)
{
	/* Runs ONCE for the suite. Allocate here, not in before(). */
	static struct feeder_edit_fixture fixture;

	return &fixture;
}

static void edit_before(void *arg)
{
	/* Runs before EVERY test. This is the line that makes the suite
	 * order-independent, and it is the one people leave out. Without it,
	 * the test below inherits a schedule that is already full, and the
	 * suite stays green only as long as nobody reorders it. */
	struct feeder_edit_fixture *f = arg;

	feeder_clear(&f->f);
	f->rejected = 0;
}

static void edit_after(void *arg)
{
	/* Runs after every test, pass or fail. Nothing to release here, but a
	 * heap buffer or an open file would go in this function. */
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
	/* This is the test that fails when edit_before is removed, because the
	 * test above leaves the schedule full. */
	zassert_true(feeder_add(&fixture->f, FEED_MORNING),
		     "a fresh schedule refused its first slot");

	zassert_equal(feeder_count(&fixture->f), 1, "schedule holds %d slots, want 1",
		      feeder_count(&fixture->f));
}

ZTEST_F(feeder_edit, test_a_time_past_midnight_is_rejected)
{
	/* 1439 is 23:59 and is the last legal minute of the day. */
	zassert_true(feeder_add(&fixture->f, MINUTES_PER_DAY - 1), "23:59 was refused");
	zassert_false(feeder_add(&fixture->f, MINUTES_PER_DAY), "minute 1440 was accepted");
}

/* ------------------------------------------------------------------------ */
/* Suite 3: table-driven cases.                                             */
/*                                                                          */
/* ztest has no parametrize. A table plus a loop is the substitute, and it   */
/* costs you something real: the whole table reports as ONE test, and it     */
/* stops at the first failure. Naming each row is what claws back most of    */
/* the loss.                                                                 */
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
	/* Nothing sorts the slot table, so every entry has to be considered.
	 * Its own test rather than a table row, because it is about the search
	 * and not about the clock arithmetic. */
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
	/* The predicate decides whether the whole suite runs, and it is
	 * evaluated on the device at run time. That is the difference between
	 * it and platform_allow in testcase.yaml, which twister evaluates
	 * before anything is built. */
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
	/* ztest_test_skip() reports SKIP, not PASS. Worth the extra line: a
	 * suite that goes green because it did nothing is the most expensive
	 * kind of green there is. */
	if (FEEDER_SLOTS_MAX % 2 != 0) {
		ztest_test_skip();
	}

	zassert_equal(FEEDER_SLOTS_MAX % 2, 0);
}
