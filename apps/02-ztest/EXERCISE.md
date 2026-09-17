# 02-ztest - exercises

Your first suite is green and there is time left.

The answers are in the upstream Zephyr docs rather than in this repo.

## ★ warm-up

1. **Make a test fail on purpose.** Change one expected `"ON"` to `"OFF"` in
   `tests/unit/src/main.c` and run twister.
   *Check:* you can find the failing assertion in `twister-out/` without
   scrolling through the console, and you know which file to open.

2. **Add a fourth test.** Assert that toggling twice returns to where you
   started.
   *Check:* the scenario still reports as one configuration and now runs four
   tests.

3. **Tighten the scope.** Run only this scenario:
   `west twister -T apps/ -p native_sim --test app02.blink.logic`
   *Check:* twister reports 1 selected, not 1 of many filtered.

4. **Link the wrong thing.** Add `blinky.c` to `tests/unit/CMakeLists.txt` and
   rebuild.
   *Check:* you can explain the error in terms of devicetree, and you can say
   what `prj.conf` would need before it would link.

## ★★ go deeper

1. **Use the tags.** Add `tags: - fast` to the scenario and run
   `west twister -T apps/ -p native_sim --tag fast`.
   *Check:* only this scenario runs, and you can say how that differs from
   `--test`.

2. **Split the five-press test.** It currently loops and stops at the first
   failure. Make each press its own `ZTEST` and compare the output of a
   deliberate failure in both versions.
   *Check:* you can state the trade you just made, in one sentence, both ways.

3. **Add a `before` hook that does nothing useful.** Give the suite a
   `before` function that prints. Run the suite and watch where the line
   lands relative to each test.
   *Check:* you can describe the order of `setup`, `before`, test, `after`,
   `teardown` from what you saw rather than from the docs.

4. **Measure coverage.**
   `west twister -T apps/02-ztest -p native_sim --coverage --coverage-tool gcovr`
   *Check:* you can find the HTML report and say what percentage of
   `blink_logic.c` is covered, and whether that number means anything.

## ★★★ off the map

1. **Argue with the seam.** `blink_logic_toggle()` is `return !led_on;`. Write
   down an honest answer to "is a test for this worth the two files it cost?"
   Then write down what would have to be true of the function for the answer
   to flip.
   *Why it is interesting:* this is the question you will actually be asked
   when you take this back to your own codebase, and "test everything" is not
   a usable answer.

2. **Run the suite on a real board.** Add a board to `platform_allow` and run
   twister with `--device-testing`. The logic has no dependencies, so it
   should pass unchanged.
   *Why it is interesting:* what you learn is how much slower it is, and that
   is the number that decides where each of your suites should run.

3. **Find out what `ZTEST_USER` is for** and whether this suite could use it.
   *Why it is interesting:* it leads into userspace and memory domains, which
   is a much bigger topic than it looks from the macro name.

## If you want to go further

- [ztest API reference](https://docs.zephyrproject.org/latest/develop/test/ztest.html#api-reference) - every `zassert_*` and `zassume_*`, including the ones that print better failures than `zassert_true`.
- [Twister test configuration](https://docs.zephyrproject.org/latest/develop/test/twister.html#test-cases) - `filter`, `extra_configs`, `integration_platforms`, and the difference between `platform_allow` and `platform_exclude`.
- [Code coverage](https://docs.zephyrproject.org/latest/develop/test/coverage.html) - how Zephyr wires gcov into a native_sim build.
