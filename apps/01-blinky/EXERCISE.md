# 01-blinky - exercises

## ★ warm-up

1. **Find the generated devicetree.** Open
   `build/zephyr/include/generated/zephyr/devicetree_generated.h` and search for
   `sw0`.

   *Check:* you can point at the line where the alias becomes a node, and name the
   overlay file that put it there.

2. **Break the alias.** Rename `sw0` to `sw1` in the board overlay you are building
   for, and rebuild.

   *Check:* you can quote the error, and say whether it names the macro or the pin.
   That shape of error is what a missing devicetree node always looks like. Put the
   name back when you are done.

3. **Move the LED to a different pin.** In `boards/native_sim_native.overlay`, change
   the LED from pin 10 to pin 12 and rebuild.

   *Check:* it still builds, and `git status` shows nothing changed under `src/`. Put
   the pin back when you are done.

4. **Predict the polarity.** The button in the native_sim overlay is
   `GPIO_PULL_UP | GPIO_ACTIVE_LOW`. Write down which raw pin level means "pressed",
   then compare against `pressed_raw_level()` in
   `apps/03-emul-gpio/tests/emul/src/main.c`.

   *Check:* your answer matches what that function computes.

## ★★ go deeper

1. **Add a second button.** Give the overlay an `sw1`, bind it, and make it turn the
   LED off unconditionally instead of toggling.

   *Check:* it builds for `native_sim` and for at least one real board, without you
   touching the other overlay.

2. **Debounce it.** The callback fires on every edge. Add a `k_work_delayable` so
   presses closer together than 50 ms are ignored.

   *Check:* you can say what you would need in order to test this, and why that is
   harder than testing the toggle.

3. **Count the ways this file resists testing.** List every line in `main.c` that a
   test cannot reach without a board or an emulator. Then write down the smallest
   change that would move the toggle decision somewhere testable.

   *Check:* compare your answer against `apps/02-ztest/src/blink_logic.h`. If they
   disagree, work out which one you would rather maintain.

4. **Build for two boards in one command.**

   ```bash
   west twister -T apps/01-blinky --build-only -p native_sim -p qemu_cortex_m3
   ```

   *Check:* twister reports both builds even though there is no test here, and you
   can say where under `twister-out/` it put each one.

## ★★★ off the map

1. **Make the LED blink on a timer as well as on the button, without a second
   thread.** Use `k_timer` or `k_work_delayable`.

   *Why it is interesting:* you now have two writers for one piece of state, which is
   the point where a global `bool` stops being enough. Decide what you would do about
   it.

2. **Read the `gpio_emul` driver source.** `$ZEPHYR_BASE/drivers/gpio/gpio_emul.c`,
   around `gpio_emul_pin_configure`.

   *Why it is interesting:* it explains something app 03's test suite has a long
   comment about, which is that configuring a pin can fire a callback you did not ask
   for.

## If you want to go further

- [Devicetree bindings](https://docs.zephyrproject.org/latest/build/dts/bindings.html) - where `gpio-leds` and `gpio-keys` are defined, and what properties they accept.
- [Input subsystem](https://docs.zephyrproject.org/latest/services/input/index.html) - the modern alternative to binding `gpio-keys` by hand, which the overlays here already half set up with `zephyr,code`.
- [Interrupts and callbacks](https://docs.zephyrproject.org/latest/kernel/services/interrupts.html) - what you are and are not allowed to do inside `button_pressed_cb`.
