# The three assertions worth having if you only get three.
#
# Everything else in this directory is a variation. Start here.

import logging

from twister_harness import Shell

logger = logging.getLogger(__name__)


def test_shell_is_alive(shell: Shell):
    """The device booted and the shell answers.

    Almost every red pytest run on real hardware is this, not a logic bug: the
    board did not come up, or the serial port belongs to something else. Put
    the cheapest possible check first so the failure says so.
    """
    lines = shell.exec_command('app led')
    assert any('led=' in line for line in lines), f'shell gave nothing usable: {lines}'


def test_led_starts_off(app):
    assert app.led() == 'off'


def test_one_press_turns_the_led_on(app):
    app.press(1)
    assert app.led() == 'on'
