# 08-fff-mocks - exercises

## ★ warm-up

1. **Read what the fake recorded.** Add a print at the end of
   `test_a_jam_is_retried_and_the_feed_still_counts` in `tests/fff/src/main.c`:

   ```c
   printk("calls=%d arg0[0]=%u arg0[1]=%u\n", auger_run_fake.call_count,
          auger_run_fake.arg0_history[0], auger_run_fake.arg0_history[1]);
   ```

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* both history entries are 250, and you can say why they are the same value
   even though the first call returned `-EIO`. Take the print back out afterwards.

2. **Break the retry on purpose.** In `src/dispenser.h`, change
   `DISPENSER_ATTEMPTS` from 2 to 1 and run the suite again.

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim -O /tmp/tw --clobber-output
   ```

   *Check:* exactly one test fails, and its name tells you what broke without opening
   the file. Put the 2 back when you are done.

3. **Stop the `before` hook from resetting anything.** In `tests/fff/src/main.c`,
   delete everything in the body of `fff_before` except the last line, so it becomes:

   ```c
   static void fff_before(void *f)
   {
   	ARG_UNUSED(f);
   	auger_run_fake.return_val = 0;
   }
   ```

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim -O /tmp/tw --clobber-output
   ```

   Three of the four tests fail. Read the PASS and FAIL lines in the output rather than
   the source file: ztest runs tests inside a suite alphabetically, not in the order
   they appear.

   *Check:* you can say why the one surviving test survived, and name the two pieces of
   state that leaked between the others. Put the body back when you are done.

   Leaving the hook registered but empty is deliberate. Removing it from the
   `ZTEST_SUITE` line instead leaves `fff_before` defined and unused, and the build
   fails with `-Werror=unused-function` before any test runs.

4. **Ask for a portion the app never asks for.** Add a test that feeds 1 gram and
   predict `call_count` and `d.dispensed_g` before you run it.

   *Check:* your prediction matched. If it did not, the interesting question is whether
   `dispenser_feed()` is wrong or your expectation was.

## ★★ go deeper

1. **Add a second dependency and fake it too.** Give the dispenser a hopper level to
   check: declare `uint16_t hopper_grams(void);` in a new `src/hopper_port.h`, have
   `dispenser_feed()` refuse with `-ENOSPC` when the hopper holds less than the
   portion, and write the shipping implementation in `src/hopper_port_sim.c`.

   In the test, add it to the fakes and to the reset list:

   ```c
   FAKE_VALUE_FUNC(uint16_t, hopper_grams);
   ```

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* twister passes with `src/hopper_port_sim.c` absent from
   `tests/fff/CMakeLists.txt`, and you have a test that proves the motor is never told
   to run on an empty hopper.

2. **Use a `custom_fake` for an out-parameter.** `return_val` covers a return code and
   nothing else. Change `auger_run()` to `int auger_run(uint16_t grams, uint16_t *actual)`
   so the motor can report it moved less than asked, then make `dispenser_feed()` count
   `*actual` rather than `grams`.

   A fake with a body is the only way to fill that pointer:

   ```c
   static int short_turn(uint16_t grams, uint16_t *actual) { *actual = grams / 2; return 0; }
   ```

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* `auger_run_fake.custom_fake = short_turn;` in one test makes
   `d.dispensed_g` half the portion, and the other tests still pass unchanged.

3. **Compare the two frameworks on the same code.** Build the C++ version and read its
   test file next to this one:

   ```bash
   west twister -T apps/09-gtest-gmock -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* you can point at the line in `apps/09-gtest-gmock/tests/gtest/src/test_dispenser.cpp`
   that fails the test when the auger is called the wrong number of times, and say why
   there is no equivalent single line in the FFF suite.

## ★★★ off the map

1. **Move the seam and see what it costs.** Rewrite `dispenser.c` so it calls
   `gpio_pin_set_dt()` directly instead of `auger_run()`, then try to keep the four
   tests passing.

   *Why it is interesting:* you end up either faking a Zephyr API you do not own, or
   building a `gpio_emul` overlay the way `apps/03-emul-gpio` does. Both work. Working
   out which one you would actually ship, and at what point in a project you would
   decide, is the question worth sitting with.

2. **Decide what these tests are worth.** Delete `src/auger_port_sim.c` entirely and run
   the suite.

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim -O /tmp/tw --clobber-output
   ```

   *Why it is interesting:* it still passes, because the test build never linked that
   file. Write down, in one sentence, what would have to break for this suite to notice,
   and what other test you would need to cover the rest. `apps/06-sensor` is one answer.
   Restore the file when you are done, the app build needs it.

## If you want to go further

- [FFF README](https://github.com/meekrosoft/fff) - the full macro list, including
  `FAKE_VOID_FUNC`, argument history for more than one parameter, and the value/void
  variants for variadic functions.
- [`$ZEPHYR_BASE/include/zephyr/fff.h`](https://github.com/zephyrproject-rtos/zephyr/blob/main/include/zephyr/fff.h) -
  the copy your build is using. The macro expansion is worth reading once.
- [Test doubles, Martin Fowler](https://martinfowler.com/bliki/TestDouble.html) - stub,
  fake, spy and mock, and where FFF sits among them.
