# The three most valuable assertions; the other test files are variations.

import logging

from twister_harness import Shell

logger = logging.getLogger(__name__)


def test_shell_is_alive(shell: Shell):
    """The device booted and the shell answers.

    Runs first because most red runs on real hardware are a board that did not
    come up or the wrong serial port, not a logic bug.
    """
    lines = shell.exec_command('app led')
    assert any('led=' in line for line in lines), f'shell gave nothing usable: {lines}'


def test_led_starts_off(app):
    assert app.led() == 'off'


def test_one_press_turns_the_led_on(app):
    app.press(1)
    assert app.led() == 'on'
