# twister_harness

`twister_harness` is the pytest plugin Twister loads when a scenario declares
`harness: pytest`. It builds the image, starts the device, and hands your test
functions a connection to it through fixtures. Verified against Zephyr v4.4.2, source
under `$ZEPHYR_BASE/scripts/pylib/pytest-twister-harness/src/twister_harness/`.

`apps/04-shell-pytest` and `apps/05-pytest-advanced` are the worked examples in this
repo.

## Turning it on

`harness: pytest` in `testcase.yaml` is the whole opt-in.

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
| `unlaunched_dut` | function by default | the same object with logs attached but not started, for tests that control `launch()` themselves |
| `shell` | function by default | a `Shell` wrapping `dut`, with the prompt already waited for |
| `mcumgr`, `mcumgr_ble` | function | MCUmgr helpers |
| `device_object` | session | the underlying adapter. `dut` wraps this. |
| `twister_harness_config` | session | what Twister passed in |

`dut`, `unlaunched_dut` and `shell` share one scope, resolved by `determine_scope()` in
`fixtures.py`. It returns `'function'` unless `--dut-scope` was passed, which is what
`pytest_dut_scope` sets.

A session-scoped fixture cannot depend on `dut`. pytest raises `ScopeMismatch` at setup
rather than at collection, so it will show up when you run, not when you write.

## `Shell`

`shell.py`. The fixture constructs it as `Shell(dut, timeout=20.0)`, with the prompt
taken from `CONFIG_SHELL_PROMPT_UART` in the build's `.config` if it is set, and
`uart:~$` otherwise.

```python
exec_command(command: str, timeout: float | None = None, print_output: bool = True) -> list[str]
wait_for_prompt(timeout: float | None = None) -> bool
get_filtered_output(command_lines: list[str]) -> list[str]
```

`exec_command` appends two newlines, one to run the command and one to get the next
prompt back, which is how it knows execution finished. It reads until the prompt regex
matches and returns everything it read.

`get_filtered_output` strips prompts and log lines out of that. With `CONFIG_LOG` on,
log output interleaves with your command's output, and this is what separates them.

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
raises `AssertionError` if the timeout expires before the pattern matches. It waits on
the line you care about, so a test does not need `time.sleep()` to guess how long the
device will take.

## `shell` and `dut` are one buffer

`Shell` wraps the same adapter the `dut` fixture returns, so anything one of them reads
is gone for the other. Two things follow from that, and both present as timeouts.

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

Both are in `apps/05-pytest-advanced/tests/shell_pytest/pytest/test_fixtures.py`.

## Device output a test can parse

A test that matches on free-text `printk` lines breaks whenever someone rewords the
log. The harness in `apps/05-pytest-advanced` prints two lines for that reason:

```
Test: triggering 3 emulated button press(es)
btn=done count=3
```

The first is for a human on a serial console, the second is what the suite parses.
`parse_kv()` in that app's `conftest.py` turns every `key=value` on a line into a dict.

Set `CONFIG_SHELL_VT100_COLORS=n` as well. Colour escapes do not show up in a terminal
but they are still in the bytes `str.find()` is searching.

## `harness: shell`

`harness: shell` sends commands and checks the output contains a string, with no Python
involved:

```yaml
harness: shell
harness_config:
  shell_commands:
    - command: "app led"
      expected: "led=off"
```

pytest is the option that adds computing an expected value, parametrizing, keeping
state across commands, and talking to something outside the device.

## Running and debugging

```bash
west twister -T apps/05-pytest-advanced -p native_sim -O /tmp/tw --clobber-output
```

```bash
west twister -T apps/05-pytest-advanced -p native_sim --pytest-args=-v --pytest-args=--log-cli-level=DEBUG
```

The pytest traceback lands in `<outdir>/<platform>/.../<scenario>/twister_harness.log`.
`handler.log` beside it holds what the device printed. When an assertion is about a line
the device should have sent, `handler.log` is where you find out whether it sent it at
all.

## References

| | |
|---|---|
| [Twister pytest harness](https://docs.zephyrproject.org/latest/develop/test/pytest.html) | the upstream page on `harness: pytest` and every `harness_config` key |
| [Twister harnesses](https://docs.zephyrproject.org/latest/develop/test/twister.html#harnesses) | the other harnesses, including `shell`, `console` and `gtest` |
| `$ZEPHYR_BASE/scripts/pylib/pytest-twister-harness/` | the plugin itself. `fixtures.py`, `shell.py` and `device/device_adapter.py` are the three files behind this page. |
| [pytest fixtures](https://docs.pytest.org/en/stable/how-to/fixtures.html) | you can find about `conftest.py`, scopes and parametrized fixtures in more detail |
| [`boards.md`](boards.md) | the Twister flags for running any of this on hardware |
