---
name: zephyr-pytest
description: Write tests that run off the device in Python and drive it from outside, using twister_harness, usually end to end over a shell backdoor. Covers scoping, the dut and shell fixtures, conftest.py, parametrize, markers, and the built-in shell harness as the cheaper alternative. Use this when a test needs to compute an expected value, keep state across commands, talk to a network or a file, or exercise the whole application rather than one module. If the test only needs to check C logic on the device, use zephyr-ztest instead.
---

# pytest

Tests where the test process and the device under test are two different programs. The
test runs on the host, the device runs its own image, and they talk over a UART.

## Scope it before you write it

Same three steps as the `zephyr-ztest` skill, and the first question is whether the
test belongs here at all.

### Does it belong off the device?

Reach for pytest when the test needs something the device does not have:

- an expected value that has to be **computed**, not hardcoded
- **state across several commands**, where the assertion is about the sequence
- **parametrization**, with one reported result per case
- the host's world: a network, a database, a file, a reference implementation
- the **whole application**, rather than one module

Stay in ztest when the answer is about C logic. A pytest that only checks arithmetic
has bought a serial port, a process boundary and several seconds per run, for nothing.

### Then pick how much to build

| Approach | Cost | Use when |
|---|---|---|
| `harness: shell` | no Python at all | commands and expected substrings are enough |
| `harness: pytest`, one file | a conftest and a test file | you need computation or parametrization |
| `harness: pytest`, split scenarios | several entries in `testcase.yaml` | the suite is slow enough that a fast subset earns its keep |

Start with `harness: shell`. It is three lines and it fails honestly:

```yaml
harness: shell
harness_config:
  shell_commands:
    - command: "app led"
      expected: "led=off"
```

Move to pytest when you hit its limit, and be able to name the limit you hit.

## Design the backdoor first

The test is only as good as what the device will tell it.

**Keep the backdoor out of the application.** `test_harness.c` is listed in the test's
`CMakeLists.txt` and nowhere else, so the shell command exists in the test image and no
other build. `src/main.c` stays byte-identical between them. A shell that drives your
hardware is a real attack surface, and the only version of "it is only in the test
build" that holds up is the one the build system enforces.

**Print machine-readable output alongside the human line.**

```
Test: triggering 3 emulated button press(es)
btn=done count=3
```

Asserting on free-text `printk` lines works for one test and breaks twenty the first
time someone rewords a log line. A `key=value` line and a `parse_kv()` helper in
`conftest.py` means assertions talk about `stats['presses']` instead of about wording.

**Add commands that report state without changing it.** A test that can only act and
then read the log is much weaker than one that can ask.

**Set `CONFIG_SHELL_VT100_COLORS=n`.** Colour escapes are invisible to you and very
visible to `str.find()`.

## The two fixtures, and the trap

`twister_harness` gives you `dut` and `shell`.

- **`shell`** sends a command, waits for the prompt, and returns the lines. Use it for
  almost everything.
- **`dut`** is the raw `DeviceAdapter` underneath. Use it when what you want is not a
  response to a command.

**They share one buffer.** `Shell` wraps the same adapter. This is the single most
expensive thing to learn the hard way, and it produces two failures that look like
timeouts:

| Symptom | Cause | Fix |
|---|---|---|
| the boot banner is gone, `readlines()` returns `[]` | `shell` drained the buffer waiting for its first prompt | ask for `dut` **only**, do not request `shell` in that test |
| `readlines_until()` times out on a line the device definitely printed | `exec_command()` read until the prompt and consumed it | write the command with `dut.write(b'cmd\n')` instead |

```python
def test_dut_sees_boot_output(dut: DeviceAdapter):
    lines = dut.readlines_until(regex='GPIO Button .* Toggle started', timeout=5.0)
    assert lines, 'no boot banner before the timeout'


def test_dut_readlines_until(dut: DeviceAdapter):
    dut.readlines_until(regex='Ready. Press the button', timeout=5.0)
    dut.write(b'app btn 1\n')
    lines = dut.readlines_until(regex='Button pressed!', timeout=5.0)
    assert lines, 'device never reported the press'
```

**`dut` is function-scoped.** A session-scoped fixture that depends on it raises
`ScopeMismatch` at setup, not at collection, so you find out when you run rather than
when you write. `pytest_dut_scope` in `harness_config` changes the scope if you
genuinely need it.

**Never use `time.sleep()` to wait for the device.** It is too short on a loaded CI
runner and wasted everywhere else. `readlines_until(regex=..., timeout=...)` is the
answer.

## conftest.py

This is where the suite's vocabulary lives. A fixture that wraps `shell` in a small
API is worth writing early, because when the command spelling changes you edit one
class instead of twenty tests.

```python
@pytest.fixture()
def app(shell: Shell):
    class App:
        def __init__(self, sh): self._sh = sh
        def press(self, n=1): return parse_kv(self._sh.exec_command(f'app btn {n}'))
        def led(self): return parse_kv(self._sh.exec_command('app led'))['led']
    a = App(shell)
    a.reset()          # every test starts from a known state
    return a
```

`conftest.py` is found by directory, not by import, so no test file imports anything
from it.

Register markers, or `--strict-markers` cannot help you and a typo is silent:

```python
def pytest_configure(config):
    config.addinivalue_line('markers', 'slow: takes more than a couple of seconds')
```

## Writing the tests

**Parametrize rather than loop.** A parametrized test reports one result per case, so
a red run says which input broke. A `for` loop stops at the first failure and tells you
nothing about the rest.

```python
@pytest.mark.parametrize('presses', [1, 2, 5, 20],
                         ids=['single', 'double', 'handful', 'max'])
def test_press_counter_matches(app, presses):
    ...
```

Always pass `ids=`. Without it the cases are named `[20]` and a CI log tells you
nothing.

**Test rejection as well as acceptance.** A device that silently accepts nonsense is a
bug you find in the field.

**Use markers to split fast from slow**, then select on them from `testcase.yaml`, so
the quick half can run in a pre-commit hook:

```yaml
app05.shell.pytest.smoke:
  harness: pytest
  harness_config:
    pytest_args: ["-m", "not slow"]
```

**`xfail(strict=True)` writes an assumption down.** An unexpected pass becomes a
failure, so the day the behaviour changes, something tells you. Without `strict` it
passes either way and nothing does.

**Teardown goes after a `yield`**, so it still runs when the test body fails. On real
hardware that is the difference between a failed test and a failed test plus a device
left in a strange state.

## Running it

```bash
west twister -T apps/<app> -p native_sim -O /tmp/tw --clobber-output
```

Debugging the Python side:

```bash
west twister ... --pytest-args=-v --pytest-args=--log-cli-level=DEBUG
```

The pytest traceback is in `<outdir>/<platform>/.../<scenario>/twister_harness.log`.
`handler.log` next to it has what the device actually said. When an assertion is about
a line the device should have printed, read `handler.log` first and find out whether it
printed it at all.

On hardware, see the `zephyr-build-run` skill. The ESP32-S3 needs `--flash-before` or
the harness reads a stale port and every test times out.

## Running pytest without twister

`apps/05-pytest-advanced/tests/pytest_raw/` does this against the native_sim binary
with `subprocess`. It is worth reading once, because it shows what the harness is
doing for you: spawning the process, a reader thread, a prompt parser and a timeout, in
about sixty lines, and it still only works against native_sim.

Do not write new suites this way. Use it to explain the harness.

## Before you call it done

1. **Run it in the container.** Python that imports is not Python that passes. Three
   tests in this repo were written, reviewed, and shipped with a "not yet run" note;
   all three were broken, and all three broke on fixture behaviour that no amount of
   reading would have shown.
2. **Make an assertion fail on purpose** and confirm the failure names the thing you
   expected.
3. **Check the counts.** `N of N executed test cases passed` in the summary, and the
   number matches how many tests you think you wrote. A scenario that passes with
   fewer cases than expected is a collection error, not a pass.
4. Report the real numbers.
