// src/blinky.h
//
// The hardware side of the seam: binds the led0/sw0 devicetree aliases and
// wires the button interrupt to the logic in blink_logic.h.
//
// Note there is no main() here. That matters -- a test can link this file,
// call blinky_init() and drive the pins, without fighting the application
// over who owns main().

#ifndef BLINKY_H_
#define BLINKY_H_

/** Configure the LED and button and start reacting to presses. 0 on success. */
int blinky_init(void);

#endif /* BLINKY_H_ */
