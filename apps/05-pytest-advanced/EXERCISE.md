# 05-pytest-advanced - exercises

## ★ warm-up

1. **Watch a marker do its job.** Run the full scenario, then the smoke one, and
   compare the counts:

   ```bash
   west twister -T apps/ -p native_sim --test app05.shell.pytest.full
   ```

   ```bash
   west twister -T apps/ -p native_sim --test app05.shell.pytest.smoke
   ```

   *Check:* you can say how many tests the marker filter removed, and name them.

2. **Take away the ids.** Remove the `ids=` argument from
   `test_press_counter_matches` in `pytest/test_parametrize.py`, then run just that
   file:

   ```bash
   west twister -T apps/ -p native_sim --test app05.shell.pytest.parametrize
   ```

   *Check:* you can read the names pytest generated instead, and say what a CI log
   would look like with those names in it. Put `ids=` back when you are done.

3. **Make the xfail pass.** Change `cmd_reset` in `test_harness.c` to also turn the
   LED off, then run the suite:

   ```bash
   west twister -T apps/05-pytest-advanced -p native_sim
   ```

   *Check:* the suite goes RED, not green, and you can say why that is the correct
   behaviour for `strict=True`. Put it back when you are done.

4. **Add a negative case.** `test_bad_press_count_is_rejected` has five inputs. Add
   one more that you expect the device to reject.

   *Check:* either it passes, or it fails because the device actually accepts your
   input. The second outcome is the more useful one, so say which you got.

## ★★ go deeper

1. **Add a fixture with real teardown.** Write one that records the press count before
   and after, and asserts the test did not leave the device somewhere surprising. Use
   `yield`.

   *Check:* the teardown still runs when you make the test body fail on purpose.

2. **Parametrize over something structural.** Turn `test_led_state_after_n_presses`
   into a single parametrize over a list of `(presses, led, count)` triples, and
   delete the tests it duplicates.

   *Check:* the same behaviours are covered in fewer lines, and the failure messages
   got no worse. Break one case to confirm the second half.

3. **Use `dut` for something `shell` cannot do.** Assert that the app prints nothing at
   all for ten seconds when no command is sent.

   *Check:* you used `readlines_until` with a timeout, not `time.sleep`.

4. **Add a fifth scenario.** One that runs only `pytest/test_markers.py`, with
   `--strict-markers` in its `pytest_args`. Then misspell a marker on purpose and run
   it.

   *Check:* the run fails at collection rather than at assertion time, and you can
   quote the message. That is the whole reason to turn the flag on.

5. **Compare the two conftests.**

   ```bash
   diff -y --width 160 tests/shell_pytest/pytest/conftest.py tests/pytest_raw/conftest.py | less
   ```

   *Check:* you can list four things twister does that the raw one had to do by hand,
   and one thing twister does that the raw one still does not.

## ★★★ off the map

1. **Make the raw suite work against a real board.** Replace the subprocess in
   `tests/pytest_raw/conftest.py` with `pyserial`, and keep the test bodies identical.

   *Why it is interesting:* the test bodies should not have to change, and whether they
   do is a good measure of how well you drew the line between fixture and assertion.

2. **Add a flaky test and deal with it honestly.** Write one that fails about one run
   in five. Then decide: retry it, mark it, fix it, or delete it. Write down why.

   *Why it is interesting:* every long-running embedded suite arrives here, and
   `pytest-rerunfailures` is the easy answer that is often the wrong one.

3. **Generate a report CI could publish.** `--junitxml`, or `pytest-html`, wired
   through `pytest_args`.

   *Why it is interesting:* finding where twister puts the output, and getting it out
   of `twister-out/` and into a GitHub Actions artifact, is most of the work and none
   of the documentation.

4. **Measure what the shell round trip costs.** Time 100 `app led` calls on
   `native_sim`, and on a board at 115200 baud.

   *Why it is interesting:* it tells you how many assertions per test you can afford,
   which is a design constraint on the backdoor itself.

## If you want to go further

- [pytest how-to guides](https://docs.pytest.org/en/stable/how-to/index.html) - the whole index is worth a skim; `conftest.py`, marks and fixtures are the three that pay off fastest here.
- [Twister harness configuration](https://docs.zephyrproject.org/latest/develop/test/pytest.html#usage) - every key `harness_config` accepts, including `pytest_dut_scope`, which changes how often the device is reset between tests.
- [pytest-rerunfailures](https://github.com/pytest-dev/pytest-rerunfailures) - read it, then read the second exercise under ★★★ again before you install it.
