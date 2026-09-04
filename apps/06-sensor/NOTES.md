## esp32s3

```bash
west build -b esp32s3_devkitc/esp32s3/procpu \
  -s apps/06-sensor \
  -p always \
  -d build_esp32s3_sensor \
  -- -DEXTRA_DTC_OVERLAY_FILE="boards/esp32s3_devkitc_esp32s3_procpu.overlay"

west flash --runner esp32 --esp-device /dev/ttyACM0 -d build_esp32s3_sensor

python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
```

## esp32

```bash
west build -b esp32_devkitc/esp32/procpu \
  -s apps/06-sensor \
  -p always \
  -d build_esp32_sensor \
  -- -DEXTRA_DTC_OVERLAY_FILE="boards/esp32_devkitc_esp32_procpu.overlay" \
&& west flash --runner esp32 --esp-device /dev/ttyUSB0 -d build_esp32_sensor \
&& python3 -m serial.tools.miniterm --raw /dev/ttyUSB0 115200
```

## nrf5340dk

```bash
west build -b nrf5340dk/nrf5340/cpuapp \
  -s apps/06-sensor \
  -p always \
  -d build_nrf5340dk_sensor \
  -- -DEXTRA_DTC_OVERLAY_FILE="boards/nrf5340dk_nrf5340_cpuapp.overlay" \
&& west flash --runner nrfutil -d build_nrf5340dk_sensor \
&& python3 -m serial.tools.miniterm --raw /dev/ttyACM2 115200
```
---

## Testing

Two suites, both `native_sim` only:

```bash
west twister -T apps/06-sensor -p native_sim
```

| Suite | Scenario | What it links | What it proves |
|---|---|---|---|
| `tests/unit` | `app06.climate.logic` | `climate_logic.c` only | fixed-point rounding, alarm hysteresis. No driver, no bus, no board. |
| `tests/emul` | `app06.climate.emul` | `bme280.c` + `bme280_emul.c` + the real Bosch driver | the app talks to a fake chip over a fake bus and cannot tell |

`src/sensors/bme280_emul.c` is an emulated BME280. Zephyr ships emulators for
bmi160, bma4xx, f75303 and friends but not for this part, so it is ours. It
hangs off the `zephyr,i2c-emul-controller` that `native_sim` already provides
as `i2c0`, which is why the overlays only add a child node.

That is also why `west build -b native_sim/native` now runs the sensor on your
laptop with live readings, instead of needing a board.

### Known finding: UBSAN fires inside the Bosch driver

```bash
west twister -T apps/06-sensor -p native_sim --enable-asan --enable-ubsan
```

fails with:

```
zephyr/drivers/sensor/bosch/bme280/bme280.c:103:35:
    runtime error: left shift of negative value -4509070
```

This is **upstream Zephyr code, not ours** — line 103 is
`((var1 * (int64_t)data->dig_p2) << 12)`, part of the compensation formula
copied verbatim from the BME280 datasheet (see the comment at `bme280.c:88`).
`dig_p2` is negative in the datasheet reference calibration and on most real
parts, so the product is negative and left-shifting a negative signed value is
undefined behaviour in C. It is not an artefact of the emulator: a real BME280
returns the same shape of calibration data and would trip the same check.

Left in place deliberately. Nothing in CI runs UBSAN against this app
(`.github/workflows/sanitizers_native-sim_ci.yml` builds `apps/01-blinky`), and
it makes a better exercise than a footnote: turn the sanitizer on, read the
report, work out whether it is your bug, and decide what you would do about it
in a vendor driver you do not own.
