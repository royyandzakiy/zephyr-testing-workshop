# src/slow_calc.py
import time

def add(a,b):
    return a + b

def slow_add(a,b):
    time.sleep(5)
    return a + b

def wrong_add(a,b):
    return -1