# Same assertions as tests/shell_pytest/pytest/test_smoke.py, without twister.
# The test bodies barely differ; the cost is the process, thread and prompt
# parser that conftest.py needs to provide `raw_shell`.

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
