# 09-gtest-gmock - exercises

## ★ warm-up

1. **Link the wrong main.** Change `GTest::gmock_main` to `GTest::gtest_main` in
   `tests/CMakeLists.txt`, then rebuild and run. This is the loop every exercise
   below uses, once `cmake -S ... -B ...` has been run once:

   ```bash
   cmake --build apps/09-gtest-gmock/build -j && ctest --test-dir apps/09-gtest-gmock/build --output-on-failure
   ```

   *Check:* the suite still builds and still passes. Name which expectations stopped
   being checked, and say what a passing run now tells you about the mocks. Put it
   back when you are done.

2. **Delete a `PrintTo`.** Remove the `PrintTo(const AlarmCase&, ...)` function from
   `tests/test_logic.cpp`, rebuild, then list the tests instead of running them:

   ```bash
   ctest --test-dir apps/09-gtest-gmock/build -N | grep Hysteresis
   ```

   *Check:* you can see the hex dump where the case names used to be, and say which
   part of CMake's `GoogleTest.cmake` picked that string up. Put it back when you are
   done.

3. **Turn a NiceMock into a plain mock.** Pick one in `tests/test_service.cpp` and drop
   the wrapper, then watch what the run prints:

   ```bash
   ctest --test-dir apps/09-gtest-gmock/build -R ServiceMock --output-on-failure 2>&1 | grep -c "GMOCK WARNING"
   ```

   *Check:* you have the count, and a decision about whether you would want those
   warnings on in CI. Put the wrapper back when you are done.

4. **Make a strict mock fail.** In `QuietReadingsNeverTouchTheAlarm`, change the
   reading from `quiet()` to `hot()`.

   *Check:* the failure names the unexpected call and its arguments, and it fires at
   the call rather than at the end of the test. Put it back when you are done.

5. **Break the edge detection.** Same change as in app 08: call `alarm_port_.set(next)`
   unconditionally in `src/service.cpp`. Then do the same to app 08 and compare:

   ```bash
   west twister -T apps/08-fff-mocks -p native_sim
   ```

   *Check:* you have both failure messages side by side and can say which told you more
   about what went wrong. Put both back when you are done.

## ★★ go deeper

1. **Add a case to the truth table.** One that is currently untested, then confirm the
   name generator produced something readable for it:

   ```bash
   ctest --test-dir apps/09-gtest-gmock/build -R "your case name"
   ```

   *Check:* that command runs exactly one test.

2. **Use a custom matcher.** Write one with `MATCHER_P` that means "a Reading whose
   temperature is above the trip point", and use it in an `EXPECT_CALL`.

   *Check:* make it fail on purpose. The message names the temperature and the
   threshold it was compared against.

3. **Turn on the sanitizers and find something.** Reconfigure, then introduce a
   deliberate use-after-free in a test fixture:

   ```bash
   cmake -S apps/09-gtest-gmock -B apps/09-gtest-gmock/build -DCLIMATE_SANITIZERS=ON && cmake --build apps/09-gtest-gmock/build -j
   ```

   *Check:* ASan reports it with a stack trace. Say how that compares to getting the
   same information off a Cortex-M.

4. **Measure coverage.** Add `--coverage` to the compile and link options in
   `tests/CMakeLists.txt`, rebuild, run, then:

   ```bash
   gcovr --root apps/09-gtest-gmock --html-details apps/09-gtest-gmock/build/cov.html
   ```

   *Check:* you can say what percentage of `service.cpp` is covered, and separately,
   whether any of the uncovered lines matter.

5. **Make the C and C++ services share a test vector file.** Put the hysteresis truth
   table in one place, read by both `apps/08-fff-mocks/tests/fff/src/main.c` and
   `tests/test_logic.cpp`.

   *Check:* adding one row to that file changes both suites.

## ★★★ off the map

1. **Get this to run on a Cortex-M.** GoogleTest needs a heap, a decision about
   exceptions and RTTI, and a `main`. Find out how far you get and where you stop.

   *Why it is interesting:* the honest answer for most teams is that you do not, and
   knowing exactly why is what lets you argue for the split this repo uses: host tests
   for logic, ztest and emulators on target.

2. **Decide whether the vtable is affordable.** Measure the cost of `ISensorPort`
   against a compile-time policy template on a real target. Then write the templated
   version of `Service` and test it.

   *Why it is interesting:* the templated version is faster and harder to mock, and
   which one is right depends on numbers you do not have until you measure.

3. **Argue the other side.** This app is fast, ergonomic, and proves nothing about
   hardware. Write the paragraph you would put in a design doc arguing the project
   should adopt it anyway, then the paragraph arguing it should not.

   *Why it is interesting:* both paragraphs are true, and the decision is a judgement
   about where your bugs actually come from.

4. **Pin GoogleTest without FetchContent.** Vendor it as a git submodule, or via CPM,
   or from your distribution's packages, and make the build work with the network off:

   ```bash
   cmake -S apps/09-gtest-gmock -B build_offline -DGTEST_LOCAL_DIR=/path/to/googletest
   ```

   *Why it is interesting:* it is the first thing that breaks on an air-gapped runner,
   and every option has a different failure mode.

## If you want to go further

- [GoogleTest advanced guide](https://google.github.io/googletest/advanced.html) - `TEST_P`, typed tests, `SCOPED_TRACE`, and how to write good failure messages.
- [gMock cookbook](https://google.github.io/googletest/gmock_cook_book.html) - the long one. `NiceMock`, `StrictMock`, `DoAll` and the full matcher list are in here.
- [gtest_discover_tests](https://cmake.org/cmake/help/latest/module/GoogleTest.html) - every option, including `DISCOVERY_MODE PRE_TEST` for cross-compiled builds where you cannot run the binary at build time.
