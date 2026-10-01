// The seam: every decision the app makes, as pure functions with no GPIO,
// devicetree or kernel, so they can be tested without a board or emulator.

#ifndef BLINK_LOGIC_H_
#define BLINK_LOGIC_H_

#include <stdbool.h>

/** Next LED state after one button press. */
bool blink_logic_toggle(bool led_on);

/** How that state is spelled in the log line: "ON" or "OFF". */
const char *blink_logic_str(bool led_on);

#endif /* BLINK_LOGIC_H_ */
