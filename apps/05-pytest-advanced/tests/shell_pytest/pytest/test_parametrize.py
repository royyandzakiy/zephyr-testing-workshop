# Parametrization reports each case as its own result, so a red run shows which
# input broke. A for-loop in one test stops at the first failure and hides the rest.

import logging

import pytest

logger = logging.getLogger(__name__)


@pytest.mark.parametrize('presses,expected', [
    (1, 'on'),
    (2, 'off'),
    (3, 'on'),
    (4, 'off'),
])
def test_led_state_after_n_presses(app, presses, expected):
    """Toggling is odd/even. Four cases, four test results."""
    app.press(presses)
    assert app.led() == expected


@pytest.mark.parametrize('presses', [1, 2, 5, 20],
                         ids=['single', 'double', 'handful', 'max'])
def test_press_counter_matches(app, presses):
    """ids= names the cases `[max]` instead of `[20]`, so the CI log reads clearly."""
    app.press(presses)
    assert int(app.stats()['presses']) == presses


@pytest.mark.negative
@pytest.mark.parametrize('arg', ['0', '21', 'abc', '-1', '3.5'],
                         ids=['zero', 'over-max', 'not-a-number',
                              'negative', 'float'])
def test_bad_press_count_is_rejected(app, arg):
    """Each bad input is its own result, so the one that slipped through is obvious."""
    lines = app.raw(f'app btn {arg}')
    assert any('err=bad_arg' in line for line in lines), \
        f'device accepted {arg!r}: {lines}'


@pytest.mark.slow
@pytest.mark.parametrize('cycle', range(1, 4))
def test_toggle_is_stable_over_many_cycles(app, cycle):
    """Marked slow for real boards on a 115200 baud UART; native_sim is fast.

    Three cycles, because more would only repeat the same path.
    """
    app.press(2)
    assert app.led() == 'off', f'cycle {cycle} left the LED on'
