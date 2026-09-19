# Markers, skips and expected failures.
#
# The useful question behind this file: when a test cannot run, what do you
# want the report to say? "Passed" is a lie, "failed" is noise, and pytest
# gives you three honest answers instead.

import logging

import pytest

logger = logging.getLogger(__name__)


def test_reset_zeroes_the_counter(app):
    app.press(3)
    app.reset()
    assert int(app.stats()['presses']) == 0


@pytest.mark.xfail(reason='`app reset` deliberately leaves the LED alone; '
                          'see the comment on cmd_reset in test_harness.c',
                   strict=True)
def test_reset_also_turns_the_led_off(app):
    """An assumption written down as a failing test.

    strict=True is the important part. Without it, a plain xfail passes
    whether or not the behaviour changes, so the day somebody *does* make
    reset clear the LED, nothing tells you. With it, an unexpected pass is a
    failure and you go delete this test.
    """
    app.press(1)
    app.reset()
    assert app.led() == 'off'


def test_emulated_button_only_exists_off_target(app, board_name):
    """Skipping on a condition you only know at runtime.

    testcase.yaml already narrows the platform list, so this is belt and
    braces. It still earns its place: platform_allow is checked by twister
    before the build, and this is checked against the device that actually
    answered, which are not always the same board.
    """
    if not board_name.startswith('native_sim'):
        pytest.skip(f'{board_name} drives a real pin; nothing to emulate')

    app.press(1)
    assert app.led() == 'on'


@pytest.mark.slow
def test_twenty_presses_leave_the_led_off(app):
    app.press(20)
    stats = app.stats()
    assert int(stats['presses']) == 20
    assert stats['led'] == 'off'
