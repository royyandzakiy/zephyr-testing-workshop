# 03-emul-gpio

The same five button presses as `02-ztest`, except now they go through the
real GPIO driver, the real interrupt callback and the real `blinky.c`. What is
fake is the controller underneath, and a test-only overlay decided that.

**What changed since `02-ztest`:** one new directory, `tests/emul/`. Zero
changed files in `src/`. That is the whole of this session.

```bash
diff -r apps/02-ztest/src apps/03-emul-gpio/src
```

## What to learn here

- What a test-only `app.overlay` is: a devicetree fragment that belongs to the
  test application, not to the app, and that can reroute an alias the app
  binds without the app knowing.
- Why **both** aliases have to move. Reroute `sw0` and leave `led0`, and the
  first toggle from the callback runs against a controller this build never
  enabled. The comment in `tests/emul/app.overlay` is there because that
  failure mode is a SIGSEGV with no useful message.
- `gpio_emul_input_set()` and `gpio_emul_output_get()`: how a test drives a
  pin that does not exist.
- Why the suite uses `setup` and not `before`. `gpio_emul` fires registered
  callbacks straight out of `pin_configure()`, so re-running `blinky_init()`
  between tests looks exactly like a phantom button press.
- That `pressed_raw_level()` reads the polarity off `button.dt_flags` instead
  of hardcoding 0. Flip `GPIO_ACTIVE_LOW` in the overlay and the test still
  passes, which is the difference between a test that describes behaviour and
  one that describes wiring.

## Layout

```
03-emul-gpio/
├── src/                      IDENTICAL to 02-ztest/src
└── tests/
    ├── unit/                 unchanged from 02-ztest: pure logic
    └── emul/                 NEW
        ├── app.overlay       gpio_emul controller + rerouted sw0 and led0
        ├── prj.conf          CONFIG_GPIO_EMUL=y
        ├── CMakeLists.txt    links blinky.c AND blink_logic.c this time
        ├── testcase.yaml     scenario app03.blink.emul
        └── src/main.c        presses the pin, reads the other pin
```

## Run it

```bash
cd apps/03-emul-gpio
```

```bash
west twister -T apps/03-emul-gpio -p native_sim
```

One suite at a time:

```bash
west twister -T apps/ -p native_sim --test app03.blink.emul
```

The app itself still runs:

```bash
west build -b native_sim/native -p && ./build/zephyr/zephyr.exe
```

## Expected outcome

Two scenarios, both green:

```
INFO    - 2 test scenarios (2 configurations) selected
...
INFO    - 2 of 2 executed test configurations passed (100.00%)
```

`app03.blink.logic` and `app03.blink.emul`. The second one prints its five
presses through `TC_PRINT`:

```
START - test_presses_toggle_the_led
press 1: LED level 1
press 2: LED level 0
press 3: LED level 1
press 4: LED level 0
press 5: LED level 1
 PASS - test_presses_toggle_the_led
```

Those levels are read back off an emulated output pin, not returned from a
function. Nothing in `src/` knows.

## References

| | |
|---|---|
| [Emulators](https://docs.zephyrproject.org/latest/hardware/emulator/bus_emulators.html) | the framework, and the list of what Zephyr already emulates |
| [`gpio_emul`](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html) | the driver source at `$ZEPHYR_BASE/drivers/gpio/gpio_emul.c` is short and worth reading |
| [Devicetree overlays](https://docs.zephyrproject.org/latest/build/dts/howtos.html#set-devicetree-overlays) | which overlay files get picked up, and in what order |
| [apps/06-sensor](../06-sensor) | the same trick one layer down, on an I2C bus, with a chip Zephyr does not ship an emulator for |
