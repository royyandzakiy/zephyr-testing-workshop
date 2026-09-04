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
&& west flash --runner esp32 --esp-device /dev/ttyACM0 -d build_esp32_sensor \
&& python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
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