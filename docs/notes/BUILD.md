## ESP32

```bash
west build -d build_esp32_shell -s apps/05-shell-pytest -p always -b esp32s3_devkitc/esp32s3/procpu --no-sysbuild \
&& west flash --runner esp32 --esp-device /dev/ttyUSB0 -d build_esp32_shell \
&& python3 -m serial.tools.miniterm --raw /dev/ttyUSB0 115200
```

## Native Sim

```bash
west build -d build_nativesim_shell -s apps/05-shell-pytest -p always -b native_sim/native \
&& ./build_nativesim_shell/zephyr/zephyr.exe
```

## nRF

```bash
west build -d build_nrf52 -s apps/00-hello -p always -b nrf52840dk/nrf52840 \
&& west flash --runner nrfutil -d build_nrf52 \
&& python3 -m serial.tools.miniterm --raw /dev/ttyACM0 115200
``` 