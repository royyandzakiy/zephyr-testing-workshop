// src/alarm_port.h
//
// The other half of the seam: where the decision goes out.
//
// On a board this drives an LED. In tests it is a FAKE_VOID_FUNC, and the
// assertions are about *when* it was called and with what, which is not
// something you can ask a GPIO.

#ifndef ALARM_PORT_H_
#define ALARM_PORT_H_

#include <stdbool.h>

/** Drive the alarm indicator. Called only when the state actually changes. */
void alarm_port_set(bool on);

#endif /* ALARM_PORT_H_ */
