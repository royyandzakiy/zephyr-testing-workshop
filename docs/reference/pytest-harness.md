# twister_harness reference

For someone writing a pytest suite that Twister runs against a device. Verified
against Zephyr v4.4.2, source under
`$ZEPHYR_BASE/scripts/pylib/pytest-twister-harness/src/twister_harness/`.

`apps/04-shell-pytest` and `apps/05-pytest-advanced` are the worked examples.

## Turning it on

`harness: pytest` in `testcase.yaml` is the whole opt-in. Twister builds the image,
starts the device, and runs pytest with its plugin loaded.

```yaml
tests:
  app05.shell.pytest.smoke:
    harness: pytest
    harness_config:
      pytest_root:
        - "pytest/test_smoke.py"
      pytest_args:
        - "-m"
        - "not slow"
```

| Key | Effect |
|---|---|
| `pytest_root` | which files or directories to collect. Defaults to `pytest/` |
| `pytest_args` | extra arguments, one list entry per token |
| `pytest_dut_scope` | becomes `--dut-scope=<value>`, which changes the fixture scope below |

## Fixtures

```python
from twister_harness import DeviceAdapter, Shell, MCUmgr
```

| Fixture | Scope | Is |
|---|---|---|
| `dut` | function by default | a launched `DeviceAdapter`. The device is running. |
| `unlaunched_dut` | function by default | the same object with logs attached but not started, for tests that want to control `launch()` |
| `shell` | function by default | a `Shell` wrapping `dut`, with the prompt already waited for |
| `mcumgr`, `mcumgr_ble` | function | MCUmgr helpers |
| `device_object` | session | the underlying adapter. `dut` wraps this. |
| `twister_harness_config` | session | what Twister passed in |

`dut`, `unlaunched_dut` and `shell` share one scope, resolved by `determine_scope()` in
`fixtures.py`. It returns `'function'` unless `--dut-scope` was passed, which is what
`pytest_dut_scope` sets.

**A session-scoped fixture cannot depend on `dut`.** pytest raises `ScopeMismatch` at
setup rather than at collection, so it surfaces when you run, not when you write.

## `Shell`

`shell.py`. Constructed by the fixture as `Shell(dut, timeout=20.0)`, with the prompt
taken from `CONFIG_SHELL_PROMPT_UART` in the build's `.config` if it is set, and
`uart:~$` otherwise.

```python
exec_command(command: str, timeout: float | None = None, print_output: bool = True) -> list[str]
wait_for_prompt(timeout: float | None = None) -> bool
get_filtered_output(command_lines: list[str]) -> list[str]
```

`exec_command` appends two newlines: one to run the command, one to get the next
prompt back, which is how it knows execution finished. It reads until the prompt regex
matches and returns everything it read.

`get_filtered_output` strips prompts and log lines out of that, which is worth using
when `CONFIG_LOG` is on and log output interleaves with your command's.

## `DeviceAdapter`

`device/device_adapter.py`. The raw interface underneath `Shell`.

```python
readline(connection_index=0, **kwargs) -> str
readlines(connection_index=0, **kwargs) -> list[str]
readlines_until(connection_index=0, **kwargs) -> list[str]
write(data: bytes, connection_index=0) -> None
clear_buffer() -> None
launch() / close() / connect(retry_s=0) / disconnect()
```

`readlines_until` takes `regex`, `timeout` and `print_output` as keyword arguments, and
raises `AssertionError` if the timeout expires before the pattern matches. It is the
replacement for `time.sleep()`, which is too short on a loaded runner and wasted
everywhere else.

## The thing that catches people

**`shell` and `dut` are one buffer, not two.** `Shell` wraps the same adapter the `dut`
fixture returns. Two consequences, and both look like timeouts:

| Symptom | Cause | Fix |
|---|---|---|
| the boot banner is gone, `readlines()` returns `[]` | the `shell` fixture calls `wait_for_prompt()` at setup, which reads the buffer dry | ask for `dut` only, do not request `shell` in that test |
| `readlines_until()` times out on a line the device definitely printed | `exec_command()` read to the prompt and consumed it | send the command with `dut.write(b'cmd\n')` instead |

```python
def test_dut_sees_boot_output(dut: DeviceAdapter):
    lines = dut.readlines_until(regex='GPIO Button .* Toggle started', timeout=5.0)
    assert lines


def test_dut_readlines_until(dut: DeviceAdapter):
    dut.readlines_until(regex='Ready. Press the button', timeout=5.0)
    dut.write(b'app btn 1\n')
    lines = dut.readlines_until(regex='Button pressed!', timeout=5.0)
    assert lines
```

Both of those were wrong in this repo until they were run. See
`apps/05-pytest-advanced/tests/shell_pytest/pytest/test_fixtures.py`.

## Device output that is easy to assert on

Free-text `printk` lines work for one test and break twenty the first time someone
rewords a log line. The harness in `apps/05-pytest-advanced` prints both:

```
Test: triggering 3 emulated button press(es)
btn=done count=3
```

The first is for a human on a serial console, the second is what the suite parses.
`parse_kv()` in that app's `conftest.py` turns every `key=value` on a line into a dict.

Also set `CONFIG_SHELL_VT100_COLORS=n`. Colour escapes are invisible to you and very
visible to `str.find()`.

## The cheaper option

`harness: shell` sends commands and checks the output contains a string, with no
Python at all:

```yaml
harness: shell
harness_config:
  shell_commands:
    - command: "app led"
      expected: "led=off"
```

Reach for pytest when you need to compute an expected value, parametrize, keep state
across commands, or talk to something outside the device.

## Running and debugging

```bash
west twister -T apps/05-pytest-advanced -p native_sim -O /tmp/tw --clobber-output
```

```bash
west twister -T apps/05-pytest-advanced -p native_sim --pytest-args=-v --pytest-args=--log-cli-level=DEBUG
```

The pytest traceback lands in `<outdir>/<platform>/.../<scenario>/twister_harness.log`.
`handler.log` beside it holds what the device actually printed. When an assertion is
about a line the device should have sent, read `handler.log` first and find out whether
it sent it at all.
