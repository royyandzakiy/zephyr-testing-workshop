# Glossary

The vocabulary the rest of these docs assumes. Zephyr-specific unless noted.

**app.overlay** A devicetree overlay that applies to whatever is being built, found
automatically next to `CMakeLists.txt`. A test suite has its own, which is how it
reroutes an alias without the application knowing.

**alias** A devicetree name that points at a node, so code asks for `sw0` rather than
for a controller and a pin number. Changing what an alias points at is a build-time
decision that needs no change in C.

**board qualifier** The part after the board name: `native_sim/native`,
`nrf5340dk/nrf5340/cpuapp`. Names the SoC and core, and is required on multi-core
parts.

**DUT** Device under test. Also the name of the pytest fixture that hands you one.

**devicetree** A description of the hardware the build should assume: which
peripherals exist, on which pins, at which addresses. Compiled at build time into
macros. It does not exist at run time.

**emul, emulated driver** An ordinary Zephyr driver that fakes a peripheral instead of
driving one, gated on a devicetree node such as `zephyr,gpio-emul`. Not a `native_sim`
feature; it builds for a real board too.

**FFF** Fake Function Framework. Generates stand-in definitions for functions your
module calls, so the test can decide what they return and see how they were called.
Vendored in Zephyr as `<zephyr/fff.h>`.

**fixture** In ztest, a struct handed to each test in a suite, created by `setup`. In
pytest, a function whose return value is passed to any test that names it. The two are
unrelated beyond the name.

**harness** How Twister turns device output into a result. `ztest`, `gtest`,
`console`, `shell` and `pytest` are the ones used here.

**HIL** Hardware in the loop. Tests that run against real hardware, usually attached to
a CI runner.

**Kconfig** The configuration system that decides which parts of Zephyr get compiled
in. You write `prj.conf`; the build merges it with Zephyr's defaults into
`build/zephyr/.config`, which is the file that actually decided.

**native_sim** A Zephyr board that compiles the kernel and your application into an
ordinary program for your own machine. Not an emulator: there is no simulated CPU.

**overlay** A devicetree fragment merged on top of a board's `.dts`. Adds or changes
nodes without editing the board.

**prj.conf** The application's Kconfig fragment. One input among several, not the
final answer.

**runner** The tool `west flash` drives to get an image onto a board: `nrfutil`,
`esp32`, `pyocd`, `jlink`, `openocd`.

**scenario** One entry under `tests:` in a `testcase.yaml`, with a name like
`app07.feeder.ztest`. What `--test` matches on and what the summary counts.

**seam** A place where a test can substitute something for the real thing. In this
repo, either the devicetree pointing at an emulated controller, or a port header whose
implementation the test does not link.

**twister** Zephyr's test runner. Walks the tree for `testcase.yaml`, builds each
scenario, runs it, and decides pass or fail through a harness.

**twister_harness** The pytest plugin Twister loads, which supplies the `dut` and
`shell` fixtures.

**west** Zephyr's command line tool. Manages the repositories, wraps CMake for
`west build`, and calls the runners for `west flash`. Not a compiler and not a build
system; it drives both.

**ztest** Zephyr's own test framework. Tests are C, compiled into their own Zephyr
image, and run on the target.
