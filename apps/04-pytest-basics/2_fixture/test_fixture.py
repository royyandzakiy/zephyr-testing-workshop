import pytest

@pytest.fixture
def numbers():
    return [1,2,3]

def test_sum(numbers):
    assert sum(numbers) == 6

@pytest.mark.parametrize("a,b,expected", [(1,2,3),(2,3,5)])
def test_add(a,b,expected):
    assert a + b == expected

@pytest.mark.parametrize("x", [0,1])
@pytest.mark.parametrize("y", [10,20])
def test_multiply(x,y):
    assert x * y == x*y
