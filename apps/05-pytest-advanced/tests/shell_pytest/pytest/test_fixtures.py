# Fixtures, and the two the twister harness gives you.
#
# `shell` is the polite one: it sends a command, waits for the prompt, and
# hands back the lines. `dut` is the raw device adapter, and you reach for it
# when the thing you want to assert on is not a response to a command.

import logging
import re

import pytest
from twister_harness import DeviceAdapter, Shell

logger = logging.getLogger(__name__)


def test_dut_sees_boot_output(dut: DeviceAdapter):
    """Asserting on something nobody asked the device for.

    The banner is printed once, at boot, before any shell command exists. The
    Shell fixture cannot see it. `dut.readlines()` can, because it reads the
    buffer the adapter has been filling since launch.
    """
    lines = dut.readlines()
    assert any('GPIO Button + LED Toggle started' in line for line in lines), \
        f'no boot banner in {lines[:20]}'


def test_dut_readlines_until(dut: DeviceAdapter, shell: Shell):
    """Waiting for a specific line instead of a fixed sleep.

    `time.sleep(2)` is the reflex here and it is wrong twice over: too short
    on a loaded CI runner, and wasted seconds everywhere else.
    """
    dut.clear_buffer()
    shell.exec_command('app btn 1')
    lines = dut.readlines_until(regex='Button pressed!', timeout=5.0)
    assert lines, 'device never reported the press'


@pytest.fixture()
def pressed_once(app):
    """A fixture that puts the device in a known non-default state.

    Anything a test needs *before* its first assertion belongs in a fixture.
    The test below reads as one line because of it.
    """
    app.press(1)
    return app


def test_second_press_turns_it_back_off(pressed_once):
    pressed_once.press(1)
    assert pressed_once.led() == 'off'


@pytest.fixture()
def counted(app):
    """Setup and teardown in one fixture, with yield in the middle.

    Teardown runs even when the test fails, which is the difference between
    this and putting the cleanup at the end of the test body. On real hardware
    that matters: a test that dies with the LED on leaves it on for the next
    one.
    """
    before = int(app.stats()['presses'])
    yield app
    after = int(app.stats()['presses'])
    logger.info('test moved the counter %d -> %d', before, after)
    app.reset()


def test_counter_is_monotonic(counted):
    first = int(counted.stats()['presses'])
    counted.press(3)
    second = int(counted.stats()['presses'])
    assert second > first


def test_uptime_moves_forward(app):
    """A property, not a value.

    Pinning uptime to a number would be a test that fails on a slower runner.
    Asserting it increases holds everywhere.
    """
    first = int(app.stats()['uptime_ms'])
    app.press(1)
    second = int(app.stats()['uptime_ms'])
    assert second >= first


def test_backdoor_advertises_every_subcommand(shell: Shell):
    """Regex against shell output, for when a plain substring is too loose.

    `app` prints its own list rather than letting the shell print help,
    because the shell's help wording has changed between Zephyr releases and
    a test that asserts on it is a test about the shell subsystem.
    """
    lines = shell.exec_command('app')
    match = re.search(r'subcmds=(\S+)', '\n'.join(lines))

    assert match, f'no subcmds line in {lines}'
    assert set(match.group(1).split(',')) == {'btn', 'led', 'stats', 'reset'}
