// src/app_state.h
//
// The read-only window the test backdoor gets onto the running app.
//
// app 04's pytest suite could only assert on lines the app happened to print.
// That works until you want to ask a question the app was not already
// answering, and then you are stuck adding printk()s to production code so a
// test can see something. These two accessors are the alternative: the app
// exports state, and tests/shell_pytest/test_harness.c is the only thing that
// turns them into shell commands.

#ifndef APP_STATE_H_
#define APP_STATE_H_

#include <stdbool.h>
#include <stdint.h>

/** Current LED state, as the app believes it to be. */
bool app_led_state(void);

/** How many button presses the app has handled since boot. */
uint32_t app_press_count(void);

/** Reset the press counter. Lets a test start from a known point without
 *  rebooting the device, which matters a lot once the device is a real board
 *  on a runner and a reboot costs ten seconds. */
void app_press_count_reset(void);

#endif /* APP_STATE_H_ */
