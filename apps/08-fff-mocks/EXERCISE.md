# 08-fff-mocks - exercises

## ★ warm-up

1. **Link the real thing on purpose.** Add `src/alarm_port_led.c` to the
   `target_sources()` call in `tests/fff/CMakeLists.txt`, then build:

   ```bash
   west build -b native_sim/native -p -s apps/08-fff-mocks/tests/fff -d build_fff
   ```

   *Check:* you can quote the duplicate symbol error, and explain in one sentence why
   it is the whole mechanism this app relies on. Take the line back out when you are
   done.

2. **Forget a reset.** Remove `alarm_port_set` from the `FFF_FAKES_LIST` macro, then
   run the suite:

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim
   ```

   *Check:* you can name a test that now fails, and confirm it passes when you run it
   on its own. That combination is what state leaking between tests looks like. Put it
   back when you are done.

3. **Break the edge detection.** In `climate_service_tick()`, call
   `alarm_port_set(next)` unconditionally instead of only on a change, then run the
   suite.

   *Check:* `test_alarm_is_driven_only_on_the_edge` fails and prints the call count.
   Then name which of the other tests stayed green, and say why that is worth noticing.
   Put it back when you are done.

4. **Fake a function that does not exist yet.** Declare
   `int sensor_port_selftest(void)` in `sensor_port.h`, add a `FAKE_VALUE_FUNC` for it
   in the test, and write a test for behaviour you have not implemented.

   *Check:* the suite builds and the new test fails because the behaviour is missing,
   not because the link broke.

## ★★ go deeper

1. **Use `custom_fake_seq`.** FFF supports a sequence of custom fakes, not just a
   sequence of return values. Rewrite
   `test_room_warming_up_raises_the_alarm_on_the_third_tick` to use one. The macro list
   is in the header:

   ```bash
   grep -n "custom_fake_seq\|SET_CUSTOM_FAKE_SEQ" $ZEPHYR_BASE/subsys/testsuite/include/zephyr/fff.h
   ```

   *Check:* you can say whether the result reads better than the stateful fake it
   replaced, and why.

2. **Assert on the argument a fake was given.** `sensor_port_read` takes an
   out-pointer. Prove the service passes the same address every call, or prove it does
   not.

   *Check:* you used `arg0_history`, and you can say what `arg0_val` on its own would
   have missed.

3. **Add a retry policy and test it.** Make the service retry a failed read once before
   counting an error. Assert the call count, not just the outcome.

   *Check:* at least one existing test breaks, and you can say why that break is
   correct rather than a regression.

4. **Give `sensor_port.h` a third implementation.** A recorded trace read out of an
   array, for replaying a fault you saw in the field. Wire it into the `if(CONFIG_...)`
   block in `CMakeLists.txt`.

   *Check:* CMake selects it, the app still builds, and `git status` shows you changed
   no test file.

5. **Port one test to the emulator style.** Take
   `test_read_error_is_counted_and_returned` and write the equivalent against
   `apps/06-sensor`'s emulated BME280.

   ```bash
   time west twister -T apps/08-fff-mocks -p native_sim
   ```

   ```bash
   time west twister -T apps/06-sensor -p native_sim
   ```

   *Check:* you have both times written down, and a sentence on what the extra seconds
   bought you.

## ★★★ off the map

1. **Fake a Zephyr API directly and find out why it hurts.** Try to `FAKE_VALUE_FUNC`
   around `sensor_sample_fetch()` instead of around your own port. Read the declaration
   first:

   ```bash
   grep -n -B4 "int sensor_sample_fetch" $ZEPHYR_BASE/include/zephyr/drivers/sensor.h
   ```

   *Why it is interesting:* it is `static inline` over an API struct, so there is no
   symbol to replace. Working out exactly why is the strongest argument there is for
   the port header.

2. **Decide how many ports a module should have.** This service has two. Add a clock
   port so the service can time out, then a logging port, then a persistence port, and
   notice where it stops being worth it.

   *Why it is interesting:* every port is a file, a fake and an indirection, and
   "inject everything" produces code nobody can follow.

3. **Work out what a fake cannot catch.** List five bugs that would ship with this
   suite green. Be specific about each one.

   *Why it is interesting:* it is the list you hand to whoever asks why the
   hardware-in-the-loop suite still exists.

4. **Compare FFF against gmock on your own terms.** Read the C++ file that asserts the
   same fourteen things:

   ```bash
   less ../09-gtest-gmock/tests/gtest/src/test_service.cpp
   ```

   Write down which file you would rather maintain, and which you would rather debug.

   *Why it is interesting:* those may well be different answers.

## If you want to go further

- [FFF README](https://github.com/meekrosoft/fff) - every macro, including `DECLARE_FAKE_*` for splitting fakes across files, and `FFF_ARG_HISTORY_LEN`.
- [`fff.h` in Zephyr](https://github.com/zephyrproject-rtos/zephyr/blob/main/subsys/testsuite/include/zephyr/fff.h) - the vendored copy that is already on your include path.
- [`docs/NOTES-testing.md`](../../docs/NOTES-testing.md) - local notes on which kind of suite belongs where.
