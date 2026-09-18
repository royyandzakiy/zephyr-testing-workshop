# 04-shell-pytest - exercises

## ★ warm-up

1. **Talk to the device yourself.** Build the test image by hand and run it:

   ```bash
   west build -b native_sim/native -p -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -d build_shell && ./build_shell/zephyr/zephyr.exe
   ```

   Type `test_btn` at the prompt.

   *Check:* you see the same two lines the pytest suite asserts on, and you can point
   at which of them came from `main.c` and which from `test_harness.c`.

2. **Use the other harness.** `testcase.yaml` has a commented-out `harness: shell`
   block. Swap to it and run twister.

   *Check:* it passes with no Python involved, and you can name one assertion the
   pytest suite makes that the shell harness cannot. Swap back when you are done.

3. **Make the assertion fail.** Change one expected `'ON'` to `'OFF'` in the pytest
   file.

   *Check:* you can find both the pytest failure and the raw device output under
   `twister-out/`, and name the file that holds each. Put it back when you are done.

4. **Add a sixth press.** `EXPECTED_STATES` has five entries.

   *Check:* the test still passes, and `git status` shows you changed no C at all.

## ★★ go deeper

1. **Add a command that reports state instead of changing it.** A `led_get` that
   prints the current LED state. Assert on it after each press.

   *Check:* your test no longer depends on the wording of the app's own `printk`.
   Compare against
   `apps/05-pytest-advanced/tests/shell_pytest/test_harness.c` afterwards.

2. **Use the `dut` fixture.** Rewrite one assertion to read the boot banner, which
   the `shell` fixture cannot see.

   *Check:* it passes, and you can say in one sentence why `shell` cannot see that
   line.

3. **Time it.** Run the suite on `native_sim`, then on a board if you have one, and
   write both numbers down.

   *Check:* you have an actual ratio. That number is what decides which suites go in
   a pre-commit hook and which go in nightly CI.

4. **Break the shell on purpose.** Set `CONFIG_SHELL_VT100_COLORS=y` and run the
   suite.

   *Check:* you can describe what the assertion saw instead of what it expected, and
   say why the failure message does not point at the real cause. Put it back when you
   are done.

## ★★★ off the map

1. **Decide where the backdoor ends.** `test_btn` presses a button. Write down three
   more commands you would add for a real product, then one you would refuse to add,
   and why.

   *Why it is interesting:* a shell backdoor is a real attack surface, and "it is
   only in the test build" holds only as long as nobody ever ships the test build.
   There is no clean answer here.

2. **Run the same suite against two boards in one twister invocation**, with a
   `hardware-map.yaml` holding both.

   *Why it is interesting:* this is where a suite turns into a matrix, and you find
   out which of your assertions were quietly about one board.

3. **Drive the device with no twister at all.** Write a plain `pytest` plus
   `pyserial` script against the built binary.

   *Why it is interesting:* `apps/05-pytest-advanced/tests/pytest_raw/` is a worked
   version, so you can compare afterwards and see what the harness was doing for you.

## If you want to go further

- [pytest fixtures](https://docs.pytest.org/en/stable/how-to/fixtures.html) - scopes, `yield` teardown, and `conftest.py`, all of which app 05 leans on.
- [twister_harness API](https://docs.zephyrproject.org/latest/develop/test/pytest.html) - `DeviceAdapter`, `Shell`, `MCUmgr`, and what each fixture actually wraps.
- [`docs/NOTES-ci-self-hosted.md`](../../docs/NOTES-ci-self-hosted.md) - what changes when this suite runs on a runner with a board attached.
