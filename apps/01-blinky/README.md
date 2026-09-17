# 01-blinky

A button toggles an LED, through a GPIO interrupt callback. One flat `main.c`,
no seam, no tests. Deliberately the code you would inherit.

**What changed since `00-hello`:** the app now binds devicetree nodes. `sw0`
and `led0` are aliases, and which pins they land on is decided by
`boards/*.overlay`, not by anything in `src/`.

## What to learn here

- What a devicetree alias is, and why the application asks for `sw0` rather
  than for a pin number.
- `GPIO_DT_SPEC_GET` and what it produces at compile time: a port, a pin and
  a flags word, all constant-folded.
- Why `GPIO_ACTIVE_LOW` in the overlay means `gpio_pin_get_dt()` can return 1
  while the pin is electrically at 0, and what that costs you when you write
  a test later.
- Why this file is hard to test. There is no function here that takes an
  input and returns an answer; every decision is tangled with a driver call.
  That observation is the whole reason app 02 exists.

## Layout

```
01-blinky/
├── CMakeLists.txt
├── prj.conf                  CONFIG_GPIO=y
├── boards/                   one overlay per board, all providing sw0 and led0
│   ├── native_sim_native.overlay      gpio_emul, so the app builds off-target
│   ├── nrf5340dk_nrf5340_cpuapp.overlay
│   ├── esp32s3_devkitc_esp32s3_procpu.overlay
│   └── ...
└── src/main.c                everything: init, callback, state, printk
```

## Run it

```bash
cd apps/01-blinky
```

```bash
west build -b native_sim/native -p
```

```bash
./build/zephyr/zephyr.exe
```

On a board:

```bash
west build -b nrf5340dk/nrf5340/cpuapp -p && west flash
```

## Expected outcome

On `native_sim`:

```
*** Booting Zephyr OS build v4.4.2 ***
GPIO Button + LED Toggle started
Ready. Press the button to toggle LED.
```

And then nothing, forever. There is no button on your laptop, and nothing in
this app can press the emulated one. That is not a bug in the app, it is the
gap that apps 02 and 03 close.

On a board, pressing the button gives you:

```
Button pressed! LED is now ON
Button pressed! LED is now OFF
```

There are still no tests in this folder. `west twister -T apps/01-blinky`
finds nothing.

## References

| | |
|---|---|
| [Devicetree guide](https://docs.zephyrproject.org/latest/build/dts/index.html) | start with "Introduction to devicetree", then "Devicetree HOWTOs" |
| [GPIO API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html) | `gpio_pin_configure_dt`, `gpio_pin_interrupt_configure_dt`, and the active-low flag |
| [`DT_ALIAS`](https://docs.zephyrproject.org/latest/build/dts/api/api.html#c.DT_ALIAS) | and why `zephyr,user` is the other common way to do this |
| [Blinky sample](https://docs.zephyrproject.org/latest/samples/basic/blinky/README.html) | upstream's version, which this one is a button-driven variant of |
