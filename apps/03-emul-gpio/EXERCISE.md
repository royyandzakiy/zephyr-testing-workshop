# 03-emul-gpio - exercises

## ★ warm-up

1. **Delete one alias and watch it crash.** Remove the `led0 = &emul_led_0;` line
   from `tests/emul/app.overlay` and run the suite.

   *Check:* you can name the controller `led0` fell through to instead, and say why
   that controller is not enabled in this build. Put the line back when you are done.

2. **Flip the polarity.** Change the button in `app.overlay` from `GPIO_ACTIVE_LOW`
   to `GPIO_ACTIVE_HIGH` and run the suite.

   *Check:* it still passes, and you can point at the function in
   `tests/emul/src/main.c` that made that true. Put it back when you are done.

3. **Watch the phantom press.** Change `emul_setup` from the suite's `setup` hook to
   its `before` hook and run the suite.

   *Check:* you can say which assertion failed and connect it to the comment above
   `emul_setup`. Put it back when you are done.

4. **Add an assertion for the starting state.** The suite already asserts the LED is
   off before the first press. Add one that asserts the button reads as not pressed.

   *Check:* it passes, and you derived the expected level from `button.dt_flags`
   rather than writing a literal.

## ★★ go deeper

1. **Test the debounce that is not there.** Press the button twice within 5 ms. Write
   the test that documents what the app currently does, even if you think it is
   wrong.

   *Check:* the test name describes the current behaviour, not the behaviour you
   would prefer.

2. **Emulate a stuck button.** Hold the pin at the active level and never release it.
   Assert what the app does.

   *Check:* you can say whether the result is a bug, and what you would need to know
   about the product before deciding.

3. **Move the emulated pins.** Change the `gpio_emul` node from `ngpios = <4>` to
   `ngpios = <2>` and renumber the button and LED.

   *Check:* it still passes. Then set `ngpios = <1>` and read the error you get.

4. **Run both suites on real hardware.** `tests/unit` should pass on anything.
   `tests/emul` should not even build for a board without `gpio_emul`.

   *Check:* you can name which mechanism refused, `platform_allow` in
   `testcase.yaml` or the devicetree, and you can quote the message that told you.

## ★★★ off the map

1. **Emulate a second controller.** Add a second `zephyr,gpio-emul` node and put the
   LED on it while the button stays on the first.

   *Why it is interesting:* it forces you to notice that a `gpio_dt_spec` is a
   `(port, pin, flags)` triple and not a pin number, which is most of what makes this
   portable in the first place.

2. **Find out what `gpio_emul` cannot do.** Read
   `$ZEPHYR_BASE/drivers/gpio/gpio_emul.c` and list three behaviours of a real pin it
   does not model.

   *Why it is interesting:* every one of those is a bug class your emulated suite
   stays green through, and that list is how you decide what still has to run on a
   board.

3. **Write the equivalent suite for a peripheral Zephyr does not emulate.** Pick one
   from your own work and sketch what the emulator would have to answer.

   *Why it is interesting:* `apps/06-sensor` is this exercise done for a BME280, so
   there is a worked answer to compare against afterwards.

## If you want to go further

- [Emulator API](https://docs.zephyrproject.org/latest/hardware/emulator/index.html) - `EMUL_DT_DEFINE`, the backend APIs, and how a bus emulator is registered.
- [Zephyr's own emulated driver tests](https://github.com/zephyrproject-rtos/zephyr/tree/main/tests/drivers) - the largest collection of worked examples, and the place to take conventions from.
- [`docs/NOTES-testing.md`](../../docs/NOTES-testing.md) - local notes on where each suite should run.
