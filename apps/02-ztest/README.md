# 02-ztest

Same behaviour as `01-blinky`, with the decisions pulled out into
`blink_logic.c` and a ztest suite pointed at them. The first green test of the
day.

**What changed since `01-blinky`:**

- `src/main.c` shrank to a `main()` that calls `blinky_init()`.
- `src/blink_logic.{c,h}` is new. Two pure functions, no Zephyr headers.
- `src/blinky.{c,h}` is new. All the driver code that used to be in `main.c`.
- `tests/unit/` is new.

## What to learn here

- What a seam is, concretely: `blink_logic_toggle()` takes a bool and returns
  a bool, so a test needs no board, no driver and no devicetree to call it.
- That a ztest suite is just another Zephyr application, with its own
  `CMakeLists.txt` and `prj.conf`, that happens to link
  `CONFIG_ZTEST=y` and some of the app's source files.
- What `tests/unit/CMakeLists.txt` links, and more importantly what it does
  not: `blinky.c` is absent, so no GPIO driver is pulled in at all.
- How twister finds this without a registry: it walks the tree looking for
  `testcase.yaml`, which is why tests live next to the app they test.
- What `zassert_str_equal` buys you over `zassert_equal` on two `char *`.

## Layout

```
02-ztest/
├── src/
│   ├── main.c              calls blinky_init(), then sleeps
│   ├── blinky.{c,h}        GPIO, callback, devicetree. Not linked by the test.
│   └── blink_logic.{c,h}   THE SEAM. Pure functions, zero dependencies.
└── tests/
    └── unit/
        ├── CMakeLists.txt  links src/main.c of the TEST plus blink_logic.c
        ├── prj.conf        CONFIG_ZTEST=y
        ├── testcase.yaml   scenario app02.blink.logic
        └── src/main.c      ZTEST_SUITE + three ZTESTs
```

## Run it

```bash
cd apps/02-ztest
```

```bash
west build -b native_sim/native -p && ./build/zephyr/zephyr.exe
```

The test suite:

```bash
west twister -T apps/02-ztest -p native_sim
```

Or build and run it by hand, which is worth doing once so twister stops
looking like magic:

```bash
west build -b native_sim/native -p -s apps/02-ztest/tests/unit -d build_unit && ./build_unit/zephyr/zephyr.exe
```

## Expected outcome

Twister:

```
INFO    - 1 test scenarios (1 configurations) selected
...
INFO    - 1 of 1 executed test configurations passed (100.00%)
```

The scenario is `app02.blink.logic`. Running the test binary directly shows
the three tests by name, plus the `TC_PRINT` lines from
`test_five_presses_from_off`:

```
Running TESTSUITE blink_logic
===================================================================
START - test_five_presses_from_off
press 1: LED is now ON
press 2: LED is now OFF
...
 PASS - test_five_presses_from_off
```

The application itself behaves exactly as `01-blinky` did, and still does
nothing on `native_sim`, because nothing can press the emulated button yet.

## References

| | |
|---|---|
| [ztest](https://docs.zephyrproject.org/latest/develop/test/ztest.html) | `ZTEST_SUITE`, `ZTEST`, and the whole `zassert_*` family |
| [Twister](https://docs.zephyrproject.org/latest/develop/test/twister.html) | `testcase.yaml` keys, especially `platform_allow` and `tags` |
| [`docs/NOTES-testing.md`](../../docs/NOTES-testing.md) | the local notes on both of the above |
| [apps/07-unit-conventions](../07-unit-conventions) | fixtures, table-driven cases and naming, once you want more than three tests |
