---
name: zephyr-ztest
description: Write tests in C that run on the device, using ztest. Covers scoping the test before writing it, pure unit tests, integration tests across a few modules, emulated-driver tests, and FFF fakes. Use this whenever asked to add or fix tests for C code in this repo, to test a module or a driver, or to raise coverage. Start here for any testing request and only hand off to zephyr-pytest if the scoping step says the test belongs off the device.
---

# ztest

Tests that run **on** the device, in C, compiled into their own Zephyr image. If the
test needs to drive the device from outside, stop and use the `zephyr-pytest` skill
instead. The scoping step below decides which.

## Scope it before you write it

Most bad test suites are correctly written tests at the wrong level. Do these three
steps in order, and say out loud which level you landed on before writing anything.

### 1. Understand the logic

Read the module. Answer, in one sentence each:

- What decision does this code make, and what does it depend on?
- Which inputs change the answer? Which are just passed through?
- Where can it be wrong in a way that still looks plausible? That is the test worth
  writing. A wrong answer that looks obviously wrong gets caught by the first person
  who runs it.

### 2. Pick the level

| Level | Links | Answers | Where |
|---|---|---|---|
| **unit** | one module, nothing else | is the logic right | `tests/unit/` or `tests/ztest/` |
| **integration** | a few modules together, or your module plus a real driver over an emulated bus | do these pieces agree | `tests/emul/` |
| **fake-dependency** | the module, with its dependencies replaced by FFF fakes | what does it do when a dependency misbehaves | `tests/fff/` |
| **end to end** | the whole app, driven over a shell or a protocol | does the product work | not ztest, see `zephyr-pytest` |

Rules of thumb:

- If the module has **no dependencies**, it is a unit test and nothing else. Do not
  add an emulator to test arithmetic. `apps/07-unit-conventions` is the model.
- If you are asking **"does my code drive this chip correctly"**, that is an emulated
  integration test. `apps/06-sensor`.
- If you are asking **"what happens when the chip returns `-EIO` three times"**, that
  is a fake, not an emulator. Arranging misbehaviour in an emulator is painful; in a
  fake it is one line. `apps/08-fff-mocks`.
- If the answer involves a serial port, a network, or a file on the host, it is not a
  ztest.

A module with no seam cannot be unit tested. If you find that, say so and propose the
smallest extraction that would fix it, rather than writing an emulated test to work
around it. `apps/01-blinky` to `apps/02-ztest` is that extraction, done.

### 3. Decide what "enough" is

Write the tests that would catch a real defect. Stop there.

Worth a test:

- boundaries, and one step either side of them
- the empty case and the full case
- anything that wraps: time of day, a millisecond tick, a ring buffer index
- anything with an accumulator wider than its elements
- the error path, and what state is left behind after it
- ordering, when a caller could reasonably do things in another order

Not worth a test:

- a getter that returns a field
- the same boundary three times with different numbers
- a case you cannot describe a failure for. If you cannot say what would break, the
  test is documentation of the implementation, and it will need editing every time the
  implementation changes.

Prefer six tests that each name a distinct behaviour over twenty that overlap.

## The mechanics

A test suite is a complete Zephyr application. Its `CMakeLists.txt` links the module
under test and **not** its dependencies. What it leaves out is as deliberate as what
it includes.

```cmake
cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(feeder_ztest)

target_sources(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../src/feeder.c
)
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../../src)
```

`prj.conf` enables only what the test needs. A test image that pulls in `CONFIG_GPIO`
it never uses is a slower build and a wider blast radius.

```yaml
# testcase.yaml
tests:
  app07.feeder.ztest:
    platform_allow:
      - native_sim
    tags:
      - feeder
      - unit
```

Scenario names are `appNN.<domain>.<kind>`.

## Suite hooks

```c
ZTEST_SUITE(name, predicate, setup, before, after, teardown);
```

| Hook | Runs | For |
|---|---|---|
| `predicate` | once, on the device, before anything | skipping the whole suite when this build lacks something |
| `setup` | once, returns the fixture pointer | allocating the fixture |
| `before` | before every test | **resetting anything a test mutates** |
| `after` | after every test, pass or fail | releasing what `before` or the test acquired |
| `teardown` | once | releasing what `setup` acquired |

**The rule that catches most real bugs: anything a test changes gets reset in
`before`, not in `setup`.** `setup` runs once, so state one test dirtied is still
dirty for the next.

**ztest runs tests within a suite alphabetically, not in source order.** Verify this
when reasoning about which test inherits what. Renaming a test changes the order.

`predicate` is evaluated at run time on the device. `platform_allow` is evaluated by
twister before anything is built. Use `platform_allow` for "this board cannot build
it" and a predicate for "this build does not have the thing I need".

## Writing the tests

`apps/07-unit-conventions/tests/ztest/src/main.c` is the reference. The conventions,
each of which exists because of a specific failure:

**One behaviour per test.** Four assertions about four different things report one
failure and hide three. Two assertions about one contract, such as "returns false AND
leaves `*out` alone", belong together.

**A name that states the behaviour.**
`test_next_feed_on_an_empty_schedule_fails` tells you what broke from the summary line
alone. `test_next_2` makes you open the file.

**Arrange, Act, Assert, with a blank line between.** Once every test has that shape
you scan a suite instead of reading it.

**Assertion messages that print the offending value.** These cost the same to type:

```c
zassert_equal(got, want);
zassert_equal(got, want, "at minute %u got %u, want %u", now, got, want);
```

The second one ends the investigation. The first one starts it.

**A fixture only when setup is long.** A struct on the stack is cheaper and cannot
leak between tests. When you do use one, the type must be named
`struct <suite>_fixture`, because `ZTEST_F` builds that name by token pasting, and a
mismatch produces an error pointing at the macro rather than at your typo.

**A table when the bodies would be copy-paste.** ztest has no parametrization, so a
table plus a loop is the substitute, and it costs you something real: the whole table
is one result and it stops at the first failure. Name each row in the message to claw
most of that back.

```c
static const struct { const char *name; uint16_t now; uint16_t want; } cases[] = {
    {"before the first feed", 300, 60},
    {"standing exactly on a feed", 360, 0},
};
for (size_t i = 0; i < ARRAY_SIZE(cases); i++) {
    zassert_equal(got, cases[i].want, "case %zu (%s): at minute %u got %u, want %u",
                  i, cases[i].name, cases[i].now, got, cases[i].want);
}
```

A case that is about something else, rather than one more row of the same thing, gets
its own test with a name that says what it is about.

**`ztest_test_skip()` reports SKIP, not PASS.** A suite that goes green because it did
nothing is the most expensive kind of green.

## Emulated-driver tests

When the question is whether your code drives a real driver correctly, put the fake
underneath the driver rather than replacing it. See the `zephyr-build-run` skill for
the three wiring arrangements and for what `native_sim` already provides.

The assertions to prefer, from `apps/06-sensor/tests/emul`:

- properties rather than values: in range, monotonic in the input, one channel not
  bleeding into another
- exactly one characterization test with literal numbers, labelled as such

Pinning expected engineering values everywhere means reimplementing the driver's math
in the test, and then the test only checks your arithmetic against itself.

## FFF fakes

When the question is how the module behaves while a dependency misbehaves. The whole
mechanism is the linker: leave the real implementation out of `target_sources`, and
`FAKE_VALUE_FUNC` becomes the definition of that symbol.

```c
#include <zephyr/fff.h>          /* vendored in Zephyr, nothing to add to west.yml */
DEFINE_FFF_GLOBALS;

FAKE_VALUE_FUNC(int, auger_run, uint16_t);

static void fff_before(void *f) {
    ARG_UNUSED(f);
    RESET_FAKE(auger_run);
    FFF_RESET_HISTORY();
}
```

`apps/08-fff-mocks` is the worked example. With more than two or three fakes, list them
once in a `FFF_FAKES_LIST(FAKE)` macro and loop `RESET_FAKE` over it. The fake you
forget to reset is the one that makes a test pass only when the whole suite runs in
order.

What a fake records: `call_count`, `arg0_val`, `arg0_history[]`, `return_val`,
`return_val_seq` via `SET_RETURN_SEQ`, and `custom_fake` when the function
communicates through an out-parameter. `call_count` increments **before** `custom_fake`
runs, so inside the body it is already 1 on the first call.

The assertion an emulator cannot make is "nothing ever tried to change it". An LED can
only tell you its final state.

This only works if your module calls **your** function. Zephyr's driver APIs are
`static inline` over an API struct, so there is no symbol to replace. If you find
yourself wanting to fake `gpio_pin_set_dt()` or `sensor_sample_fetch()`, the answer is a
port header, not a linker trick.

## Before you call it done

1. **Verify the expected values on the host first.** For pure C with no Zephyr
   headers, compile the module with `gcc -std=c11 -Wall -Wextra` and drive every table
   row from a small harness in the scratchpad. Numbers you reasoned about are not
   numbers you verified.
2. **Then build and run it in the container.** The host says nothing about Kconfig,
   ztest ordering, or the harness. See the `zephyr-build-run` skill.
3. **Prove the test can fail.** Break the thing it is supposed to catch and confirm
   which assertion fires. A test never seen red is a test you are guessing about.
4. **Prove the suite is order-independent.** Remove the `before` hook and confirm
   something fails; that tells you the hook is load-bearing. Put it back.
5. Report the real numbers from the run, not an impression.
