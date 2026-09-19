# tests/shell_pytest/pytest/conftest.py
#
# Everything shared between the four test files. If you have written pytest
# before, none of this is new; what is new is that the thing under test is a
# device on the other end of a serial port, and twister_harness hands it to
# you as an ordinary fixture.

import logging
import re

import pytest
from twister_harness import DeviceAdapter, Shell

logger = logging.getLogger(__name__)

KV_RE = re.compile(r'(\w+)=(\S+)')


def pytest_configure(config):
    """Register our markers.

    Without this, pytest prints a PytestUnknownMarkWarning for every custom
    marker and `-m "not slow"` still works, so it is easy to skip. Do it
    anyway: `--strict-markers` turns those warnings into errors, and a typo in
    a marker name is otherwise completely silent.
    """
    config.addinivalue_line('markers', 'slow: takes more than a couple of seconds')
    config.addinivalue_line('markers', 'negative: asserts the device rejects bad input')


def parse_kv(lines):
    """Collapse every key=value pair in the shell output into one dict.

    This is the whole reason test_harness.c prints key=value. Assertions get
    to talk about `stats['presses']` instead of about the exact wording of a
    printk, so rewording a log line does not break twenty tests.
    """
    out = {}
    for line in lines:
        out.update(dict(KV_RE.findall(line)))
    return out


@pytest.fixture()
def app(shell: Shell):
    """A tiny wrapper over the raw Shell fixture.

    Two things happen here that are worth copying into your own suites:

    1. Setup: every test starts with the press counter at zero, so tests do
       not depend on the order pytest happened to run them in.
    2. A narrow API. Tests call `app.press(3)`, not
       `shell.exec_command('app btn 3')`. When the command spelling changes
       you edit this class, not the tests.
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


@pytest.fixture(scope='session')
def board_name(dut: DeviceAdapter) -> str:
    """The platform twister is running against.

    Session-scoped on purpose: it never changes during a run, and this is the
    fixture the skipif tests in test_markers.py key off.
    """
    return dut.device_config.platform
