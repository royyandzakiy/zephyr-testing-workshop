// The seam: one function, no implementation here. The dispenser only knows
// auger_run(), so a test can link its own definition in place of the real one.

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
