# 00-hello - exercises

## ★ warm-up

1. **Find where `CONFIG_BOARD_TARGET` gets its value.** It prints
   `native_sim/native`, and that string is in none of the files in this folder.
   Search the build directory for it:

   ```bash
   grep -rn "native_sim/native" build/zephyr/.config build/zephyr/include/generated/
   ```

   *Check:* you can name the two generated files that carry it, and say which of the
   two the C compiler actually reads.

2. **Delete `CONFIG_PRINTK=y` from `prj.conf` and rebuild.** It still prints. Find out
   why, starting with what the build decided:

   ```bash
   grep CONFIG_PRINTK build/zephyr/.config
   ```

   It is still `y`, so something inside Zephyr is switching it on without being asked.
   Find what:

   ```bash
   grep -rn "select PRINTK" $ZEPHYR_BASE
   ```

   *Check:* you can name the Kconfig symbol that forces `PRINTK` on, and you can say
   why `prj.conf` is not the place to look when you want to know if a symbol is
   enabled. Put the line back when you are done.

3. **Build for a second board without owning one.** QEMU boards run under west:

   ```bash
   west build -b qemu_cortex_m3 -p && west build -t run
   ```

   Quit QEMU with `Ctrl-A` then `x`.

   *Check:* the same `main.c` prints a different board name, and you did not edit a
   single source file to get there.

4. **Make the build fail on purpose.** In `CMakeLists.txt`, delete the `src/main.c`
   line from `target_sources()`, then build again.

   Read the output from the top, not from the bottom. Find the first line that reports
   an error, and note which tool printed it: `cmake`, `gcc`, or `ld`.

   *Check:* you can say which of the three failed, and therefore whether the problem is
   in the build files or in the C code. That is the habit worth taking away, because
   most of the time you will be reading somebody else's build log. Put the line back
   when you are done.

## ★★ go deeper

1. **See where the bytes went.** Run both size reports:

   ```bash
   west build -t rom_report
   ```

   ```bash
   west build -t ram_report
   ```

   *Check:* you can name the largest few entries, and say why a program with two
   `printk()` calls still carries a kernel, a scheduler and a console driver with it.

2. **Add a second source file.** Move the board line into `src/banner.c` with a header,
   and wire it up in `CMakeLists.txt`.

   *Check:* it still builds for both `native_sim` and a real board, with no other
   change anywhere.

3. **Run it under a debugger.**

   ```bash
   gdb ./build/zephyr/zephyr.exe
   ```

   Break on `main`, run, and step.

   *Check:* the Zephyr banner is already printed before your breakpoint hits, and you
   can say which part of Zephyr printed it and when.

4. **Give this app a test suite.** Add `tests/unit/` with a `testcase.yaml`, a
   `prj.conf` containing `CONFIG_ZTEST=y`, and one `ZTEST` that asserts something
   trivially true. Then:

   ```bash
   west twister -T apps/00-hello -p native_sim
   ```

   *Check:* twister reports 1 of 1 passed. Compare your `testcase.yaml` against
   `apps/02-ztest/tests/unit/testcase.yaml` afterwards.

## ★★★ off the map

1. **Make native_sim print to a pseudo-terminal instead of your shell.**
   `boards/native_sim_native.conf` has the other option commented out. Switch it,
   rebuild, and attach a terminal to the pty the binary announces.
   [`docs/reference/native-sim.md`](../../docs/reference/native-sim.md) has the commands.

   *Why it is interesting:* this is the configuration that lets a test harness drive
   the binary while you watch it, and it is how the pytest suite in app 04 talks to the
   device.

2. **Work out what `-p` does, and what happens when you leave it out.** Build once
   without `-p`, change the board, then build again without `-p`.

   *Why it is interesting:* the error you get does not mention the build directory at
   all, so there is nothing in the message that points you at the real cause. Seeing it
   once is how you recognise it the second time.

## If you want to go further

- [West build, flash, debug](https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html) - every flag on the command you just ran, including `-p`, `-d` and `-t`.
- [Zephyr build system internals](https://docs.zephyrproject.org/latest/build/cmake/index.html) - what `find_package(Zephyr)` pulls in, and where the generated `autoconf.h` and `devicetree_generated.h` end up.
- [Twister](https://docs.zephyrproject.org/latest/develop/test/twister.html) - the test runner every app from 02 onwards uses, if you want to read ahead.
