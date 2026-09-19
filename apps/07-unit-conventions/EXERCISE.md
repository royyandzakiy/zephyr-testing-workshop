# 07-unit-conventions - exercises

## ★ warm-up

1. **Delete the `before` hook.** Remove `edit_before` from the `ZTEST_SUITE` line of
   `feeder_edit`, leaving `NULL` in its place, then run the suite:

   ```bash
   west twister -T apps/07-unit-conventions -p native_sim
   ```

   *Check:* one test fails. You can name it, say which of its two assertions caught
   the problem and which one did not, and explain why ztest running the suite
   alphabetically matters here. Put the hook back when you are done.

2. **Take a message away.** In `test_minutes_until_the_next_feed`, change the
   `zassert_equal(got, cases[i].want, "case %zu (%s): ...")` to a bare
   `zassert_equal(got, cases[i].want)`, then break one row so it fails:

   ```bash
   west build -b native_sim/native -p -s apps/07-unit-conventions/tests/ztest -d build_conv && ./build_conv/zephyr/zephyr.exe
   ```

   *Check:* you have both failure outputs in front of you, and you can say which row
   broke from the second one. Put the message back when you are done.

3. **Rename the fixture struct.** Change `struct feeder_edit_fixture` to
   `struct edit_fixture` everywhere it appears, then build:

   ```bash
   west build -b native_sim/native -p -s apps/07-unit-conventions/tests/ztest -d build_conv
   ```

   *Check:* you can quote the compiler error and name the macro that produced it. Put
   the name back when you are done.

4. **Make the skip fire.** Set `FEEDER_SLOTS_MAX` in `src/feeder.h` to 3 and run the
   binary directly, so you see every line ztest prints:

   ```bash
   west build -b native_sim/native -p -s apps/07-unit-conventions/tests/ztest -d build_conv && ./build_conv/zephyr/zephyr.exe
   ```

   *Check:* you can point at the SKIP line, tell it apart from a PASS, and say why
   `feeder_capacity` ran at all instead of being skipped whole. Put it back to 4 when
   you are done.

## ★★ go deeper

1. **Break the midnight wrap.** In `feeder_next()`, replace the gap calculation with
   the tempting version:

   ```c
   uint16_t gap = f->slots[i] - now_minute;
   ```

   ```bash
   west twister -T apps/07-unit-conventions -p native_sim
   ```

   *Check:* two of the six rows fail and four still pass. You can name which two, and
   say why the number they report is 1440 rather than something obviously wrong. Put
   it back when you are done.

2. **Split the table.** `test_minutes_until_the_next_feed` has six rows. Turn it into
   six `ZTEST`s, then break two rows at once and run both versions.

   *Check:* you can say what the table version told you and what the six-test version
   told you, in one sentence each.

3. **Add a behaviour and its test, in both orders.** Add `feeder_remove(f, at_minute)`.
   Write the implementation first and the test after. Then throw both away and do it
   the other way round.

   *Check:* the two test suites you wrote are not the same. Write down one concrete
   difference.

4. **Sort the slots and find out what breaks.** Make `feeder_add()` keep the table in
   ascending order.

   *Check:* `test_slots_out_of_order_still_find_the_nearest` now tests nothing, because
   the condition it describes cannot happen any more. Decide whether to delete it or
   rewrite it, and say why.

## ★★★ off the map

1. **Write the conventions down for your own team.** One page, ten rules at most, each
   with a reason attached. Then find a test in your own codebase that breaks three of
   them and decide whether to fix it.

   *Why it is interesting:* a rule with no reason attached gets argued away in the
   first code review, and a list of twenty gets ignored entirely.

2. **Work out when a fixture is worse than a local.** `feeder_basic` uses a stack
   struct, `feeder_edit` uses a fixture. Write the version of `feeder_edit` that uses
   locals, and decide which you prefer.

   *Why it is interesting:* the usual advice is "use fixtures", and it is wrong about
   half the time.

3. **Find the other wrapping clock in your own firmware.** `k_uptime_get_32()` rolls
   over every 49.7 days and `k_cycle_get_32()` much sooner. Find one place that
   compares them, and decide whether it survives the rollover.

   *Why it is interesting:* it is the same bug as the one in this module, one order of
   magnitude further out, and no amount of bench testing will show it to you.

## If you want to go further

- [ztest](https://docs.zephyrproject.org/latest/develop/test/ztest.html) - the suite hooks and the full assertion list, including `zassert_mem_equal` and `zassert_within`.
- [Zephyr's own test tree](https://github.com/zephyrproject-rtos/zephyr/tree/main/tests) - thousands of worked examples at very mixed quality. Reading a few and deciding which you would copy is itself the exercise.
- [Twister test cases](https://docs.zephyrproject.org/latest/develop/test/twister.html#test-cases) - `extra_configs`, `filter`, and how one directory can produce several scenarios.
