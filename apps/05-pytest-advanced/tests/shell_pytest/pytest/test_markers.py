# Markers, skips and expected failures: honest outcomes for a test that cannot
# or should not pass, instead of a false pass or a noisy fail.

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

    strict=True turns an unexpected pass into a failure. A plain xfail would
    stay green if reset started clearing the LED, and nothing would flag it.
    """
    app.press(1)
    app.reset()
    assert app.led() == 'off'


def test_emulated_button_only_exists_off_target(app, board_name):
    """Skips on a condition known only at runtime, from the device that answered.

    `platform_allow` is checked before the build. app.overlay reroutes sw0 to
    gpio_emul on every platform, so this only runs on native_sim until that
    reroute moves into a boards/native_sim_native.overlay.
    """
    if not board_name.startswith('native_sim'):
        pytest.skip(f'{board_name}: the emulated button reroute is native_sim only')

    app.press(1)
    assert app.led() == 'on'


@pytest.mark.slow
def test_twenty_presses_leave_the_led_off(app):
    app.press(20)
    stats = app.stats()
    assert int(stats['presses']) == 20
    assert stats['led'] == 'off'
