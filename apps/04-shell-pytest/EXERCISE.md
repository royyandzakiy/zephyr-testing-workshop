# 04-shell-pytest - exercises

Your pytest suite is green, which was the gate for session 4.

The answers are in the upstream docs rather than in this repo.

## ★ warm-up

1. **Talk to the device yourself.** Build the test app by hand and run it:
   `west build -b native_sim/native -p -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -d build_shell && ./build_shell/zephyr/zephyr.exe`
   Then type `test_btn` at the prompt.
   *Check:* you see the same lines the pytest suite asserts on.

2. **Use the other harness.** `testcase.yaml` has a commented-out
   `harness: shell` block. Swap to it and run twister.
   *Check:* it passes, with no Python involved, and you can say what you gave
   up.

3. **Make the assertion fail.** Change one expected `'ON'` to `'OFF'` in the
   pytest file.
   *Check:* you can find both the pytest failure and the raw device output in
   `twister-out/`.

4. **Add a sixth press.** `EXPECTED_STATES` has five entries.
   *Check:* the test still passes and you did not have to touch any C.

## ★★ go deeper

1. **Add a command that reports state instead of changing it.** A `led_get`
   that prints the current LED state. Assert on it after each press.
   *Check:* your test no longer depends on the wording of the app's own
   `printk`. Compare with `apps/05-pytest-advanced/tests/shell_pytest/test_harness.c`.

2. **Use the `dut` fixture.** Rewrite one assertion to read the boot banner,
   which the `shell` fixture cannot see.
   *Check:* you can say why `shell` cannot see it.

3. **Time it.** Run the suite on `native_sim`, then on a board if you have
   one, and write both numbers down.
   *Check:* you have an actual ratio, which is the number that decides what
   goes in a pre-commit hook and what goes in nightly CI.

4. **Break the shell on purpose.** Set `CONFIG_SHELL_VT100_COLORS=y` and run
   the suite.
   *Check:* you can describe exactly how it fails, and why the failure is
   confusing.

## ★★★ off the map

1. **Decide where the backdoor ends.** `test_btn` presses a button. Write down
   three more commands you would add for a real product, then write down the
   one you would refuse to add and why.
   *Why it is interesting:* a shell backdoor is a genuine attack surface and
   "it is only in the test build" is only true if nobody ever ships the test
   build. There is no clean answer, which is the point.

2. **Run the same suite against two boards in one twister invocation** with a
   `hardware-map.yaml` holding both.
   *Why it is interesting:* it is the moment the suite stops being "a test"
   and becomes "a matrix", and you find out which of your assertions were
   quietly about one board.

3. **Drive the device with no twister at all.** Write a plain
   `pytest` + `pyserial` script against the built binary.
   *Why it is interesting:* `apps/05-pytest-advanced/tests/pytest_raw/` is a
   worked version, so you can compare afterwards and see what the harness was
   doing for you.

## If you want to go further

- [pytest fixtures](https://docs.pytest.org/en/stable/how-to/fixtures.html) - scopes, `yield` teardown, and `conftest.py`, all of which app 05 leans on.
- [twister_harness API](https://docs.zephyrproject.org/latest/develop/test/pytest.html) - `DeviceAdapter`, `Shell`, `MCUmgr`, and what each fixture actually wraps.
- [`docs/NOTES-ci-self-hosted.md`](../../docs/NOTES-ci-self-hosted.md) - what changes when this suite runs on a runner with a board attached.
