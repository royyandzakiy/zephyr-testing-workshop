---
name: zephyr-build-run
description: Build, flash, monitor and test a Zephyr app in this repo, on native_sim or on a real board, and diagnose a failing run. Use this whenever asked to build, flash, run, monitor, or "check that it works"; whenever running west twister or reading twister-out; and whenever a build, flash or test run has failed and the cause is not yet known. Also use it when choosing between native_sim and hardware, or when a board-specific flash command is needed for nRF5340DK, ESP32-S3, Nucleo G474RE or QEMU.
---

# Build, flash, run, diagnose

[`docs/NOTES-build-flash.md`](../../../docs/NOTES-build-flash.md) is the authoritative
per-board command reference in this repo, with a cheat sheet per platform. **Read it
before inventing a command.** This skill covers the decisions around those commands:
which target to pick, which console mode, how the emulation is wired, and what to do
when a run goes red.

## Always build in the container

The repo is edited on Windows and built in a Linux devcontainer.

```bash
docker ps --format '{{.ID}}\t{{.Image}}' | grep zephyr-devcontainer
```

```bash
docker exec <id> bash -lc 'cd /workspaces/zephyr-testing-workshop && <command>'
```

**Always give twister its own output directory**, or the run dies before it starts:

```bash
west twister -T apps/07-unit-conventions -p native_sim -O /tmp/tw07 --clobber-output
```

Twister rotates `twister-out/` into `twister-out.1` at startup. On the Windows bind
mount that rotation fails with `OSError: [Errno 39] Directory not empty: 'modules'`,
which looks like a twister bug and is not one.

## The three phases

Almost all flag confusion is a flag in the wrong phase.

```
cmake/ninja ──build──▶ zephyr.elf ──flash──▶ board ──observe──▶ pass/fail
 build args              flash args            device args
```

- Changes the bytes in the image, so it is a **build** arg: overlays, `CONFIG_*`,
  `--extra-args`, anything after `--`.
- Changes where the bytes go, so it is a **flash** arg: `--west-flash=`,
  `--west-runner`, `--dev-id`, `--esp-device`.
- Changes how you watch, so it is a **device** arg: `--device-serial`,
  `--device-serial-baud`.

`-p` means **pristine** in `west build` and **platform** in `west twister`. They are
unrelated.

An overlay is consumed at configure time. Passing one to `west flash` is meaningless
and the runner rejects it.

## Picking a target

Ask what the test needs before picking a board.

| The thing under test | Target |
|---|---|
| arithmetic, state machines, anything with no driver | `native_sim` |
| a driver, through an emulated bus or pin | `native_sim`, emulation only exists here |
| the shell, a UART protocol, a pytest end-to-end flow | `native_sim` first, hardware second |
| timing, a real peripheral, a real radio, power | hardware, no substitute |

### native_sim is 32-bit by default

```
native_sim/native        32-bit  (the default in this repo)
native_sim/native/64     64-bit
```

The variant is declared in the board's `board.yml` as soc `native`, variant `64`.
Reach for the 64-bit one when host library linking fails on a 32-bit multilib problem,
which is the usual reason C++ or GoogleTest builds break. If you switch, update every
command in the app's README, `platform_allow` in its `testcase.yaml`, and the root
`README.md` together.

`-p native_sim` in twister matches both. `-p native_sim/native` pins the 32-bit one.

### native_sim console: PTY or your shell

Set in `boards/native_sim_native.conf`. Exactly one of these:

```
CONFIG_UART_NATIVE_PTY_0_ON_STDINOUT=y   # zephyr.exe behaves like a console app
CONFIG_UART_NATIVE_PTY_0_ON_OWN_PTY=y    # allocates /dev/pts/N and prints it
```

- **STDINOUT** is what you want when a human runs `./build/zephyr/zephyr.exe` and
  types at it, and it is what every app in this repo sets.
- **OWN_PTY** is the default when neither is set. The binary prints
  `uart connected to pseudotty: /dev/pts/3` and waits. Attach with
  `python3 -m serial.tools.miniterm --raw /dev/pts/3 115200`, or let the binary do it:
  `./zephyr.exe --attach_uart_cmd="python3 -m serial.tools.miniterm --raw %s 115200"`.

This matters for tests. A `harness: console` or `harness: pytest` run needs the
harness to reach the console, and choosing the wrong mode produces a timeout with no
useful message. [`docs/NOTES-native-sim.md`](../../../docs/NOTES-native-sim.md) has
the details.

### Shell mode input

When `CONFIG_SHELL=y`, the device only answers after a prompt. Two consequences:

- Set `CONFIG_SHELL_VT100_COLORS=n`. Colour escapes are invisible to a human and very
  visible to `str.find()` and to a `harness: shell` regex.
- Commands are newline-terminated and the response ends at the next prompt. The
  `Shell` pytest fixture handles that for you; anything driving the port by hand has
  to read until `uart:~$`.

## How emulation gets wired

Three different arrangements are in use here, and they are not interchangeable.

### 1. A test-only overlay that reroutes an alias

`apps/03-emul-gpio/tests/emul/app.overlay`. The app binds `DT_ALIAS(sw0)`; the test's
own `app.overlay` points that alias at a `zephyr,gpio-emul` node instead of a real
controller. The application source does not change and does not know.

Move **every** alias the app touches. Rerouting `sw0` and leaving `led0` produces a
build that compiles, links, and dies on the first `gpio_pin_toggle_dt()` with a
SIGSEGV and no message.

### 2. An emulator compiled into the app

`apps/06-sensor/src/sensors/bme280_emul.c`, selected by
`target_sources_ifdef(CONFIG_EMUL app PRIVATE ...)`. The fake chip is a normal source
file that registers itself with `EMUL_DT_INST_DEFINE`. On a real board `CONFIG_EMUL`
is unset and the file is simply not compiled.

Use this when Zephyr has no emulator for the part and you need the real vendor driver
in the loop.

### 3. Faking your own functions, no devicetree at all

`apps/08-fff-mocks`. The test links the module and not its dependencies, so
`FAKE_VALUE_FUNC` supplies the missing symbols. No emulation framework involved. See
the `zephyr-ztest` skill.

### native_sim already has emulated controllers

Check before adding one. `native_sim.dts` already declares:

| Node | Compatible |
|---|---|
| `i2c0` | `zephyr,i2c-emul-controller` |
| `spi0` | `zephyr,spi-emul-controller` |
| `gpio0` | `zephyr,gpio-emul` |
| `espi0`, `mspi0` | the eSPI and MSPI equivalents |
| plus | `rtc-emul`, `adc-emul`, `dma-emul`, `otp-emul` |

So app 06 only adds a child node under the existing `i2c0`. App 03 adds its own second
`gpio_emul` node beside the built-in `gpio0`, which is also legal and is what you do
when you want the test's pins isolated from anything else.

### Real and emulated at the same time

Nothing stops a devicetree holding both. They are separate nodes on separate
controllers, and code selects by label or alias:

```dts
&i2c0 {                          /* the emulated controller */
    status = "okay";
    bme280_fake: bme280@77 { compatible = "bosch,bme280"; reg = <0x77>; status = "okay"; };
};

&i2c1 {                          /* a real bus on a real board */
    bme280_real: bme280@76 { compatible = "bosch,bme280"; reg = <0x76>; status = "okay"; };
};

/ { aliases { climate = &bme280_fake; }; };   /* one line decides which one ships */
```

Bind `DT_ALIAS(climate)` in the application and the alias is the only thing that
changes between a test build and a product build. Binding `DT_NODELABEL(bme280_fake)`
directly defeats the point, because now the source names the fake.

## Running tests

```bash
west twister -T apps/<app> -p native_sim -O /tmp/tw --clobber-output
```

Useful narrowing:

- `--test <scenario>` runs one scenario by name, for example `app07.feeder.ztest`.
- `--tag <tag>` selects on `tags:` in `testcase.yaml`.
- `--build-only` compiles without running.
- `--enable-ubsan`, `--enable-asan` for sanitizers on native_sim.
- `--coverage --coverage-tool gcovr`.

For hardware, prefer a `hardware-map.yaml` over per-invocation flags. The map's `id`
field is translated into the right runner argument automatically (`--dev-id` for
nrfutil, `--board-id` for pyocd, `--esp-device` for esp32), which is why the map form
needs no `--west-flash` escape hatch. The map and `-p` / `--device-serial` are
alternative approaches; do not mix them.

Board specifics that bite, all expanded in `docs/NOTES-build-flash.md`:

| Board | Watch for |
|---|---|
| nRF5340DK | runner `nrfutil`, not `nrfjprog`. `--dev-id`, not `--snr`. `sw0`/`led0` already in the board DTS. Console is `/dev/ttyACM1` while flashing goes via the J-Link, so they legitimately differ. |
| ESP32-S3 | `--flash-before` is required, because native USB re-enumerates on reset and a port opened first goes stale. Runner is `esp32` for all Espressif parts. No `sw0`/`led0` in the default DTS, so an overlay is mandatory. `ttyACM0` and `ttyUSB0` are different endpoints; pick one and keep it. |
| Nucleo G474RE | runner `pyocd`, `--dev-id` is the ST-LINK serial. `--west-flash` with no value is fine when only one probe is attached. |
| QEMU | `west build -t run`. Quit with `Ctrl-A` then `x`. |

## Twister harnesses

`harness:` in `testcase.yaml` decides how twister turns device output into a result.

| Harness | Decides from | Use when |
|---|---|---|
| `ztest` | ztest's own output, parsed per test case | `CONFIG_ZTEST=y`. The default, and the best reporting. |
| `gtest` | `[ RUN ] / [ OK ] / [ FAILED ]` lines, per test case | a GoogleTest binary |
| `console` | a regex over the console, one overall result | anything that just prints |
| `shell` | commands sent and substrings expected, no Python | a shell backdoor and simple assertions |
| `pytest` | pytest's exit status | anything needing computation, parametrization, or state |
| `robot`, `ctest`, `bsim`, `power` | their own tooling | not used in this repo |

`console` gives you one pass or fail for the whole run. `gtest` and `ztest` give you a
row per test case, which is what you want in CI. If a suite is GoogleTest and is using
`harness: console` with a regex, `harness: gtest` is the upgrade.

`console` config:

```yaml
harness: console
harness_config:
  type: one_line        # or multi_line, with ordered: true|false
  regex:
    - '\[  PASSED  \] \d+ tests?\.'
```

Single-quote the regex. In a double-quoted YAML scalar a backslash starts an escape,
so `\[` is a parse error.

## When a run fails

Read the logs, not the summary. Everything is under
`<outdir>/<platform>/.../<scenario>/`:

| File | Holds |
|---|---|
| `build.log` | compile and link errors |
| `device.log` | flash output, runner errors |
| `handler.log` | everything the device printed |
| `twister_harness.log` | the pytest side, including tracebacks |

The console summary tells you a scenario failed and little else. The per-case detail
is in `twister.json`:

```bash
python3 -c "
import json; d=json.load(open('/tmp/tw/twister.json'))
for t in d['testsuites']:
    if t.get('status') not in ('passed','filtered','not run'):
        print(t['status'].upper(), t['name'], t.get('reason',''))
"
```

Read the output **from the top**. Find the first line that reports an error and note
which tool printed it: `cmake`, `gcc`, `ld`, the runner, or the harness. That single
fact tells you whether to look at build files, C, Kconfig, the wiring, or the test.

Failure shapes seen in this repo:

| Symptom | Cause |
|---|---|
| `fatal error: <header>: No such file or directory` on a `CONFIG_` you set | Kconfig silently dropped the symbol because its `depends on` was unmet. Check `build/zephyr/.config`, not `prj.conf`. |
| a test passes alone and fails in the suite | state leaking. A missing `before` hook. ztest runs tests **alphabetically**, not in source order. |
| pytest `ScopeMismatch` at setup | a session-scoped fixture depending on `dut`, which is function-scoped |
| pytest waits the full timeout for a line the device definitely printed | `shell` and `dut` share one buffer. `exec_command()` already consumed it. |
| SIGSEGV on the first GPIO write in an emulated test | an alias was not rerouted and fell through to a controller this build never enabled |
| ESP32-S3 harness reads nothing | missing `--flash-before` |
| `OSError: Errno 39 Directory not empty` before anything builds | twister rotating `twister-out/` on the Windows bind mount. Use `-O /tmp/...`. |

## Reporting the result

Give the numbers, not an impression.

```
13 of 13 executed test configurations passed (100.00%)
126 of 126 executed test cases passed (100.00%)
```

If something failed, name the scenario, name the file you read, and quote the line
that explains it. If a build was not run, say so explicitly rather than implying it
passed.
