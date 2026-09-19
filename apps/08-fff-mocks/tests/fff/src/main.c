// tests/fff/src/main.c
//
// FFF: Fake Function Framework. One header, a pile of macros, and it is
// already vendored in Zephyr as <zephyr/fff.h>, so there is nothing to add to
// west.yml.
//
// What it gives you, for a function you declare with FAKE_VALUE_FUNC or
// FAKE_VOID_FUNC:
//
//   <fn>_fake.call_count        how many times it was called
//   <fn>_fake.arg0_val          the last value of the first argument
//   <fn>_fake.arg0_history[i]   the value on call i
//   <fn>_fake.return_val        what it hands back
//   <fn>_fake.return_val_seq    a sequence, one per call
//   <fn>_fake.custom_fake       your own function body
//   RESET_FAKE(<fn>)            wipe all of the above
//
// It does not do expectations the way gmock does. There is no EXPECT_CALL
// that fails the test on its own; you record, then you assert afterwards. In
// C that trade is usually worth it, and it is the difference you will see in
// apps/09-gtest-gmock.
//
// THE THING THAT MAKES THIS WORK: CMakeLists.txt in this directory does NOT
// link sensor_port_bme280.c, sensor_port_sim.c or alarm_port_led.c. The fakes
// below ARE the definitions of those symbols for this binary. No linker
// wrapping, no weak symbols, no preprocessor. Just a header with no
// implementation attached, which is what sensor_port.h has been all along.

#include <errno.h>
#include <string.h>

#include <zephyr/ztest.h>
#include <zephyr/fff.h>

#include "climate_service.h"
#include "climate_logic.h"
#include "sensor_port.h"
#include "alarm_port.h"

DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(bool, sensor_port_ready);
FAKE_VALUE_FUNC(int, sensor_port_read, struct climate_raw *);
FAKE_VOID_FUNC(alarm_port_set, bool);

/* Listing the fakes once and looping over them beats remembering to add a
 * RESET_FAKE line every time somebody introduces a new dependency. The fake
 * you forget to reset is the one that makes a test pass only when the whole
 * suite runs in order. */
#define FFF_FAKES_LIST(FAKE)   \
	FAKE(sensor_port_ready) \
	FAKE(sensor_port_read)  \
	FAKE(alarm_port_set)

/* ------------------------------------------------------------------------ */
/* Custom fakes: a fake with a body.                                        */
/*                                                                          */
/* return_val gets you a return code. When the function communicates through */
/* an out-parameter, as sensor_port_read() does, you need a body -- and this */
/* is the point at which people usually discover FFF has one.                */
/* ------------------------------------------------------------------------ */

static struct climate_raw canned;

static int read_canned(struct climate_raw *out)
{
	*out = canned;
	return 0;
}

static int read_fails_eio(struct climate_raw *out)
{
	ARG_UNUSED(out);
	return -EIO;
}

/* A custom fake that changes its answer based on how many times it has been
 * called. arg_history is read-only bookkeeping; call_count is the counter you
 * can branch on. */
static int read_hot_after_two_calls(struct climate_raw *out)
{
	out->press_pa = 100000;
	out->hum_mrh = 40000;
	out->temp_mc = (sensor_port_read_fake.call_count > 2) ? 31000 : 25000;
	return 0;
}

static void fff_before(void *f)
{
	ARG_UNUSED(f);

	FFF_FAKES_LIST(RESET_FAKE);
	FFF_RESET_HISTORY();

	canned = (struct climate_raw){
		.temp_mc = 25000,
		.press_pa = 100000,
		.hum_mrh = 45000,
	};
	sensor_port_read_fake.custom_fake = read_canned;
	sensor_port_ready_fake.return_val = true;
}

ZTEST_SUITE(climate_fff, NULL, NULL, fff_before, NULL, NULL);

/* ------------------------------------------------------------------------ */
/* 1. Call counts: did the thing under test call its dependency at all?     */
/* ------------------------------------------------------------------------ */

ZTEST(climate_fff, test_tick_reads_the_sensor_exactly_once)
{
	struct climate_service svc;

	climate_service_init(&svc);

	zassert_ok(climate_service_tick(&svc));

	zassert_equal(sensor_port_read_fake.call_count, 1,
		      "sensor was read %d times", sensor_port_read_fake.call_count);
}

ZTEST(climate_fff, test_quiet_readings_never_touch_the_alarm)
{
	struct climate_service svc;

	climate_service_init(&svc);

	for (int i = 0; i < 5; i++) {
		zassert_ok(climate_service_tick(&svc));
	}

	/* The assertion that is impossible with an LED: not "the LED is off"
	 * but "nothing ever tried to change it". */
	zassert_equal(alarm_port_set_fake.call_count, 0,
		      "alarm was driven %d times with nothing to report",
		      alarm_port_set_fake.call_count);
}

/* ------------------------------------------------------------------------ */
/* 2. Argument capture: was it called with the right value?                 */
/* ------------------------------------------------------------------------ */

ZTEST(climate_fff, test_alarm_is_raised_once_when_it_gets_hot)
{
	struct climate_service svc;

	climate_service_init(&svc);
	canned.temp_mc = CLIMATE_TEMP_ON_MC + 500;

	zassert_ok(climate_service_tick(&svc));

	zassert_equal(alarm_port_set_fake.call_count, 1);
	zassert_true(alarm_port_set_fake.arg0_val, "alarm was raised with false");
}

ZTEST(climate_fff, test_alarm_is_driven_only_on_the_edge)
{
	struct climate_service svc;

	climate_service_init(&svc);
	canned.temp_mc = CLIMATE_TEMP_ON_MC + 500;

	/* Four hot ticks in a row. */
	for (int i = 0; i < 4; i++) {
		zassert_ok(climate_service_tick(&svc));
	}

	/* One call, not four. This is the edge detection in
	 * climate_service_tick(), and it is worth an entire test of its own:
	 * get it wrong and on a real product you publish an MQTT alert every
	 * two seconds for as long as the room stays warm. */
	zassert_equal(alarm_port_set_fake.call_count, 1,
		      "alarm driven %d times across four hot ticks",
		      alarm_port_set_fake.call_count);
}

ZTEST(climate_fff, test_alarm_history_records_the_full_on_off_cycle)
{
	struct climate_service svc;

	climate_service_init(&svc);

	canned.temp_mc = CLIMATE_TEMP_ON_MC + 500;
	zassert_ok(climate_service_tick(&svc));

	/* Below the release point, so hysteresis lets go. */
	canned.temp_mc = CLIMATE_TEMP_OFF_MC - 500;
	canned.hum_mrh = 40000;
	zassert_ok(climate_service_tick(&svc));

	zassert_equal(alarm_port_set_fake.call_count, 2);
	zassert_true(alarm_port_set_fake.arg0_history[0], "first edge was not ON");
	zassert_false(alarm_port_set_fake.arg0_history[1], "second edge was not OFF");
}

ZTEST(climate_fff, test_hysteresis_holds_the_alarm_between_the_thresholds)
{
	struct climate_service svc;

	climate_service_init(&svc);

	canned.temp_mc = CLIMATE_TEMP_ON_MC + 100;
	zassert_ok(climate_service_tick(&svc));

	/* Between OFF and ON. Should stay latched, so no second call. */
	canned.temp_mc = (CLIMATE_TEMP_ON_MC + CLIMATE_TEMP_OFF_MC) / 2;
	canned.hum_mrh = 40000;
	zassert_ok(climate_service_tick(&svc));

	zassert_equal(alarm_port_set_fake.call_count, 1,
		      "alarm let go inside the hysteresis band");
	zassert_true(svc.alarm);
}

/* ------------------------------------------------------------------------ */
/* 3. Return sequences: making the dependency misbehave.                    */
/*                                                                          */
/* This is the reason to reach for a fake rather than an emulator. Getting   */
/* an emulated BME280 to fail three reads and then recover is fiddly. Here   */
/* it is an array.                                                          */
/* ------------------------------------------------------------------------ */

ZTEST(climate_fff, test_read_error_is_counted_and_returned)
{
	struct climate_service svc;
	int ret;

	climate_service_init(&svc);
	sensor_port_read_fake.custom_fake = read_fails_eio;

	ret = climate_service_tick(&svc);

	zassert_equal(ret, -EIO, "tick returned %d", ret);
	zassert_equal(svc.errors, 1);
	zassert_equal(svc.reads, 0);
}

ZTEST(climate_fff, test_read_error_does_not_clear_a_raised_alarm)
{
	struct climate_service svc;

	climate_service_init(&svc);

	canned.temp_mc = CLIMATE_TEMP_ON_MC + 500;
	zassert_ok(climate_service_tick(&svc));
	zassert_true(svc.alarm);

	/* Sensor stops answering. */
	sensor_port_read_fake.custom_fake = read_fails_eio;
	zassert_equal(climate_service_tick(&svc), -EIO);

	zassert_true(svc.alarm, "a failed read cleared the alarm");
	zassert_equal(alarm_port_set_fake.call_count, 1,
		      "a failed read drove the alarm port");
}

ZTEST(climate_fff, test_three_consecutive_errors_latch_a_fault)
{
	struct climate_service svc;

	climate_service_init(&svc);
	sensor_port_read_fake.custom_fake = read_fails_eio;

	for (int i = 0; i < CLIMATE_MAX_CONSECUTIVE_ERRORS; i++) {
		zassert_equal(climate_service_tick(&svc), -EIO, "tick %d", i);
	}

	zassert_true(svc.faulted, "no fault after %d failures",
		     CLIMATE_MAX_CONSECUTIVE_ERRORS);

	/* Faulted means it stops asking. The fake proves it: call_count does
	 * not move. */
	zassert_equal(climate_service_tick(&svc), -EIO);
	zassert_equal(sensor_port_read_fake.call_count,
		      CLIMATE_MAX_CONSECUTIVE_ERRORS,
		      "service kept reading after faulting");
}

ZTEST(climate_fff, test_a_good_read_resets_the_consecutive_counter)
{
	struct climate_service svc;
	/* return_val_seq: one entry per call, in order. Two failures, one
	 * success, then a failure again -- which must NOT latch, because the
	 * run of three was broken. */
	static int seq[] = {-EIO, -EIO, 0, -EIO};

	climate_service_init(&svc);
	sensor_port_read_fake.custom_fake = NULL;
	SET_RETURN_SEQ(sensor_port_read, seq, ARRAY_SIZE(seq));

	for (int i = 0; i < 4; i++) {
		climate_service_tick(&svc);
	}

	zassert_false(svc.faulted, "latched a fault on non-consecutive errors");
	zassert_equal(svc.errors, 3, "counted %u errors", svc.errors);
	zassert_equal(svc.consecutive_errors, 1);
}

ZTEST(climate_fff, test_clearing_a_fault_lets_reads_resume)
{
	struct climate_service svc;

	climate_service_init(&svc);
	sensor_port_read_fake.custom_fake = read_fails_eio;

	for (int i = 0; i < CLIMATE_MAX_CONSECUTIVE_ERRORS; i++) {
		climate_service_tick(&svc);
	}
	zassert_true(svc.faulted);

	climate_service_clear_fault(&svc);
	sensor_port_read_fake.custom_fake = read_canned;

	zassert_ok(climate_service_tick(&svc));
	zassert_equal(svc.reads, 1);
}

/* ------------------------------------------------------------------------ */
/* 4. A stateful custom fake, for a scenario that unfolds over time.        */
/* ------------------------------------------------------------------------ */

ZTEST(climate_fff, test_room_warming_up_raises_the_alarm_on_the_third_tick)
{
	struct climate_service svc;

	climate_service_init(&svc);
	sensor_port_read_fake.custom_fake = read_hot_after_two_calls;

	for (int i = 0; i < 4; i++) {
		zassert_ok(climate_service_tick(&svc));
	}

	zassert_equal(alarm_port_set_fake.call_count, 1);
	zassert_true(alarm_port_set_fake.arg0_val);
	zassert_equal(svc.reads, 4);
}

/* ------------------------------------------------------------------------ */
/* 5. The limit of the technique.                                           */
/* ------------------------------------------------------------------------ */

ZTEST(climate_fff, test_fakes_say_nothing_about_the_real_implementation)
{
	/* Everything above passes with src/sensor_port_bme280.c deleted. That
	 * is the price of testing against a seam: you have pinned the service,
	 * and you have pinned nothing at all about whether the BME280 is wired
	 * to the right I2C address.
	 *
	 * Which is what apps/06-sensor is for. The two are not alternatives,
	 * they answer different halves of the same question, and a suite that
	 * only has one half is the one that goes green while the product does
	 * not work. */
	zassert_true(sensor_port_ready(), "the fake said no, which is also fine");
	zassert_equal(sensor_port_ready_fake.call_count, 1);
}
