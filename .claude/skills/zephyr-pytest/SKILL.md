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
    - command: "test_btn"
      expected: "Test: Triggering emulated button press"
```

Move to pytest when you hit its limit, and be able to name the limit you hit.

## Design the backdoor first

The test is only as good as what the device will tell it.

**Keep the backdoor out of the application.** `test_harness.c` is listed in the test's
`CMakeLists.txt` and nowhere else, so the shell command exists in the test image and no
other build. `src/main.cpp` stays byte-identical between them. A shell that drives your
hardware is a real attack surface, and the only version of "it is only in the test
build" that holds up is the one the build system enforces.

**Keep the lines you assert on stable.** A test that matches free-text `printk` output
breaks when someone rewords it. Once a suite grows past a few tests, a machine-readable
line such as `btn=done count=3` next to the human one is easier to assert on.

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

`apps/05-shell-pytest/tests/emul_button_toggle/pytest/test_gpio_toggle.py` has both
sides. `test_button_toggle_dut` uses `dut` alone:

```python
dut.write(b'test_btn\n')
lines = dut.readlines_until(regex=f'LED is now {expected}', timeout=2)
```

The opposite case also happens. With deferred logging on a real board, a line can
arrive after the prompt, so `exec_command()` returns without it. The shell tests only
wait when the line is not there yet, because an unconditional `readlines_until` times
out on native_sim, where the line was already read:

```python
lines = shell.exec_command('test_btn')
if f'LED is now {expected}' not in '\n'.join(lines):
    lines += shell._device.readlines_until(regex=f'LED is now {expected}', timeout=2)
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
def press(shell: Shell):
    def _press() -> list[str]:
        return shell.exec_command('test_btn')
    return _press
```

`conftest.py` is found by directory, not by import, so no test file imports anything
from it.

Register markers, or `--strict-markers` cannot help you and a typo is silent. App 05
does it in `pytest/pytest.ini`:

```ini
[pytest]
markers =
    slow: long-running tests
```

## Writing the tests

**Parametrize rather than loop.** A parametrized test reports one result per case, so
a red run says which input broke. A `for` loop stops at the first failure and tells you
nothing about the rest.

```python
@pytest.mark.parametrize('presses', [1, 2, 3], ids=['one', 'two', 'three'])
def test_led_after_presses(shell, presses):
    ...
```

Always pass `ids=`. Without it the cases are named `[3]` and a CI log tells you
nothing.

**Test rejection as well as acceptance.** A device that silently accepts nonsense is a
bug you find in the field.

**Use markers to split fast from slow.** Select them from the command line, as app 05
does:

```bash
west twister -T apps/05-shell-pytest/tests/emul_button_toggle -p native_sim --pytest-args="-m slow"
```

Or fix the selection in a scenario with `harness_config: pytest_args: ["-m", "not slow"]`,
so the quick half can run in a pre-commit hook.

**`xfail(strict=True)` records a known behaviour.** An unexpected pass then fails the
run, where a plain `xfail` passes either way.

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
