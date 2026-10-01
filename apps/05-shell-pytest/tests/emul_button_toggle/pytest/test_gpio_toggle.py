# tests/emul_button_toggle/pytest/test_gpio_toggle.py

import logging
import time
import pytest
from twister_harness import Shell, DeviceAdapter

logger = logging.getLogger(__name__)

def test_button_toggle(shell: Shell):
    # we assume default state is OFF
    for expected in ('ON', 'OFF'):
        lines = shell.exec_command('test_btn')
        # only wait if the LED line comes after the prompt (deferred logging on real boards)
        if f'LED is now {expected}' not in '\n'.join(lines):
            lines += shell._device.readlines_until(
                regex=f'LED is now {expected}', timeout=2
            )
        output = '\n'.join(lines)

        assert 'Test: Triggering emulated button press' in output
        assert f'Button pressed! LED is now {expected}' in output

@pytest.mark.slow
def test_button_toggle_slow(shell: Shell):
    # we assume default state is OFF
    for expected in ('ON', 'OFF'):
        time.sleep(3) # adding this to slow things down
        lines = shell.exec_command('test_btn')
        # only wait if the LED line comes after the prompt (deferred logging on real boards)
        if f'LED is now {expected}' not in '\n'.join(lines):
            lines += shell._device.readlines_until(
                regex=f'LED is now {expected}', timeout=2
            )
        output = '\n'.join(lines)

        assert 'Test: Triggering emulated button press' in output
        assert f'Button pressed! LED is now {expected}' in output

def test_button_toggle_dut(dut: DeviceAdapter):
    # we assume default state is OFF
    for expected in ('ON', 'OFF'):
        dut.write(b'test_btn\n')  # dut.write takes bytes and returns nothing
        lines = dut.readlines_until(
            regex=f'LED is now {expected}', timeout=2
        )
        output = '\n'.join(lines)

        assert 'Test: Triggering emulated button press' in output
        assert f'Button pressed! LED is now {expected}' in output
