# Fixtures, and the two the twister harness gives you.
#
# `shell` is the polite one: it sends a command, waits for the prompt, and
# hands back the lines. `dut` is the raw device adapter, and you reach for it
# when the thing you want to assert on is not a response to a command.

import logging

import pytest
from twister_harness import DeviceAdapter

logger = logging.getLogger(__name__)


def test_dut_sees_boot_output(dut: DeviceAdapter):
    """Asserting on something nobody asked the device for.

    The banner is printed once at boot, before any shell command exists.

    Note what this test does NOT ask for: the `shell` fixture. `Shell` wraps
    this same adapter and drains the buffer while it waits for its first
    prompt, so a test that takes both fixtures finds the banner already gone.
    Asking only for `dut` is what leaves it there to be read.
    """
    lines = dut.readlines_until(regex='GPIO Button .* Toggle started', timeout=5.0)

    assert lines, 'no boot banner before the timeout'


def test_dut_readlines_until(dut: DeviceAdapter):
    """Waiting for a specific line instead of a fixed sleep.

    `time.sleep(2)` is the reflex here and it is wrong twice over: too short on
    a loaded CI runner, and wasted seconds everywhere else.

    The command goes straight to the device rather than through
    `shell.exec_command()`, for the same reason as the test above.
    `exec_command` reads until it sees the prompt again, which consumes the
    very line this test is waiting for.
    """
    dut.readlines_until(regex='Ready. Press the button', timeout=5.0)

    dut.write(b'app btn 1\n')

    lines = dut.readlines_until(regex='Button pressed!', timeout=5.0)

    assert lines, 'device never reported the press'


@pytest.fixture()
def pressed_once(app):
    """Setup and teardown in one fixture, with yield in the middle.

    Anything a test needs *before* its first assertion belongs in a fixture.
    The test below reads as one line because of it.

    Teardown runs even when the test fails, which is the difference between
    this and putting the cleanup at the end of the test body. On real hardware
    that matters: a test that dies with the LED on leaves it on for the next
    one.
    """
    app.press(1)

    yield app

    logger.info('counter ended at %s', app.stats()['presses'])
    app.reset()


def test_second_press_turns_it_back_off(pressed_once):
    pressed_once.press(1)
    assert pressed_once.led() == 'off'
