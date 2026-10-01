// Hardware side of the seam: binds the led0/sw0 aliases and wires the button
// interrupt to blink_logic.h. No main() here, so a test can link this file
// and call blinky_init() itself.

#ifndef BLINKY_H_
#define BLINKY_H_

/** Configure the LED and button and start reacting to presses. 0 on success. */
int blinky_init(void);

#endif /* BLINKY_H_ */
