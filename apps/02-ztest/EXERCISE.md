# 02-ztest - exercises

## ★ warm-up

1. **Make a test fail on purpose.** Change one expected `"ON"` to `"OFF"` in
   `tests/unit/src/main.c` and run twister.

   *Check:* you can name the file under `twister-out/` that holds the failing
   assertion, and quote the line from it. Put the test back when you are done.

2. **Add a fourth test.** Assert that toggling twice returns to where you started.

   *Check:* the scenario still reports as one configuration, and now runs four tests
   instead of three.

3. **Tighten the scope.** Run only this scenario:

   ```bash
   west twister -T apps/ -p native_sim --test app02.blink.logic
   ```

   *Check:* twister reports 1 scenario selected, not 1 selected out of many filtered.

4. **Link the wrong thing.** Add `blinky.c` to `tests/unit/CMakeLists.txt` and
   rebuild.

   *Check:* you can explain the error in terms of the devicetree, and say what
   `tests/unit/prj.conf` would need before it would link. Take it back out when you
   are done.

## ★★ go deeper

1. **Use the tags.** Add `- fast` to the `tags:` list in the scenario, then:

   ```bash
   west twister -T apps/ -p native_sim --tag fast
   ```

   *Check:* only this scenario runs, and you can say how `--tag` differs from
   `--test`.

2. **Split the five-press test.** It currently loops and stops at the first failure.
   Make each press its own `ZTEST`, then break two presses at once and compare the
   output of both versions.

   *Check:* you can say what you gained and what you lost, in one sentence each.

3. **Watch the hooks fire.** Give the suite a `before` function that just prints. Run
   the suite and see where the line lands relative to each test.

   *Check:* you can write out the order of `setup`, `before`, test, `after`,
   `teardown` from what you saw, without opening the docs.

4. **Measure coverage.**

   ```bash
   west twister -T apps/02-ztest -p native_sim --coverage --coverage-tool gcovr
   ```

   *Check:* you can find the HTML report, say what percentage of `blink_logic.c` is
   covered, and say whether that number means anything here.

## ★★★ off the map

1. **Argue with the seam.** `blink_logic_toggle()` is `return !led_on;`. Write down
   an honest answer to "is a test for this worth the two files it cost?" Then write
   down what would have to be true of the function for your answer to flip.

   *Why it is interesting:* this is the question you get asked when you take the
   pattern back to your own codebase, and "test everything" does not survive contact
   with a review.

2. **Run the suite on a real board.** Add a board to `platform_allow` and run twister
   with `--device-testing`. The logic has no dependencies, so it should pass
   unchanged.

   *Why it is interesting:* what you learn is how much slower it is, and that number
   is what decides where each of your suites should run.

3. **Find out what `ZTEST_USER` is for**, and whether this suite could use it.

   *Why it is interesting:* it leads into userspace and memory domains, which is a
   much bigger topic than it looks from the macro name.

## If you want to go further

- [ztest API reference](https://docs.zephyrproject.org/latest/develop/test/ztest.html#api-reference) - every `zassert_*` and `zassume_*`, including the ones that print better failures than `zassert_true`.
- [Twister test configuration](https://docs.zephyrproject.org/latest/develop/test/twister.html#test-cases) - `filter`, `extra_configs`, `integration_platforms`, and the difference between `platform_allow` and `platform_exclude`.
- [Code coverage](https://docs.zephyrproject.org/latest/develop/test/coverage.html) - how Zephyr wires gcov into a native_sim build.
