# Testing gpio Button Emulation

**General Notes**:

- Here is provided 4 different platform targets that are all tested and runs correctly
- All boards that runs `tests/drivers/gpio_button_toggle` will automatically use `app.overlay` (the default overlay used by all boards, unless specified)
- If a board dts or overlay does not yet have sw0 and btn0, it will fail to compile with showing device tree symbol errors

## Off-Target Testing

This tests runs purely on the PC, without any board connected

### Testing on Native Sim (off-target)

Build, Flash, Monitor

```bash
west build \
  -p always \
  -b native_sim/native \
  -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle \
  -d build_nativesim_test_gpio_toggle -- \
  -DEXTRA_CONF_FILE=../../../boards/native_sim_native.conf

build_nativesim_test_gpio_toggle/zephyr/zephyr.exe
```

Automated testing via Twister

```bash
west twister \
  -p native_sim/native \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle \
  --extra-args=DTC_OVERLAY_FILE=../../../boards/native_sim_native.overlay \
  --extra-args=EXTRA_CONF_FILE="../../../boards/native_sim_native.conf"
```

added flags:
- `--extra-args=DTC_OVERLAY_FILE`: native_sim (obviously) does NOT have a built in switch0 and led0 in its default .dts files, hence the overlay needs to be called explicitly. another thing is, currently the native_sim overlay is stored inside the root/boards, hence does NOT get automatically captured because our target is not to root, but instead to `tests/drivers/gpio_button_toggle`. also, native_sim MUST NOT automatically use `app.overlay`, because in app.overlay, led is never defined, hence it needs to use this overlay instead
- [[explain the need to use the added .conf]]

## On-Target Testing

These tests use real boards connected to the PC

[[todo: fix the inconsistent flags: west-flash-extra]]

### Testing on nRF5340dk

Build, Flash, Monitor

```bash
west build -b nrf5340dk/nrf5340/cpuapp -p always -d build_nrf53_test_gpio_toggle -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -p always

west flash -d build_nrf53_test_gpio_toggle --runner nrfutil -- --dev-id 1050073602
# or
nrfutil device program --firmware build_nrf53_test_gpio_toggle/zephyr/zephyr.hex --serial-number 1050073602

python3 -m serial.tools.miniterm --raw /dev/ttyACM1 115200
```

Automated testing via Twister

```bash
west twister --device-testing --hardware-map apps/04-shell-pytest/hardware-map.yaml -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle

# or, without hardware-map
west twister \
  -p nrf5340dk/nrf5340/cpuapp \
  --device-testing \
  --device-serial /dev/ttyACM1 \
  --device-serial-baud 115200 \
  --west-flash="--dev-id=1050073602" \
  --west-runner nrfutil \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle
```

```bash
# hardware-map.yaml
- connected: true
  id: '1050073602'
  platform: nrf5340dk/nrf5340/cpuapp
  product: J-Link
  runner: nrfutil
  serial: /dev/ttyACM1
```

### Testing on ESP32S3

Build, Flash, Monitor

```bash
west build -b esp32s3_devkitc/esp32s3/procpu \
  -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle \
  -p always \
  -d build_esp32s3_test_gpio_toggle \
  -- -DEXTRA_DTC_OVERLAY_FILE="../../../boards/esp32s3_devkitc_esp32s3_procpu.overlay"

west flash --runner esp32 --esp-device /dev/ttyACM0 -d build_esp32s3_test_gpio_toggle

python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
```

Automated testing via Twister

```bash
west twister \
  -p esp32s3_devkitc/esp32s3/procpu \
  --device-testing \
  --device-serial /dev/ttyACM0 \
  --device-serial-baud 115200 \
  --flash-before \
  --west-flash="--esp-device=/dev/ttyACM0" \
  --west-runner esp32 \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle \
  --extra-args=DTC_OVERLAY_FILE=../../../boards/esp32s3_devkitc_esp32s3_procpu.overlay \
  --extra-args=EXTRA_DTC_OVERLAY_FILE=app.overlay
```

added flags:
- `--flash-before`: by default is, the harness opens the serial port first, then flashes, causing it to be stale and fail to read. flashes first and opens the serial connection afterwards, and it propagates into the generated pytest command
- `--west-runner esp32`: required to be able to flash. the esp32 is universal for espressif chips, not just the esp32 type board.
- `--extra-args=DTC_OVERLAY_FILE`: esp32s3 does NOT have a built in switch0 and led0 in its default .dts files, hence the overlay needs to be called explicitly. another thing is, currently the esp32s3 is stored inside the root/boards, hence does NOT get automatically captured because our target is not to root, but instead to `tests/drivers/gpio_button_toggle`
- `--west-flash`: this is a different extra arg than the `--extra-args` flag, because this is used during flashing, while `--extra-args` is used by cmake during compile time. here we add where to find and flash the esp device. this is consumed by `esptool`
- `--device-serial`: this is different from `--west-flash`, as this one is actually used by twister harness to collect outputs and decide pass/fail

```bash
# hardware-map.yaml
- connected: true
  id: '/dev/ttyACM0'
  platform: esp32s3_devkitc/esp32s3/procpu
  product: ESP32-S3
  runner: esp32
  serial: /dev/ttyACM0
  baud: 115200
```

### Testing on Nucleo G4

Build, Flash, Monitor

```bash
west build -b nucleo_g474re -s apps/04-shell-pytest/tests/drivers/gpio_button_toggle -p always -d build_nucleog4_test_gpio_toggle

west flash --runner pyocd -d build_nucleog4_test_gpio_toggle/
# or, to be specific
west flash --runner pyocd -d build_nucleog4_test_gpio_toggle/ -- --dev-id 0046002E3234510A37333934

python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
```

Automated testing via Twister

```bash
west twister --device-testing --hardware-map hardware-map.yaml -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle

# or, without hardware-map
west twister \
  -p nucleo_g474re \
  --device-testing \
  --device-serial /dev/ttyACM0 \
  --device-serial-baud 115200 \
  --west-flash \
  --west-runner pyocd \
  -T apps/04-shell-pytest/tests/drivers/gpio_button_toggle
```

```bash
# hardware-map.yaml
- connected: true
  id: '0046002E3234510A37333934'
  platform: nucleo_g474re
  product: ST-LINK/V3
  runner: pyocd
  serial: /dev/ttyACM0
  baud: 115200
```