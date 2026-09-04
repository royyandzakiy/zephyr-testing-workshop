// src/blink_logic.c

#include "blink_logic.h"

bool blink_logic_toggle(bool led_on)
{
    return !led_on;
}

const char *blink_logic_str(bool led_on)
{
    return led_on ? "ON" : "OFF";
}
