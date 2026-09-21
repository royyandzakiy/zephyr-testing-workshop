# Build, flash and monitor a board

A check that the toolchain works, using Zephyr's own `samples/hello_world` rather than
anything in this repo. First on `native_sim`, which needs no hardware, then on a real
board. Each vendor needs slightly different one-time setup, and those are below.

Once this works, [`../reference/boards.md`](../reference/boards.md) is the full command
reference for the boards in this repo.

## native_sim

Build the upstream sample:

```bash
west build -b native_sim/native -s $ZEPHYR_BASE/samples/hello_world -p always -d build_hello_world
```

Run it like any other executable:

```bash
./build_hello_world/zephyr/zephyr.exe
```

The Zephyr boot banner comes first, then the one line the sample prints:

```
Hello World! native_sim/native
```

The board target is not in the sample's source. `main()` prints `CONFIG_BOARD_TARGET`,
which Kconfig generated, so the same binary built for another board prints that board's
name instead.

## Finding your board

Every supported board has a page with its own build and flash notes, listed under
[Supported Boards and Shields](https://docs.zephyrproject.org/latest/boards/index.html#supported-boards-and-shields).
The filters down the left narrow by vendor, architecture and supported features.

![The Zephyr supported boards index](../imgs/board-search-1.png)
![The same list filtered by vendor](../imgs/board-search-2.png)

The board target string that page gives you is what goes after `-b`. The three below
are the ones this repo carries overlays for.

Port numbers depend on enumeration order and on what else is plugged in, so the ones in
these commands will not match your machine. Check which ports you have first:

```bash
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

## Nordic

No one-time setup. `nrfutil` and the Nordic command line tools are in the image, and
`arm-zephyr-eabi` is in the SDK.

```bash
west build -b nrf5340dk/nrf5340/cpuapp -s $ZEPHYR_BASE/samples/hello_world -p always -d build_hello_world
```

```bash
west flash -d build_hello_world
```

`west flash` reads the board out of the build directory and picks `nrfutil` from it.
With one probe attached it finds the probe without being told which.

```bash
python3 -m serial.tools.miniterm --raw /dev/ttyACM1 115200
```

`Ctrl-]` exits the monitor. The nRF5340DK enumerates more than one port: flashing goes
through the J-Link and the console is a separate CDC port, so the console is usually
the higher-numbered one.

Driving the tools directly, without west:

```bash
nrfutil device program --firmware build_hello_world/zephyr/zephyr.hex
```

```bash
nrfutil device reset
```

[nRF5340DK board page](https://docs.zephyrproject.org/latest/boards/nordic/nrf5340dk/doc/index.html)

## Espressif

The Xtensa toolchains are already installed. `ZSDK_TOOLCHAINS` in
[`.devcontainer/devcontainer.json`](../../.devcontainer/devcontainer.json) lists both
`xtensa-espressif_esp32_zephyr-elf` and `xtensa-espressif_esp32s3_zephyr-elf`, and
`setup-sdks.sh` installs them on first container start. `west sdk list` prints what you
have.

One thing is still needed per workspace, because the Espressif binary blobs are not in
the git tree:

```bash
west blobs fetch hal_espressif
```

Then build, flash and monitor:

```bash
west build -b esp32s3_devkitc/esp32s3/procpu -s $ZEPHYR_BASE/samples/hello_world -p always -d build_hello_world
```

```bash
west flash -d build_hello_world --runner esp32 --esp-device /dev/ttyACM0
```

```bash
python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
```

The `esp32` runner covers every Espressif part, not only the original ESP32. On the
ESP32-S3, `ttyACM0` and `ttyUSB0` are different endpoints rather than two names for one
thing: `ttyACM0` is the native USB-Serial-JTAG peripheral and `ttyUSB0` is the onboard
UART bridge. Use the same one for flashing and monitoring.

### Adding a toolchain for a different chip

For an ESP32-S2, a RISC-V ESP32-C3 or anything else, append the triple to
`ZSDK_TOOLCHAINS` in `containerEnv` and restart the container. `setup-sdks.sh` tops up
the shared SDK volume incrementally, so nothing already there is downloaded again. To
install it without restarting:

```bash
/opt/devcontainer/fetch-zephyr.sh
```

That re-runs the SDK setup for whichever toolchains `ZSDK_TOOLCHAINS` currently names.

### `C compiler ... xtensa-espressif_esp32s3_zephyr-elf-gcc not found`

CMake prints this from `cmake/compiler/gcc/target.cmake` with the full path it looked
at, under `$ZEPHYR_SDK_INSTALL_DIR`. The toolchain for that chip is not in the SDK
volume.

```bash
west sdk list
```

If the triple is missing from that output, add it to `ZSDK_TOOLCHAINS` and run
`/opt/devcontainer/fetch-zephyr.sh` as above. On a bare host with no container,
`west sdk install -t <triple>` does the same job.

[ESP32-S3 DevKitC board page](https://docs.zephyrproject.org/latest/boards/espressif/esp32s3_devkitc/doc/index.html)

## ST Microelectronics

pyocd needs a target support pack per chip family, and it does not ship with any. Find
the target name for your board:

```bash
pyocd list
```

```bash
pyocd pack install stm32g474retx
```

`pyocd list --targets` prints every target it could install, which is useful when
`pyocd list` shows the probe but not a usable target name.

```bash
west build -b nucleo_g474re -s $ZEPHYR_BASE/samples/hello_world -p always -d build_hello_world
```

```bash
west flash -d build_hello_world --runner pyocd
```

With more than one ST-LINK attached, name the one you mean. The id is the Unique ID
column from `pyocd list`:

```bash
west flash -d build_hello_world --runner pyocd --dev-id 0046002E3234510A37333934
```

```bash
python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
```

### `Target type stm32g474retx not recognized`

The pack is not installed. `pyocd list` shows this as a cross next to the target name:

```
  #   Probe/Board     Unique ID                  Target
------------------------------------------------------------------
  0   STLINK-V3       0046002E3234510A37333934   ✖︎ stm32g474retx
      NUCLEO-G474RE
```

```bash
pyocd pack install stm32g474retx
```

The same failure through west reads
`FATAL ERROR: command exited with status 1: pyocd flash ...`, with the pyocd message a
few lines above it.

[Nucleo G474RE board page](https://docs.zephyrproject.org/latest/boards/st/nucleo_g474re/doc/index.html)

## References

| | |
|---|---|
| [`../reference/boards.md`](../reference/boards.md) | the per-board commands for the suites in this repo, and what each flag does |
| [`../reference/native-sim.md`](../reference/native-sim.md) | the two native_sim board targets and where console output goes |
| [`devcontainer.md`](devcontainer.md) | where the SDK volume lives and what `setup-sdks.sh` does |
| [`../troubleshooting.md`](../troubleshooting.md) | build, flash and Twister failures keyed by the error you see |
| [Supported boards](https://docs.zephyrproject.org/latest/boards/index.html#supported-boards-and-shields) | every board Zephyr supports, with a page each |
