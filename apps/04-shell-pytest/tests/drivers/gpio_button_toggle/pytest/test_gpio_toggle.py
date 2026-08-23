# tests/drivers/gpio_button_toggle/pytest/test_gpio_toggle.py

import logging
from twister_harness import Shell

logger = logging.getLogger(__name__)

# LED starts off; every press toggles, including the first.
EXPECTED_STATES = ('ON', 'OFF', 'ON', 'OFF', 'ON')


def test_gpio_button_toggle(shell: Shell):
    for press, expected in enumerate(EXPECTED_STATES, start=1):
        logger.info('Press %d: expecting LED %s', press, expected)
        lines = shell.exec_command('test_btn')

        assert any('Test: Triggering emulated button press' in line
                   for line in lines), \
            f'press {press}: shell command produced no trigger line'

        assert any(f'Button pressed! LED is now {expected}' in line
                   for line in lines), \
            f'press {press}: expected LED {expected}, got {lines}'