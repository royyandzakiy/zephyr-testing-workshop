# The two harness fixtures. `shell` sends a command and returns the lines up to
# the next prompt; `dut` is the raw adapter, for output that is not a reply.

import logging

import pytest
from twister_harness import DeviceAdapter

logger = logging.getLogger(__name__)


def test_dut_sees_boot_output(dut: DeviceAdapter):
    """Asserts on the boot banner, printed once before the shell exists.

    Takes only `dut`, not `shell`: Shell drains the buffer while waiting for
    its first prompt, so with both fixtures the banner is already gone.
    """
    lines = dut.readlines_until(regex='GPIO Button .* Toggle started', timeout=5.0)

    assert lines, 'no boot banner before the timeout'


def test_dut_readlines_until(dut: DeviceAdapter):
    """Waits for a specific line instead of a fixed sleep.

    A `time.sleep(2)` is too short on a loaded CI runner and wasted time
    elsewhere. The command goes straight to `dut` because `exec_command` reads
    up to the next prompt and would consume the line this test waits for.
    """
    dut.readlines_until(regex='Ready. Press the button', timeout=5.0)

    dut.write(b'app btn 1\n')

    lines = dut.readlines_until(regex='Button pressed!', timeout=5.0)

    assert lines, 'device never reported the press'


@pytest.fixture()
def pressed_once(app):
    """Setup and teardown in one fixture, with yield in the middle.

    Teardown runs even when the test fails, unlike cleanup at the end of the
    test body, so a failed test cannot leave the LED on for the next one.
    """
    app.press(1)

    yield app

    logger.info('counter ended at %s', app.stats()['presses'])
    app.reset()


def test_second_press_turns_it_back_off(pressed_once):
    pressed_once.press(1)
    assert pressed_once.led() == 'off'
