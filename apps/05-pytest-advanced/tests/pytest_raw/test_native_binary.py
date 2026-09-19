# The same assertions as tests/shell_pytest/pytest/test_smoke.py, driven
# without twister.
#
# Compare the two files side by side. The test bodies are nearly identical;
# what changed is that conftest.py in this directory had to grow a process, a
# thread and a prompt parser to make `raw_shell` exist at all. That is the
# trade twister is offering.

import pytest

pytestmark = pytest.mark.e2e


def test_led_starts_off(raw_shell):
    lines = raw_shell.exec_command('app led')
    assert any('led=off' in ln for ln in lines), lines


def test_press_toggles(raw_shell):
    raw_shell.exec_command('app reset')
    raw_shell.exec_command('app btn 1')
    lines = raw_shell.exec_command('app led')
    assert any('led=on' in ln for ln in lines), lines


def test_stats_reports_the_press(raw_shell):
    raw_shell.exec_command('app reset')
    raw_shell.exec_command('app btn 3')
    lines = raw_shell.exec_command('app stats')
    assert any('presses=3' in ln for ln in lines), lines
