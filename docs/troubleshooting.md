# Troubleshooting

Keyed by what you are looking at, since that is what you can search for. For a first
build that never worked at all, start with [`setup/first-run.md`](setup/first-run.md).

## Build

### `fatal error: <header>: No such file or directory`, on a `CONFIG_` you did set

Kconfig dropped the symbol because its `depends on` was not met, and said nothing.
What `prj.conf` asks for and what the build settled on are two different things. Look
at the second one:

```bash
grep <SYMBOL> build/zephyr/.config
```

If it is absent, find what it depends on:

```bash
grep -rn -A5 "^config <SYMBOL>" $ZEPHYR_BASE
```

A real case from this repo: `CONFIG_GLIBCXX_LIBCPP` depends on
`NEWLIB_LIBC || PICOLIBC`, and native_sim uses neither, so it was silently discarded
and the C++ build fell back to the minimal library with no `<concepts>`.
`CONFIG_REQUIRES_FULL_LIBCPP` is the one that works on a native build.

### A devicetree error naming `DT_ALIAS` or a node label

The overlay that defines it was not picked up. Discovery is relative to `-s`, so
building the wrong directory is the usual cause. See
[`reference/boards.md`](reference/boards.md#overlay-auto-discovery).

### Errors that do not mention the build directory but make no sense

A stale build directory, usually after changing board. Rebuild pristine:

```bash
west build -b <board> -p always
```

### Every `<zephyr/...>` include is underlined in the editor

The code compiles, but clangd found no compilation database for that file.

clangd resolves this per file. It walks up from the source and checks each parent for
`compile_commands.json`, and for a `build/` directory beside it. **Only a directory
named exactly `build` counts**, so a build made with `-d build_native` or `-d build_conv`
is not found.

Build the app you are editing once, with no `-d`:

```bash
cd apps/06-sensor && west build -b native_sim/native
```

To see what clangd actually resolved, including the full command line it would use:

```bash
clangd --check=apps/06-sensor/src/main.c
```

It prints either `Loaded compilation database from ...` or
`Failed to find compilation database for ...`.

Two related cases:

- **The test suites** under `apps/<app>/tests/` are separate Zephyr applications, and
  the READMEs build them with `-d build_conv`, `-d build_gt` and so on. That is correct.
  The editor follows the app's own `build/`, not those, so editing a test file wants a
  plain build of the suite directory too.
- **Wrong `CONFIG_` values in completion** means the database is from another board. The
  flags and `autoconf.h` come from whichever board that `build/` was last made for.
  Rebuild it for the board you care about.

Pinning `CompilationDatabase` in `.clangd`, per repo or per app, also works. It points
at one directory, so it goes stale as soon as you build for another board.

### `undefined reference`, or `duplicate symbol`, in a test build

The test's `CMakeLists.txt` links the wrong set. A test that fakes a dependency has to
leave the real implementation out, and a test that needs the real one has to include
it. `apps/08-fff-mocks/tests/fff/CMakeLists.txt` is the example of deliberately
leaving files out.

## Twister

### `OSError: [Errno 39] Directory not empty: 'modules'`, before anything builds

Twister rotating `twister-out/` into `twister-out.1` at startup, which fails on the
Windows bind mount. Give it another output directory:

```bash
west twister -T apps/ -p native_sim -O /tmp/tw --clobber-output
```

### A scenario failed and the summary does not say why

The summary line only carries counts. The reason is in the logs under
`<outdir>/<platform>/.../<scenario>/`:

| File | Holds |
|---|---|
| `build.log` | compile and link errors |
| `device.log` | flash output, runner errors |
| `handler.log` | everything the device printed |
| `twister_harness.log` | the pytest side, including tracebacks |

Per-case detail is in `twister.json`:

```bash
python3 -c "
import json; d=json.load(open('/tmp/tw/twister.json'))
for t in d['testsuites']:
    if t.get('status') not in ('passed','filtered','not run'):
        print(t['status'].upper(), t['name'], t.get('reason',''))
"
```

### A suite passes with fewer test cases than you wrote

Something failed to collect, and the cases that were collected all passed. Check the
count in the summary line against how many tests you think exist.

## ztest

### A test passes alone and fails as part of the suite

State leaking between tests. Something a test changes is not being reset in `before`.
`setup` runs once, so it cannot do this job.

**ztest runs tests within a suite alphabetically, not in source order.** Renaming a
test changes which one inherits what. Verify the order from the output rather than
reasoning about the file.

### A suite reports PASS but you expected SKIP

`ztest_test_skip()` reports SKIP, and a suite predicate returning false skips the
whole suite. A test that ran and asserted nothing reports PASS, which is the third
case and looks identical to a test that checked something.

### The compiler error points at `ZTEST_F` rather than at your code

The fixture type has to be named `struct <suite>_fixture`. `ZTEST_F` builds that name
by token pasting, so a mismatch surfaces inside the macro.

## pytest

### `ScopeMismatch` at setup

A session-scoped fixture depends on `dut`, which is function-scoped. Either make the
dependent fixture function-scoped, or set `pytest_dut_scope: session` in
`harness_config`.

### `readlines_until` times out on a line the device definitely printed

`shell` and `dut` share one buffer. `exec_command()` reads to the prompt and consumes
the line first. Send the command with `dut.write(b'cmd\n')` instead. See
[`reference/pytest-harness.md`](reference/pytest-harness.md).

### `readlines()` returns `[]` when looking for the boot banner

Same cause. The `shell` fixture calls `wait_for_prompt()` at setup and drains the
buffer. Ask for `dut` only in that test.

### Assertions fail on output that looks correct

`CONFIG_SHELL_VT100_COLORS=y` is wrapping it in escape codes. Set it to `n`.

### A marker filter selects nothing, or a typo passes silently

Markers have to be registered, in `conftest.py` via `config.addinivalue_line` or in
`pytest.ini`. Add `--strict-markers` so an unregistered marker is an error instead of
a warning.

## Hardware

### The harness reads nothing on an ESP32-S3

Missing `--flash-before`. Native USB re-enumerates on reset, so a port opened before
flashing goes stale. J-Link and ST-LINK survive the reset and do not need it.

### A port number in the docs does not match your machine

They depend on enumeration order and on what else is plugged in.

```bash
ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null
```

On the ESP32-S3, `ttyACM0` and `ttyUSB0` are different endpoints, not two names for
one thing.

### `Target type stm32g474retx not recognized`

The pyocd pack is not installed:

```bash
pyocd pack install stm32g474retx
```

### Port busy

A monitor is still attached. Close it before flashing or running Twister.

### SIGSEGV on the first GPIO write in an emulated test

An alias was not rerouted and fell through to a controller this build never enabled.
A test-only overlay has to move **every** alias the app binds. Rerouting `sw0` and
leaving `led0` produces exactly this.
