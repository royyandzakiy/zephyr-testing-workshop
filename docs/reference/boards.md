# Build, flash and Twister commands

For someone with the repo running who has reached for a board, or who needs to know
what a flag does. Commands verified against Zephyr v4.4.2 in the devcontainer, except
where marked as needing hardware.

Every command below belongs to one of three phases. Most flag confusion is a flag in
the wrong phase.

`-p` means **pristine** in `west build` and **platform** in `west twister`. They are
unrelated, and this is the most common mix-up.

## The three phases

```
cmake/ninja  ──build──▶  zephyr.elf  ──flash──▶  board  ──observe──▶  pass/fail
     ▲                                   ▲                    ▲
  build args                        flash args          device args
  -D... / --extra-args              --west-flash=       --device-serial
```

- Changes the bytes in the image, so it is a **build** arg: overlays, Kconfig,
  `CONFIG_*`.
- Changes where the bytes go or how they get there, so it is a **flash** arg: port,
  dev-id, runner options.
- Changes how you watch the result, so it is a **device** arg: console port, baud.

A devicetree overlay is consumed by the devicetree compiler at configure time. By the
time `west flash` runs its effect is already in `zephyr.bin`, so passing an overlay to
the flash step does nothing and the runner rejects it as unknown.

## `west build`

```bash
west build -b <board> -s <source_dir> -d <build_dir> -p always -- -D<VAR>=<value>
```

- `-b, --board`: target board, for example `nrf5340dk/nrf5340/cpuapp`. Board
  qualifiers (`/soc/core`) are required on multi-core SoCs.
- `-s, --source-dir`: app or test directory. Defaults to cwd.
- `-d, --build-dir`: output directory. Use a distinct one per board so builds do not
  clobber each other.
- `-p, --pristine`: wipe the build dir first. Takes `always` / `never` / `auto`; bare
  `-p` means `-p always`.
- `-t, --target`: run a specific build target, for example `-t menuconfig`,
  `-t rom_report`, `-t run`.
- `--`: everything after this goes to CMake rather than to west.

CMake args, all after `--` and prefixed with `-D`:

- `-DEXTRA_DTC_OVERLAY_FILE=<abs_path>`: **appends** an overlay on top of the
  auto-discovered set. This is what you want in almost every case.
- `-DDTC_OVERLAY_FILE=<abs_path>`: **replaces** the whole auto-discovered set.
  `app.overlay` is silently dropped.
- `-DEXTRA_CONF_FILE=<abs_path>`: appends a Kconfig fragment on top of `prj.conf`.
- `-DCONF_FILE=<abs_path>`: replaces `prj.conf` entirely.

Paths must be **absolute**. Relative paths resolve against the app source dir, not
your cwd, so use `$PWD/...`.

### Overlay auto-discovery

Relative to the source dir given by `-s`:

```
apps/04-shell-pytest/tests/drivers/gpio_button_toggle/
  app.overlay                                 # all boards, automatic
  prj.conf                                    # all boards, automatic
  boards/
    native_sim_native.conf                    # this board only, automatic
    esp32s3_devkitc_esp32s3_procpu.overlay    # this board only, automatic
```

Board filenames use the **normalized** board name, with slashes becoming underscores.

Discovery is relative to `-s`, so a `boards/` directory anywhere else is not found. In
this repo every suite carries whatever it needs, which is why none of the commands
below pass an overlay explicitly. That was not always true, so older notes elsewhere
may still show `--extra-args=DTC_OVERLAY_FILE=...` pointing at a workspace-root
`boards/`. There is no such directory now.

## `west flash`

```bash
west flash -d <build_dir> --runner <runner> [runner options]
```

- `-d, --build-dir`: which build to flash. Required with non-default build dirs.
- `-r, --runner`: `nrfutil`, `esp32`, `pyocd`, `jlink`, `openocd`.
- `-i, --dev-id <id>`: which physical probe or board, when more than one is attached.
  Understood by most runners.
- `--erase`: full chip erase before programming.

Runner-specific:

- `--esp-device <port>`, esp32 only: serial port esptool writes to.
- `--esp-baud-rate <baud>`, esp32 only: flashing baud, independent of console baud.
- `--runner pyocd`, ST and ARM CMSIS-DAP: `--dev-id` is the ST-LINK serial number.

On Nordic, `nrfutil` is the current runner. The `nrfjprog` runner still ships with
Zephyr v4.4.2, but it drives Nordic's older command line tools. `--snr` is still
accepted and is registered as an obsolete synonym for `--dev-id`
(`scripts/west_commands/runners/nrf_common.py:107`), so old commands keep working.
Write `--dev-id` in anything new.

Nordic without west:

```bash
nrfutil device program --firmware <build_dir>/zephyr/zephyr.hex --serial-number 1050073602
```

## Monitor

```bash
python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
```

`--raw` stops control characters being mangled. Without it, shell output and ANSI
escapes render wrong.

The port must be free. Close the monitor before flashing or running Twister.

**Port numbers depend on enumeration order and on what else is plugged in.** The
numbers in this file are what one particular desk saw. Check yours:

```bash
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

On the ESP32-S3, `ttyACM0` and `ttyUSB0` are genuinely different endpoints, not two
names for one thing. `ttyACM0` is the native USB-Serial-JTAG peripheral and `ttyUSB0`
is the onboard UART bridge. Pick one and use it consistently across build, flash and
monitor.

## `west twister`

```bash
west twister -p <board> -T <testsuite_root> [--device-testing ...]
```

### Selection

- `-p, --platform`: board to test. Repeatable. Platform, not pristine.
- `-T, --testsuite-root`: directory to scan for `testcase.yaml`. Repeatable.
- `--test <scenario>`: run one scenario by name, for example `app07.feeder.ztest`.
- `--tag <tag>`: select on `tags:` in `testcase.yaml`.
- `-O, --outdir`: output dir, default `twister-out/`.
- `-c, --clobber-output`: overwrite the outdir instead of rolling it over.

On Windows, always pass `-O /tmp/<name> --clobber-output`. Twister rotates
`twister-out/` into `twister-out.1` at startup, and that rotation fails on the bind
mount with `OSError: [Errno 39] Directory not empty`.

### Build phase

- `-x, --extra-args=VAR=value`: passed to CMake as `-DVAR=value`. Repeatable. Applies
  to **every platform in the run**; there is no per-board form.
- `--build-only`: compile, do not flash or run.
- `--test-only`: skip the build, run existing binaries.
- `--enable-asan`, `--enable-ubsan`: sanitizers, native_sim.
- `--coverage --coverage-tool gcovr`.

### Flash phase

- `--device-testing`: run on real hardware. Required for every on-target run.
- `--west-flash[="a,b,c"]`: use `west flash` instead of the build system's flash
  target. The value is a **comma-separated** list of extra args. Write
  `--west-flash="--dev-id=123"`, not `--west-flash="--dev-id 123"`, because the second
  is passed as one token and the runner rejects it.
- `--west-runner <runner>`: requires `--west-flash` to also be present.
- `--flash-before`: flash first, then open the serial port. The default order is the
  reverse. Needed on the ESP32-S3, whose native USB re-enumerates on reset, so a port
  opened first goes stale. J-Link and ST-LINK are separate CDC bridges and survive the
  reset, so they do not need it.

### Observe phase

- `--device-serial <port>`: port Twister opens to read test output and decide pass or
  fail. Never handed to CMake, never handed to `west flash`.
- `--device-serial-baud <baud>`: defaults to 115200. Unrelated to flashing baud.

On the ESP32-S3 the same port is named twice for two different consumers:
`--west-flash="--esp-device=/dev/ttyACM0"` for esptool writing, and
`--device-serial /dev/ttyACM0` for the harness reading. Nothing links them. On the
nRF5340DK they legitimately differ, because flashing goes through the J-Link by
`--dev-id` while the console is a separate CDC port.

### Reading a failed run

- `-v`, `-vv` for verbosity, `-ll DEBUG` for log level.
- `--pytest-args=<arg>`, repeatable, forwarded to pytest.

Logs live in `<outdir>/<platform>/.../<scenario>/`:

| File | Holds |
|---|---|
| `build.log` | compile and link errors |
| `device.log` | flash output, runner errors |
| `handler.log` | everything the device printed |
| `twister_harness.log` | the pytest side, including tracebacks |

See [`../troubleshooting.md`](../troubleshooting.md) for failures keyed by symptom.

## Hardware map

Replaces the per-invocation device flags, and is the better option for CI and for more
than one board.

```bash
west twister --device-testing --hardware-map apps/04-shell-pytest/hardware-map.yaml \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle
```

```yaml
- connected: true
  id: '1050073602'
  platform: nrf5340dk/nrf5340/cpuapp
  product: J-Link
  runner: nrfutil
  serial: /dev/ttyACM1
  baud: 115200

- connected: true
  id: '/dev/ttyACM0'
  platform: esp32s3_devkitc/esp32s3/procpu
  product: ESP32-S3
  runner: esp32
  serial: /dev/ttyACM0
  baud: 115200

- connected: true
  id: '0046002E3234510A37333934'
  platform: nucleo_g474re
  product: ST-LINK/V3
  runner: pyocd
  serial: /dev/ttyACM0
  baud: 115200
```

- `id` is translated into the runner-specific argument automatically: `--dev-id` for
  nrfutil, `--board-id` for pyocd, `--esp-device` for esp32. That is why the map form
  needs no `--west-flash` escape hatch.
- `runner_params` is a list of extra args, the map equivalent of stuffing values into
  `--west-flash`.
- `connected: false` entries are skipped.
- `west twister --generate-hardware-map map.yaml` scaffolds the file.

The map and the `-p` / `--device-serial` flags are alternative approaches. Do not mix
them in one invocation.

## Per-platform

The suite used in these examples is
`apps/04-shell-pytest/tests/drivers/gpio_button_toggle`. It carries its own
`app.overlay`, which supplies `sw0` and `led0` on a `gpio_emul` controller, so no
overlay argument is needed on any board.

### native_sim, `native_sim/native`

```bash
west build -b native_sim/native -p -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -d build_ns
```

```bash
./build_ns/zephyr/zephyr.exe
```

```bash
west twister -p native_sim -T apps/04-shell-pytest
```

32-bit by default. `native_sim/native/64` is the 64-bit variant, and it is the
fallback when host library linking fails on a multilib problem. Console mode is set in
`boards/native_sim_native.conf`; see [`native-sim.md`](native-sim.md).

### nRF5340DK, `nrf5340dk/nrf5340/cpuapp`

```bash
west build -b nrf5340dk/nrf5340/cpuapp -p -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -d build_nrf53
```

```bash
west flash -d build_nrf53 --runner nrfutil --dev-id 1050073602
```

```bash
west twister -p nrf5340dk/nrf5340/cpuapp --device-testing \
  --device-serial /dev/ttyACM1 --device-serial-baud 115200 \
  --west-flash="--dev-id=1050073602" --west-runner nrfutil \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle
```

`sw0` and `led0` already exist in the board DTS, so the app builds here without an
overlay too. Flashing goes through the J-Link, the console is a separate CDC port.

### ESP32-S3, `esp32s3_devkitc/esp32s3/procpu`

```bash
west build -b esp32s3_devkitc/esp32s3/procpu -p -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -d build_esp32s3
```

```bash
west flash -d build_esp32s3 --runner esp32 --esp-device /dev/ttyACM0
```

```bash
west twister -p esp32s3_devkitc/esp32s3/procpu --device-testing \
  --device-serial /dev/ttyACM0 --device-serial-baud 115200 --flash-before \
  --west-flash="--esp-device=/dev/ttyACM0" --west-runner esp32 \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle
```

`--flash-before` is required here. The `esp32` runner covers all Espressif parts, not
only the original ESP32. The board's default DTS has no `sw0` or `led0`, so the
application build needs `apps/04-shell-pytest/boards/esp32s3_devkitc_esp32s3_procpu.overlay`;
the test build does not, because `app.overlay` already supplies both.

### Nucleo G474RE, `nucleo_g474re`

```bash
west build -b nucleo_g474re -p -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -d build_nucleo
```

```bash
west flash -d build_nucleo --runner pyocd --dev-id 0046002E3234510A37333934
```

```bash
west twister -p nucleo_g474re --device-testing \
  --device-serial /dev/ttyACM0 --device-serial-baud 115200 \
  --west-flash --west-runner pyocd \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle
```

`--west-flash` with no value is fine when a single ST-LINK is attached. The target
pack has to be installed once, see
[`../setup/build-flash-monitor.md`](../setup/build-flash-monitor.md).

### QEMU, `qemu_cortex_m3`

```bash
west build -b qemu_cortex_m3 -p -s apps/00-hello -d build_qemu && west build -t run -d build_qemu
```

Quit with `Ctrl-A` then `x`.

## Things that are not flags

- `--west-flash-extra` does not exist. Runner args go inside `--west-flash="..."`.
- `--snr` is accepted but obsolete. Use `--dev-id`.
- `-p` in `west twister` is platform, not pristine. Twister always builds pristine.
- An overlay passed to `west flash` is rejected. Overlays are a build-time input.
