// src/blink_logic.h
//
// THE SEAM.
//
// Everything the application decides, with no GPIO, no devicetree and no
// kernel in sight. Pure functions of their arguments -- which is exactly
// what makes them testable without a board, a probe or an emulator.

#ifndef BLINK_LOGIC_H_
#define BLINK_LOGIC_H_

#include <stdbool.h>

/** Next LED state after one button press. */
bool blink_logic_toggle(bool led_on);

/** How that state is spelled in the log line: "ON" or "OFF". */
const char *blink_logic_str(bool led_on);

#endif /* BLINK_LOGIC_H_ */
