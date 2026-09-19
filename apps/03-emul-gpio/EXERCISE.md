# 03-emul-gpio - exercises

## ★ warm-up

1. **Delete one alias and watch it crash.** Remove the `led0 = &emul_led_0;` line from
   `tests/emul/app.overlay`, then run the suite:

   ```bash
   west twister -T apps/ -p native_sim --test app03.blink.emul
   ```

   *Check:* you can name the controller `led0` fell through to instead, and say why
   that controller is not enabled in this build. Put the line back when you are done.

2. **Flip the polarity.** Change the button in `tests/emul/app.overlay` from
   `GPIO_ACTIVE_LOW` to `GPIO_ACTIVE_HIGH`, then run the suite again. It still passes.
   The reason is one function:

   ```bash
   sed -n '/pressed_raw_level/,/^}/p' tests/emul/src/main.c
   ```

   *Check:* you can say what that function would have to look like for the flip to
   break the suite. Put the flag back when you are done.

3. **Watch the phantom press.** Move `emul_setup` from the suite's `setup` slot to its
   `before` slot in the `ZTEST_SUITE` line, then run the suite.

   *Check:* you can say which assertion failed and connect it to the comment above
   `emul_setup`. Put it back when you are done.

4. **Add an assertion for the starting state.** The suite already asserts the LED is
   off before the first press. Add one that asserts the button reads as not pressed.

   *Check:* it passes, and you derived the expected level from `button.dt_flags`
   rather than writing a literal.

## ★★ go deeper

1. **Test the debounce that is not there.** Press the button twice within 5 ms. Write
   the test that documents what the app currently does, even if you think it is wrong.

   *Check:* the test name describes the current behaviour, not the behaviour you would
   prefer.

2. **Emulate a stuck button.** Hold the pin at the active level with
   `gpio_emul_input_set()` and never release it. Assert what the app does.

   *Check:* you can say whether the result is a bug, and what you would need to know
   about the product before deciding.

3. **Shrink the emulated controller.** Change the `gpio_emul` node in
   `tests/emul/app.overlay` from `ngpios = <4>` to `ngpios = <2>` and renumber the
   button and LED. Then set it to `<1>` and run again:

   ```bash
   west twister -T apps/ -p native_sim --test app03.blink.emul
   ```

   *Check:* the first change passes and the second fails. You can quote the error from
   the second one. Put it back to `<4>` when you are done.

4. **Try to run the emulated suite on real hardware.**

   ```bash
   west twister -T apps/03-emul-gpio -p nrf5340dk/nrf5340/cpuapp --build-only
   ```

   *Check:* you can name which mechanism refused, `platform_allow` in `testcase.yaml`
   or the devicetree, and quote the message that told you. Then predict what happens
   to `tests/unit` under the same command, and check whether you were right.

## ★★★ off the map

1. **Emulate a second controller.** Add a second `zephyr,gpio-emul` node and put the
   LED on it while the button stays on the first.

   *Why it is interesting:* it forces you to notice that a `gpio_dt_spec` is a
   `(port, pin, flags)` triple and not a pin number. That is what lets the same code
   bind a real controller and an emulated one.

2. **Find out what `gpio_emul` cannot do.** Read the driver and list three behaviours
   of a real pin it does not model.

   ```bash
   less $ZEPHYR_BASE/drivers/gpio/gpio_emul.c
   ```

   *Why it is interesting:* every one of those is a bug class your emulated suite stays
   green through, and that list is how you decide what still has to run on a board.

3. **Write the equivalent suite for a peripheral Zephyr does not emulate.** Pick one
   from your own work and sketch what the emulator would have to answer.

   *Why it is interesting:* `apps/06-sensor` is this exercise done for a BME280, so
   there is a worked answer to compare against afterwards.

## If you want to go further

- [Emulator API](https://docs.zephyrproject.org/latest/hardware/emulator/index.html) - `EMUL_DT_DEFINE`, the backend APIs, and how a bus emulator is registered.
- [Zephyr's own emulated driver tests](https://github.com/zephyrproject-rtos/zephyr/tree/main/tests/drivers) - the largest collection of worked examples, and the place to take conventions from.
- [`docs/concepts/what-you-cannot-test.md`](../../docs/concepts/what-you-cannot-test.md) - the full list of what `gpio_emul` does not model, and what else no test here covers.
