// Read-only view of app state for the test backdoor, so tests can query state
// without the app adding printk()s just for them. Only
// tests/shell_pytest/test_harness.c turns these into shell commands.

#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <stdbool.h>
#include <stdint.h>

/** Current LED state, as the app believes it to be. */
bool app_led_state(void);

/** How many button presses the app has handled since boot. */
uint32_t app_press_count(void);

/** Reset the press counter. Lets a test start from a known point without a
 *  reboot, which costs seconds on a real board. */
void app_press_count_reset(void);

#endif /* APP_STATE_H_ */
