# Fixtures and helpers shared by the four test files. twister_harness hands
# over the device on the serial port as an ordinary fixture.

import logging
import re

import pytest
from twister_harness import DeviceAdapter, Shell

logger = logging.getLogger(__name__)

KV_RE = re.compile(r'(\w+)=(\S+)')


def pytest_configure(config):
    """Register the custom markers.

    Unregistered markers only warn, so a typo in a marker name goes unnoticed;
    with `--strict-markers` it becomes an error.
    """
    config.addinivalue_line('markers', 'slow: takes more than a couple of seconds')
    config.addinivalue_line('markers', 'negative: asserts the device rejects bad input')


def parse_kv(lines):
    """Collapse every key=value pair in the shell output into one dict.

    Tests assert on `stats['presses']` rather than on printk wording, so
    rewording a log line does not break them.
    """
    out = {}
    for line in lines:
        out.update(dict(KV_RE.findall(line)))
    return out


@pytest.fixture()
def app(shell: Shell):
    """A thin wrapper over the Shell fixture.

    Resets the press counter first, so tests do not depend on run order.
    Tests call `app.press(3)` rather than `shell.exec_command('app btn 3')`,
    so a change in command spelling is fixed here, not in every test.
    """

    class App:
        def __init__(self, sh: Shell):
            self._sh = sh

        def raw(self, cmd: str):
            logger.info('-> %s', cmd)
            lines = self._sh.exec_command(cmd)
            logger.info('<- %s', lines)
            return lines

        def press(self, n: int = 1):
            return parse_kv(self.raw(f'app btn {n}'))

        def led(self) -> str:
            return parse_kv(self.raw('app led'))['led']

        def stats(self) -> dict:
            return parse_kv(self.raw('app stats'))

        def reset(self):
            return parse_kv(self.raw('app reset'))

    a = App(shell)
    a.reset()
    return a


@pytest.fixture()
def board_name(dut: DeviceAdapter) -> str:
    """The platform twister is running against.

    Function-scoped because `dut` is. A session-scoped version fails with
    ScopeMismatch, and only at test setup, not at collection.
    """
    return dut.device_config.platform
