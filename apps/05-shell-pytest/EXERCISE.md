# 05-shell-pytest - exercises

## ★ warm-up

1. **Run only the slow test.** The `slow` marker is registered in
   `tests/emul_button_toggle/pytest/pytest.ini`. Pass the selection through Twister:

   ```bash
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim --pytest-args="-m slow"
   ```

   Then read the pytest summary:

   ```bash
   tail -n 3 $(find twister-out -name twister_harness.log | head -1)
   ```

   *Check:* the summary reads `1 passed, 2 deselected`, and you can name the test that
   ran.

2. **Read what the device said.** After any run, open the device log:

   ```bash
   cat $(find twister-out -name handler.log | head -1)
   ```

   *Check:* you can find the line `test_btn` prints and the line the app's button
   callback prints, and say which file each one comes from: `test_harness.c` or
   `src/main.cpp`.

3. **Press the button by hand.** Build the test image and run it yourself:

   ```bash
   west build -b native_sim/native -p -s apps/05-shell-pytest/tests/emul_button_toggle -d build_shell
   ```

   ```bash
   ./build_shell/zephyr/zephyr.exe
   ```

   Type `test_btn` a few times, then `Ctrl-C` to quit.

   *Check:* the LED state alternates between ON and OFF, and `help` lists `test_btn`
   next to the shell's built-in commands.

## ★★ go deeper

1. **Find the firmware bug from the pytest output.** `exercises/ex3-broken-e2e/` runs the
   same tests against its own `src/main.cpp`, which has one bug. Run it:

   ```bash
   west twister -T apps/05-shell-pytest/exercises/ex3-broken-e2e -p native_sim -O /tmp/ex3 --clobber-output
   ```

   Find the log:

   ```bash
   find /tmp/ex3 -name twister_harness.log
   ```

   `handler.log` is in the same folder. The file to edit is
   `exercises/ex3-broken-e2e/src/main.cpp`.

   *Check:* as shipped, Twister reports `3/3 pytest scenario(s) failed`, each with
   `Did not find line "LED is now ON" within 2 seconds`. You are done when the same
   command reports `3 of 3 executed test cases passed`. Stuck? Compare with
   `exercises/ex3-broken-e2e/solution/main.cpp`. Put the line back when you are done.

2. **Add a test that presses three times.** Add a test to `test_gpio_toggle.py` that
   presses `test_btn` three times through the `shell` fixture and expects `ON`, `OFF`,
   `ON`. Run the suite:

   ```bash
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim
   ```

   *Check:* the summary in `twister_harness.log` reads `4 passed`. Then turn your test
   into one `@pytest.mark.parametrize` over the number of presses, and check that each
   case shows up as its own result.

3. **See why the wait is conditional.** In `test_button_toggle`, remove the `if` line so
   `readlines_until` always runs after `exec_command`, then run on native_sim:

   ```bash
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim
   ```

   *Check:* `test_button_toggle` fails with `Did not find line "LED is now ON" within 2
   seconds`, and `handler.log` shows that line came out before the prompt. Put the line
   back when you are done.

4. **Run the suite on a board.** Fill in `hardware-map.yaml` with your probe serial and
   port (`west twister --generate-hardware-map map.yaml` lists them), then:

   ```bash
   west twister -T apps/05-shell-pytest/tests/emul_button_toggle --device-testing --flash-before --hardware-map apps/05-shell-pytest/hardware-map.yaml
   ```

   *Check:* the same 3 tests pass, LED1 on the DK blinks on every press, and
   `handler.log` shows the `Button pressed!` line arriving after the prompt.

## ★★★ off the map

1. **Replace pytest with the shell harness.** `testcase.yaml` has a commented-out
   `harness: shell` block. Add it as a second scenario next to `app05.shell.pytest`.

   *Why it is interesting:* it runs with no Python at all, and finding out which of the
   three pytest tests you can express with it, and which you cannot, shows what pytest is
   actually adding.

2. **Run the same suite on the ESP32-S3, with and without `--flash-before`.** Use the
   commented ESP32-S3 block in `hardware-map.yaml`.

   *Why it is interesting:* the board shares one USB port for flashing and the console,
   and the run without `--flash-before` leaves the chip in its ROM download mode, which
   `handler.log` shows as `boot:0x0 (DOWNLOAD(USB/UART0))`.

## If you want to go further

- [Twister pytest harness](https://docs.zephyrproject.org/latest/develop/test/pytest.html):
  `pytest_root`, `pytest_args`, `pytest_dut_scope`, and the fixtures `twister_harness`
  provides.
- [`docs/reference/pytest-harness.md`](../../docs/reference/pytest-harness.md): the
  `dut` and `shell` fixtures and `harness_config`, as used in this repo.
- `$ZEPHYR_BASE/scripts/pylib/pytest-twister-harness/src/twister_harness/`: the plugin
  itself. `fixtures.py` and `helpers/shell.py` show what `shell.exec_command` does
  before it returns.
