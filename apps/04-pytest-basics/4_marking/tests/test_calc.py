# tests/test_calc.py
import pytest
from slow_calc import add, slow_add

def test_add():
    assert add(1,2) == 3

@pytest.mark.custom_slow
def test_slow_add():
    assert slow_add(1,2) == 3

@pytest.mark.skip(reason="not implemented yet")
def test_future():
    assert quick_add(1,2) == 3

@pytest.mark.xfail(reason="known bug")
def test_xfail():
    assert wrong_add(1,2) == 3