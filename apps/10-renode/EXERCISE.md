# 10-renode - exercises

## ★ warm-up

1. **Read the kernel's uptime from the shell.** Start the image with
   `renode-nrf-run build`, then type this at the `uart:~$` prompt:

   ```
   kernel uptime
   ```

   The kernel timer on the nRF52 series is driven by one of the chip's RTC peripherals,
   not by the Cortex-M SysTick. Check that the build selected it:

   ```bash
   grep -n "NRF_RTC_TIMER" build/zephyr/.config
   ```

   *Check:* the uptime in milliseconds is close to the last tick number times 1000, and
   you can say that the ticks advance only because Renode models that RTC.

2. **Find the pin behind `led0`.** Look the alias up in the generated devicetree:

   ```bash
   grep -n -A2 "led0: led_0" build/zephyr/zephyr.dts
   ```

   The `gpios` line reads `< &gpio0 0xd 0x1 >`. Find what the last number means:

   ```bash
   grep -n "GPIO_ACTIVE_LOW" $ZEPHYR_BASE/include/zephyr/dt-bindings/gpio/gpio.h
   ```

   *Check:* you can give the port and pin number in decimal, and say whether the pin
   is driven high or low when the tick line prints `led0 on`.

3. **Load an ELF that does not exist.** Point `run_nrf52.resc` at a wrong path:

   ```bash
   renode --console --disable-xwt -e "\$elf=@$PWD/build/zephyr/nothing.elf; include @run_nrf52.resc"
   ```

   *Check:* you can find the error line, say which line of `run_nrf52.resc` it comes
   from, and say whether any `uart0:` line appeared after it.

## ★★ go deeper

1. **Stop at `main()` in a debugger, with no probe.** In one terminal:

   ```bash
   renode-nrf-run build --gdb
   ```

   In a second terminal, from `apps/10-renode`:

   ```bash
   $ZEPHYR_SDK_INSTALL_DIR/gnu/arm-zephyr-eabi/bin/arm-zephyr-eabi-gdb build/zephyr/zephyr.elf -ex "target remote :3333" -ex "break main" -ex "continue"
   ```

   *Check:* GDB stops in `main()` before the first tick is printed, and `print led.pin`
   gives the same pin number you found in warm-up 2.

2. **Run another app's firmware in Renode.** From the repository root, build app 00 for
   this board:

   ```bash
   west build -b nrf52840dk/nrf52840 -p -s apps/00-hello -d build_nrf52
   ```

   ```bash
   renode-nrf-run build_nrf52
   ```

   *Check:* it prints `Board: nrf52840dk/nrf52840`, and you can say why app 00 needed no
   change at all to run here.

3. **List what Renode does not model.** Let `renode-nrf-run build` run for a few ticks,
   leave it, then count the warnings by peripheral:

   ```bash
   grep -o "\[WARNING\] [a-z0-9_]*:" build/renode.log | sort | uniq -c
   ```

   *Check:* you can name the peripheral with the most warnings, and read one of its
   warning lines to say which register write it did not handle.

4. **Try a board Renode has no machine for.** Build this app for the nRF5340 DK, then
   hand it to `renode-nrf-run`:

   ```bash
   west build -b nrf5340dk/nrf5340/cpuapp -p -d build_5340
   ```

   ```bash
   renode-nrf-run build_5340
   ```

   *Check:* it refuses to start, and you can name the two places it looked for a `.resc`
   before giving up.

## ★★★ off the map

1. **Give `led0` a model, so Renode logs the LED itself.** Write a
   `boards/nrf52840dk.resc` that builds the same machine and adds an LED connected to
   `gpio0` pin 13. `renode-nrf-run` picks that file up in place of its own.

   *Why it is interesting:* connecting a GPIO output to a model in the platform
   description is the same step you take to put a sensor or a second chip next to the
   SoC.

2. **Make `west build -t run_renode` work for this board.** Zephyr only creates that
   target when the board's `board.cmake` sets `RENODE_SCRIPT`, which you can see in
   `$ZEPHYR_BASE/cmake/emu/renode.cmake`. The nRF52840 DK's does not.

   *Why it is interesting:* you do not own the board definition, so there is more than
   one place the setting could go, and each has a different effect on the other apps.

## If you want to go further

- [Renode platform description format](https://renode.readthedocs.io/en/latest/advanced/platform_description_format.html) - how a `.repl` declares peripherals and connects GPIO lines, which ★★★ 1 needs.
- [Zephyr shell](https://docs.zephyrproject.org/latest/services/shell/index.html) - the `kernel` commands used in warm-up 1, and how to add a command of your own.
- [GPIO API](https://docs.zephyrproject.org/latest/hardware/peripherals/gpio.html) - `gpio_dt_spec`, the active-low flag from warm-up 2, and the `_dt` helpers `src/main.c` uses.
