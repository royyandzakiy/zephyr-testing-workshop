# 03-emul-gpio - exercises

Both suites are green before the break and you want the extra credit.

The answers are in the upstream Zephyr docs rather than in this repo.

## ★ warm-up

1. **Break it the interesting way.** Delete the `led0 = &emul_led_0;` line
   from `tests/emul/app.overlay` and run the suite.
   *Check:* you can explain the crash in terms of which controller `led0`
   fell through to, and why it is not enabled in this build.

2. **Flip the polarity.** Change the button in `app.overlay` from
   `GPIO_ACTIVE_LOW` to `GPIO_ACTIVE_HIGH`.
   *Check:* the suite still passes, and you can point at the line that made
   that true.

3. **Watch the phantom press.** Change `emul_setup` to a `before` hook and run
   the suite.
   *Check:* you can describe what failed and connect it to the comment above
   `emul_setup`.

4. **Add an assertion for the starting state.** The suite already asserts the
   LED is off before the first press. Add one that asserts the button reads as
   not pressed.
   *Check:* it passes, and you used `button.dt_flags` rather than a literal.

## ★★ go deeper

1. **Test the debounce that is not there.** Press the button twice within
   5 ms. What does the app do? Write the test that documents the current
   behaviour, even if you think the behaviour is wrong.
   *Check:* the test name says what the app does, not what you wish it did.

2. **Emulate a stuck button.** Hold the pin at the active level and never
   release it. Assert what the app does.
   *Check:* you can say whether the result is a bug, and what you would need
   to know about the product to decide.

3. **Move the emulated pins.** Change the `gpio_emul` node from `ngpios = <4>`
   to `ngpios = <2>` and renumber the button and LED.
   *Check:* it still passes. Now set `ngpios = <1>` and read the error.

4. **Run both suites on real hardware.** `tests/unit` should pass on anything.
   `tests/emul` should not even build for a board without `gpio_emul`. Confirm
   both, and work out which line of which file stopped it.
   *Check:* you can name the mechanism that refused, `platform_allow` or the
   devicetree.

## ★★★ off the map

1. **Emulate a second controller.** Add a second `zephyr,gpio-emul` node and
   put the LED on it while the button stays on the first.
   *Why it is interesting:* it forces you to notice that `gpio_dt_spec` is a
   `(port, pin, flags)` triple and not a pin number, which is most of what
   makes this portable.

2. **Find out what `gpio_emul` cannot do.** Read
   `$ZEPHYR_BASE/drivers/gpio/gpio_emul.c` and list three behaviours of a real
   pin it does not model.
   *Why it is interesting:* every one of those is a bug class your emulated
   suite will stay green through, and knowing the list is how you decide what
   still has to run on a board.

3. **Write the equivalent suite for a peripheral Zephyr does not emulate.**
   Pick one from your own work. Sketch what the emulator would have to answer.
   *Why it is interesting:* apps/06-sensor is exactly this exercise, done for
   a BME280, so you have a worked answer to compare against afterwards.

## If you want to go further

- [Emulator API](https://docs.zephyrproject.org/latest/hardware/emulator/index.html) - `EMUL_DT_DEFINE`, the backend APIs, and how a bus emulator is registered.
- [Zephyr's own emulated driver tests](https://github.com/zephyrproject-rtos/zephyr/tree/main/tests/drivers) - the best available collection of worked examples, and the place to copy conventions from.
- [`docs/NOTES-testing.md`](../../docs/NOTES-testing.md) - local notes on where each suite should run.
