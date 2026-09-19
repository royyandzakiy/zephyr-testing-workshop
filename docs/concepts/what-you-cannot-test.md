# What these tests cannot tell you

For someone whose suite is green and who is deciding how much that is worth. The
companion to [`testing-levels.md`](testing-levels.md).

Every level in this repo is blind to something. Knowing the list is what stops a green
run from meaning more than it does, and it is the argument for whatever hardware
testing you still do.

## What `gpio_emul` does not model

It is a variable with callbacks attached. It gets pin levels, interrupt edges and
pull-ups right, and knows nothing about:

- drive strength and slew rate
- contact bounce
- current limits, and what happens past them
- two things driving the same net
- capacitance, rise time, anything analog
- what a pin does during reset, or before `main()`

Source is short and worth reading: `$ZEPHYR_BASE/drivers/gpio/gpio_emul.c`.

## What a bus emulator does not model

`i2c_emul` and friends deliver bytes to a function you wrote. They do not model:

- clock stretching, arbitration, or a second master
- a device that NAKs intermittently, unless you write that in
- bus capacitance, pull-up sizing, rise time
- a device that is simply not soldered on
- the wrong address, because the fake answers at whatever address you gave it

`apps/06-sensor` proves the app talks to a BME280 correctly. It cannot tell you there
is a BME280 on the board.

## What `native_sim` does not model

It is native code in a normal process, not a simulated CPU.

- **Timing.** `k_sleep()` is honoured against the host clock. Nothing tells you whether
  an ISR meets a deadline on a Cortex-M.
- **Concurrency as it will really interleave.** The scheduler runs, but on different
  hardware with a different memory model.
- **The instruction set.** Compiled for x86 with different integer promotion,
  alignment and word size. `native_sim/native` is 32-bit, which catches some of this;
  a 64-bit build catches less.
- **Memory limits.** No flash or RAM ceiling, so nothing here tells you it fits.
- **Faults.** No MPU, no bus fault, no hard fault handler doing what it would.

## What a fake does not tell you

Every test in `apps/08-fff-mocks` still passes with the real implementation deleted.
That is the trade: you pinned the module and pinned nothing about the thing behind it.

The same applies to gmock in `apps/09-gtest-gmock`.

## What no test in this repo covers

- power draw, sleep current, wake latency
- EMC, ESD, anything about the physical product
- analog accuracy, ADC noise, reference drift
- flash wear, brownout, unexpected power loss mid-write
- anything about the radio: range, interference, coexistence
- the bootloader, secure boot, and what happens to a half-written update
- a chip that is out of spec, counterfeit, or damaged

## What to do about it

The honest version is that a green suite means the things you wrote tests for still
work. It is not a statement about the product.

Useful habits:

- When adding a test, write down what would have to break for it to fail. If nothing
  plausible, it is documenting the implementation rather than protecting it.
- Keep the list of what is not covered somewhere visible, so the gap is a decision
  rather than an oversight.
- `apps/06-sensor` carries a UBSAN finding inside Zephyr's own BME280 driver, left in
  on purpose. Sanitizers see a class of problem that assertions do not, and running
  them occasionally is cheap on `native_sim`.

## In this repo

| Blind spot | Where it would bite |
|---|---|
| `gpio_emul` electrical behaviour | `03-emul-gpio`, `04-shell-pytest` |
| bus behaviour and whether the part exists | `06-sensor` |
| the real implementation behind a fake | `08-fff-mocks`, `09-gtest-gmock` |
| timing, memory, faults | every `native_sim` run |
