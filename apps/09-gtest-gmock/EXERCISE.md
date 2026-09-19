# 09-gtest-gmock - exercises

## ★ warm-up

1. **Make a mock fail without writing an assertion.** In
   `tests/gtest/src/test_dispenser.cpp`, change the expectation in
   `Dispenser.SplitsAPortionIntoWholeTurns` from `.Times(3)` to `.Times(2)` and run it:

   ```bash
   west twister -T apps/09-gtest-gmock -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* the test fails, the message names the actual and expected call counts, and
   there is no `EXPECT_EQ` anywhere in that test checking the count. Say where the
   failure came from. Put the 3 back when you are done.

2. **Change the argument the mock is matching on.** In the same test, change
   `run(feeder::kGramsPerTurn)` to `run(100)`.

   ```bash
   west twister -T apps/09-gtest-gmock -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* the output has four complaints in it, not one. gmock reports
   `Unexpected mock function call` once per actual call, printing `run(250)` and the
   expectation it tried, and then once more at the end for the expectation that was
   never satisfied. Put `feeder::kGramsPerTurn` back.

3. **Break a test that never runs.** In `tests/gtest/src/test_portion.cpp`, change

   ```c
   static_assert(feeder::turnsFor(251) == 2);
   ```

   to `== 1` and build.

   ```bash
   west build -b native_sim/native -p -d build_gt apps/09-gtest-gmock/tests/gtest
   ```

   *Check:* you never reach a test run, and you can say which tool reported the failure
   and at what point in the build. Put the 2 back when you are done.

4. **Add a row to the table.** Add `{500, 2, "AMediumPortion"}` to `kCases` in
   `test_portion.cpp` and run the suite:

   ```bash
   west twister -T apps/09-gtest-gmock -p native_sim -O /tmp/tw --clobber-output -v
   ```

   *Check:* the new row appears as its own `[ OK ]` line with the name you gave it, and
   the total went from 8 to 9. Compare that against how
   `apps/07-unit-conventions/tests/ztest/src/main.c` reports its table. You can leave
   the row in, it is a correct case.

## ★★ go deeper

1. **Use a matcher instead of a literal.** `run(250)` is a matcher that happens to be an
   exact value. Replace it with one that accepts a range:

   ```c
   using testing::AllOf;
   using testing::Ge;
   using testing::Le;
   EXPECT_CALL(auger, run(AllOf(Ge(200), Le(300)))).Times(3).WillRepeatedly(Return(0));
   ```

   *Check:* the test still passes, and changing `kGramsPerTurn` to 275 keeps it passing
   while the original version would have failed. Say when you would want each of the
   two.

2. **Assert on ordering.** Add a second mock to the dispenser, for example an
   `IHopper` with `virtual std::uint16_t level() = 0;`, make `Dispenser::feed()` check
   the hopper before the first turn, then pin the order:

   ```c
   testing::InSequence seq;
   ```

   *Check:* with `InSequence` in scope, swapping the two calls in `dispenser.cpp` fails
   the test. Without it, both orders pass. FFF has no equivalent that is anywhere near
   this short.

3. **Find out what GoogleTest costs.** Build the test image, then read the reports out
   of that same directory:

   ```bash
   west build -b native_sim/native -p -d build_gt apps/09-gtest-gmock/tests/gtest
   ```

   ```bash
   west build -d build_gt -t ram_report
   ```

   Then do the same for `apps/08-fff-mocks/tests/fff` into a directory of its own.

   *Check:* you can give both numbers and name the three `CONFIG_` symbols in
   `tests/gtest/prj.conf` that account for most of the difference.

## ★★★ off the map

1. **Get one result per test out of twister.** `harness: console` gives one pass or
   fail for the whole binary, so a suite of eight shows up as one. GoogleTest can emit
   JUnit XML with `--gtest_output=xml:`, and twister writes its own `twister.xml`.

   *Why it is interesting:* there is no supported way to merge them, so you have to
   decide whether per-test reporting is worth a post-processing step in CI, or whether
   one result per binary is enough. Both answers are defensible and the reasoning is
   the useful part.

2. **Try to run this on a real board.** Pick one you own and build the test directory
   for it:

   ```bash
   west build -b nrf5340dk/nrf5340/cpuapp -p -d build_board apps/09-gtest-gmock/tests/gtest
   ```

   *Why it is interesting:* it will not work as it stands, and the first error is not
   the real problem. Work out how far down the table in the README's Trivia section you
   get before you stop, and what you would have to give up to go further.

3. **Decide which framework you would actually ship.** You now have the same three
   behaviours tested twice, in `apps/08-fff-mocks` and here.

   *Why it is interesting:* write down which one you would put in a product and what
   would change your mind. The answer depends on the target's RAM, on whether the team
   already writes C++, and on how much the self-verifying expectation is worth to you.

## If you want to go further

- [gMock cookbook](https://google.github.io/googletest/gmock_cook_book.html) - matchers,
  actions, `NiceMock` and `StrictMock`, and mocking free functions.
- [gMock cheat sheet](https://google.github.io/googletest/gmock_cheat_sheet.html) - the
  one page to have open while writing expectations.
- [Zephyr C++ support](https://docs.zephyrproject.org/latest/develop/languages/cpp/index.html) -
  what each `CONFIG_*_LIBCPP` choice gives you, and which standard library headers exist
  under each one.
