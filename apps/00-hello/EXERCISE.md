# 00-hello - exercises

For when the build finished in twenty seconds and the room is still on step 3.

The answers are in the upstream Zephyr docs rather than in this repo. Finding
them is the exercise.

## ★ warm-up

1. **Predict, then check.** Before you run it, write down what
   `CONFIG_BOARD_TARGET` will print for `native_sim/native`. Then grep the
   build directory for where that string is defined.
   *Check:* you can name the generated file it comes from.

2. **Delete `CONFIG_PRINTK=y` from `prj.conf` and rebuild.** Does it still
   print? Work out why before you put it back.
   *Check:* you can say which other Kconfig symbol is selecting it.

3. **Build for a second board without owning one.** Try
   `west build -b qemu_cortex_m3 -p`, then `west build -t run`.
   *Check:* the same `main.c` prints a different board name.

4. **Make the build fail on purpose.** Remove `src/main.c` from
   `CMakeLists.txt` and read the error.
   *Check:* you can tell a CMake configure error from a link error by looking
   at the message, without scrolling up.

## ★★ go deeper

1. **Find the four largest symbols in the binary.** Run
   `west build -t rom_report` and then `west build -t ram_report`.
   *Check:* you can explain why a program with two `printk()` calls is not
   two instructions long.

2. **Add a second source file.** Split the board line into
   `src/banner.c` with a header, and wire it up in `CMakeLists.txt`.
   *Check:* it still builds for both `native_sim` and a real board with no
   other change.

3. **Run it under a debugger.** `gdb ./build/zephyr/zephyr.exe`, break on
   `main`, step.
   *Check:* you can see the Zephyr banner being printed before your
   breakpoint hits, and explain why.

4. **Give this app a test suite.** Add `tests/unit/` with a `testcase.yaml`, a
   `prj.conf` with `CONFIG_ZTEST=y` and one `ZTEST` that asserts something
   trivially true. Run `west twister -T apps/00-hello -p native_sim`.
   *Check:* twister reports 1 of 1 passed. You have now built the smallest
   possible version of everything in apps 02 onwards.

## ★★★ off the map

1. **Make `native_sim` print to a pseudo-terminal instead of your shell.**
   `boards/native_sim_native.conf` has the other option commented out. Switch
   it, rebuild, and get a terminal attached to the pty it announces.
   *Why it is interesting:* this is the configuration that lets a test harness
   drive the binary while you watch, and it is how the pytest suite in app 04
   talks to it.

2. **Work out what `-p` actually does, and when omitting it will burn you.**
   Build once without `-p`, change the board, and build again without `-p`.
   *Why it is interesting:* the failure is confusing, and recognising it saves
   you twenty minutes at least once a month.

## If you want to go further

- [West build, flash, debug](https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html) - every flag on the command you just ran, including `-p`, `-d` and `-t`.
- [Zephyr build system internals](https://docs.zephyrproject.org/latest/build/cmake/index.html) - what `find_package(Zephyr)` pulls in, and where the generated `autoconf.h` and `devicetree_generated.h` end up.
- [Twister](https://docs.zephyrproject.org/latest/develop/test/twister.html) - the test runner you will use all day, if you want to read ahead.
