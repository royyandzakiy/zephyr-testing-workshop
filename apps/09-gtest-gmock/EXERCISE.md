# 09-gtest-gmock - exercises

## ★ warm-up

1. **Initialise the wrong framework.** `tests/gtest/src/main.cpp` calls
   `testing::InitGoogleMock`. Change it to `testing::InitGoogleTest` and rebuild:

   ```bash
   west build -b native_sim/native -p -s apps/09-gtest-gmock/tests/gtest -d build_gt && ./build_gt/zephyr/zephyr.exe
   ```

   *Check:* the suite still builds and still passes. Name which expectations stopped
   being checked, and say what a passing run now tells you about the mocks. Put it back
   when you are done.

2. **Delete a `PrintTo`.** Remove the `PrintTo(const AlarmCase&, ...)` function from
   `tests/gtest/src/test_logic.cpp`, rebuild, then list the tests instead of running
   them:

   ```bash
   ./build_gt/zephyr/zephyr.exe --gtest_list_tests | grep -A3 Hysteresis
   ```

   *Check:* you can see the hex dump where the case parameters used to print, and say
   which GoogleTest function produced it. Put it back when you are done.

3. **Break a compile-time test.** Change one of the `static_assert` lines at the top of
   `test_logic.cpp` so it is wrong, then build.

   ```bash
   west build -b native_sim/native -p -s apps/09-gtest-gmock/tests/gtest -d build_gt
   ```

   *Check:* the build fails and no test runs at all. Say what that gives you over a
   failing `EXPECT_EQ`, and what it takes away. Put it back when you are done.

4. **Make a strict mock fail.** In `QuietReadingsNeverTouchTheAlarm`, change the
   reading from `quiet()` to `hot()` and run.

   *Check:* the failure names the unexpected call and its arguments, and it fires at
   the call rather than at the end of the test. Put it back when you are done.

5. **Break the edge detection.** Call `alarm_port_.set(next)` unconditionally in
   `src/service.cpp`. Then make the same change in app 08 and run both:

   ```bash
   west twister -T apps/09-gtest-gmock -T apps/08-fff-mocks -p native_sim
   ```

   *Check:* you have both failure messages side by side and can say which told you more
   about what went wrong. Put both back when you are done.

## ★★ go deeper

1. **Watch the console harness do its job.** Change the regex in
   `tests/gtest/testcase.yaml` to something that will never match, then run the suite.

   ```bash
   west twister -T apps/09-gtest-gmock -p native_sim
   ```

   *Check:* twister reports a failure even though every GoogleTest assertion passed,
   and you can find the device output under `twister-out/` that proves it. Put the
   regex back when you are done.

2. **Find out what each Kconfig line costs.** Comment out `CONFIG_CPP_RTTI=y` in
   `tests/gtest/prj.conf`, build, and read the error. Then do the same for
   `CONFIG_CPP_EXCEPTIONS=y` and `CONFIG_REQUIRES_FULL_LIBCPP=y`.

   ```bash
   west build -b native_sim/native -p -s apps/09-gtest-gmock/tests/gtest -d build_gt
   ```

   *Check:* you can say which of the three fails at compile time and which at link
   time, and quote one symbol name from each failure.

3. **Swap in the symbol that looks right.** Replace `CONFIG_REQUIRES_FULL_LIBCPP=y`
   with `CONFIG_GLIBCXX_LIBCPP=y` and build. It fails on `<concepts>`. Now find out
   what the build actually decided:

   ```bash
   grep LIBCPP build_gt/zephyr/.config
   ```

   *Check:* `CONFIG_GLIBCXX_LIBCPP` is not in `.config` at all, and you can name the
   two `depends on` lines upstream that caused it to be dropped. Put it back when you
   are done.

4. **Measure what GoogleTest weighs.** Compare this image against the ztest suite in
   app 07:

   ```bash
   west build -b native_sim/native -p -s apps/09-gtest-gmock/tests/gtest -d build_gt -t rom_report
   ```

   ```bash
   west build -b native_sim/native -p -s apps/07-unit-conventions/tests/ztest -d build_zt -t rom_report
   ```

   *Check:* you have both numbers written down, and a view on whether the difference
   would fit on a target you actually ship.

5. **Add a case to the truth table.** One that is currently untested, then run just
   that case:

   ```bash
   ./build_gt/zephyr/zephyr.exe --gtest_filter='*your_case_name*'
   ```

   *Check:* exactly one test runs, and the name generator gave it something readable.

6. **Use a custom matcher.** Write one with `MATCHER_P` that means "a Reading whose
   temperature is above the trip point", and use it in an `EXPECT_CALL`.

   *Check:* make it fail on purpose. The message names the temperature and the
   threshold it was compared against.

## ★★★ off the map

1. **Port the sensor port to `std::expected`.** C++23 has it, and
   `int read(Reading& out)` is exactly the signature it was designed to replace.
   Change `ISensorPort::read()` to return `std::expected<Reading, int>` and fix the
   mocks.

   *Why it is interesting:* the tests get shorter, `DoAll(SetArgReferee<0>(...))`
   disappears, and you find out whether your toolchain's libstdc++ is new enough for
   `<expected>` at all. That last part is the real lesson about C++23 on embedded
   targets.

2. **Get this onto a Cortex-M.** Add a board to `platform_allow` and start working
   through what breaks. The Kconfig table in the README is your list of suspects.

   *Why it is interesting:* the honest answer for most teams is that you do not, and
   knowing exactly which of exceptions, RTTI and the standard library stopped you is
   what lets you argue for the split this repo uses.

3. **Decide whether the vtable is affordable.** Measure the cost of `ISensorPort`
   against a compile-time policy template. The concepts in `ports.hpp` are already
   there, so the templated `Service` can be constrained without a base class at all.

   *Why it is interesting:* the templated version is faster and harder to mock, and
   which one is right depends on numbers you do not have until you measure.

4. **Make more of the suite compile-time.** Eight assertions in `test_logic.cpp` are
   already `static_assert`. Work out how much of `Service` could be `constexpr` too,
   and what stops the rest.

   *Why it is interesting:* it pushes you into what a `constexpr`-friendly design
   looks like, and into the point where making something testable at compile time
   makes it worse to read.

## If you want to go further

- [GoogleTest advanced guide](https://google.github.io/googletest/advanced.html) - `TEST_P`, typed tests, `SCOPED_TRACE`, and how to write good failure messages.
- [gMock cookbook](https://google.github.io/googletest/gmock_cook_book.html) - the long one. `NiceMock`, `StrictMock`, `DoAll` and the full matcher list are in here.
- [Zephyr C++ support](https://docs.zephyrproject.org/latest/develop/languages/cpp/index.html) - every `CONFIG_CPP_*` symbol, and which parts of the standard library each library option gives you.
- [Twister harnesses](https://docs.zephyrproject.org/latest/develop/test/twister.html#harnesses) - `console`, `pytest`, `shell`, `ztest`, and what each one reads to decide pass or fail.
