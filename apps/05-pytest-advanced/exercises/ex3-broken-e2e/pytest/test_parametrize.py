# Parametrization: one test body, many cases.
#
# The lesson is not that @pytest.mark.parametrize exists. It is that a
# parametrized test reports each case as its own pass or fail, so a red run
# tells you *which* input broke. A for-loop inside one test stops at the first
# failure and tells you nothing about the rest.

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
    """ids= is worth the extra line.

    Without it pytest names these `test_press_counter_matches[20]`. With it
    you get `[max]`, and a CI log that reads like a sentence.
    """
    app.press(presses)
    assert int(app.stats()['presses']) == presses


@pytest.mark.negative
@pytest.mark.parametrize('arg', ['0', '21', 'abc', '-1', '3.5'],
                         ids=['zero', 'over-max', 'not-a-number',
                              'negative', 'float'])
def test_bad_press_count_is_rejected(app, arg):
    """Negative cases deserve the same treatment.

    An embedded shell that silently accepts nonsense is a bug you find in the
    field. Each of these is a separate result, so you can see at a glance that
    it is the float case that slipped through and not all five.
    """
    lines = app.raw(f'app btn {arg}')
    assert any('err=bad_arg' in line for line in lines), \
        f'device accepted {arg!r}: {lines}'


@pytest.mark.slow
@pytest.mark.parametrize('cycle', range(1, 4))
def test_toggle_is_stable_over_many_cycles(app, cycle):
    """Marked slow, so `-m "not slow"` skips every cycle.

    Repeating a toggle is not actually slow on native_sim. It is slow on a
    board over a 115200 baud UART, which is the case the marker is there for.
    Three cycles, because the tenth would exercise the same path as the second.
    """
    app.press(2)
    assert app.led() == 'off', f'cycle {cycle} left the LED on'
