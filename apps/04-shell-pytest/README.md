# 04-shell-pytest

A shell command as a test backdoor, and `pytest` asserting on what the device
says from outside it. The first suite in this repo where the test process and
the device under test are two different programs.

**What changed since `03-emul-gpio`:** the app went back to a flat `main.c`
with no seam, and the test moved out of the device entirely. The suite now
sits in `tests/drivers/gpio_button_toggle/pytest/` and talks over a UART.

## What to learn here

- What `twister_harness` gives you: a `shell` fixture that is already
  connected to the device, whether that device is a `native_sim` process on
  your laptop or a board on a runner.
- Why `test_harness.c` is a separate file from `src/main.c`. The shell command
  is compiled into the test image only, so the backdoor never ships.
- That `harness: pytest` in `testcase.yaml` is the entire opt-in, and that
  `harness: shell` next to it does string matching with no Python at all.
- Why `CONFIG_SHELL_VT100_COLORS=n` is in `prj.conf`: colour escape codes are
  invisible to you and very visible to `str.find()`.
- The one thing this buys you over ztest: the assertions run on a machine that
  can talk to a network, a database and a file system, so an end-to-end test
  is possible at all.

## Layout

```
04-shell-pytest/
├── src/main.c                      flat app, no seam, no shell commands
├── hardware-map.yaml               probe serial + port, for --device-testing
└── tests/drivers/gpio_button_toggle/
    ├── app.overlay                 gpio_emul + rerouted sw0 and led0
    ├── prj.conf                    CONFIG_SHELL=y, CONFIG_GPIO_EMUL=y
    ├── test_harness.c              THE BACKDOOR. test_btn shell command.
    ├── testcase.yaml               harness: pytest
    └── pytest/test_gpio_toggle.py  five presses, asserted from outside
```

## Run it

```bash
cd apps/04-shell-pytest
```

```bash
west twister -T apps/04-shell-pytest -p native_sim
```

Against a board, after editing `hardware-map.yaml` with your own probe serial
and serial port:

```bash
west twister -T apps/04-shell-pytest --device-testing --hardware-map hardware-map.yaml
```

On ESP32-S3 add `--flash-before`, or the harness holds a stale descriptor
after the USB peripheral re-enumerates.

## Expected outcome

```
INFO    - 1 test scenarios (1 configurations) selected
...
INFO    - 1 of 1 executed test configurations passed (100.00%)
```

The scenario is `drivers.gpio.button_toggle`. The interesting output is in
`twister-out/native_sim/.../handler.log`, which holds everything the device
said, including the shell prompt and the echo of each `test_btn`.

When it goes red, that log is the answer almost every time. Learning your way
around `twister-out/` is most of debugging a failed CI run.

## References

| | |
|---|---|
| [`docs/PYTEST_GUIDE.md`](../../docs/PYTEST_GUIDE.md) | local notes on `twister_harness`, the `dut` and `Shell` fixtures |
| [Twister pytest harness](https://docs.zephyrproject.org/latest/develop/test/pytest.html) | the fixtures, and what `harness_config` accepts |
| [Shell subsystem](https://docs.zephyrproject.org/latest/services/shell/index.html) | `SHELL_CMD_REGISTER`, subcommand sets, and the argument count checks |
| [apps/05-pytest-advanced](../05-pytest-advanced) | fixtures, parametrization, markers, and the same thing with no twister at all |
