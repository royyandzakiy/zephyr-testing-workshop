// src/auger_port.h
//
// THE SEAM. One function, and no implementation in this header.
//
// The dispenser knows there is something called auger_run() and nothing else.
// Whether the thing behind it drives a motor, a servo or a printk is not its
// problem, and that is exactly what lets a test put its own definition here
// instead.

#ifndef AUGER_PORT_H_
#define AUGER_PORT_H_

#include <stdint.h>

/**
 * Turn the auger long enough to push @p grams of pellets out.
 *
 * @retval 0    the pellets went out
 * @retval -EIO the auger jammed, nothing moved
 */
int auger_run(uint16_t grams);

#endif /* AUGER_PORT_H_ */
