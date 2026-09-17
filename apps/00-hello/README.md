# 00-hello

Two `printk()` calls and a board name. No devicetree node is bound, no driver
is enabled, and nothing can fail except your toolchain. That is the job: if
this builds and runs, the rest of the day is about testing rather than about
setup.

**What this opens:** the whole sequence. Everything after it adds exactly one
idea to the folder before.

## What to learn here

- What a minimal Zephyr application is made of: `CMakeLists.txt`, `prj.conf`,
  `src/`. Three files, and two of them are nearly empty.
- That `west build -b <board>` and `west build -b native_sim/native` are the
  same command with a different argument, and that the second one produces an
  executable you run like any other program.
- Where `CONFIG_BOARD_TARGET` comes from, and why the string it prints is not
  in any source file.
- That `prj.conf` is a list of Kconfig symbols, not a config file the app
  reads at run time.

## Layout

```
00-hello/
├── CMakeLists.txt          target_sources(app PRIVATE src/main.c)
├── prj.conf                CONFIG_PRINTK=y, and a comment about why that is all
├── boards/
│   └── native_sim_native.conf   puts the console on stdin/stdout
└── src/main.c              two printk() calls
```

## Run it

```bash
cd apps/00-hello
```

```bash
west build -b native_sim/native -p
```

```bash
./build/zephyr/zephyr.exe
```

For a board you own:

```bash
west build -b nrf5340dk/nrf5340/cpuapp -p && west flash
```

## Expected outcome

```
*** Booting Zephyr OS build v4.4.2 ***
Hello from the Zephyr testing workshop!
Board: native_sim/native
```

The banner comes from Zephyr, not from `main()`. On a board the third line
reads `nrf5340dk/nrf5340/cpuapp` instead, and `main.c` is unchanged, which is
the first small demonstration of the thing the whole day is about.

There are no tests in this folder, so `west twister -T apps/00-hello` finds
nothing and says so. That is correct, not a failure.

## References

| | |
|---|---|
| [Application development](https://docs.zephyrproject.org/latest/develop/application/index.html) | what the three files are and what west does with them |
| [native_sim](https://docs.zephyrproject.org/latest/boards/native/native_sim/doc/index.html) | the board that is your laptop, including the console options in `boards/native_sim_native.conf` |
| [Kconfig](https://docs.zephyrproject.org/latest/build/kconfig/index.html) | why `CONFIG_PRINTK=y` is a build-time decision and not a runtime one |
| [`docs/PRE-general-guide.md`](../../docs/PRE-general-guide.md) | setup, if this app did not run |
