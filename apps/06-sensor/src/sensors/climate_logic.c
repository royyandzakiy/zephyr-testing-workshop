#include "climate_logic.h"

int32_t climate_milli(int32_t val1, int32_t val2)
{
    /* val2 is millionths, so a milli-unit is 1000 of them. Round to nearest:
     * truncation drops up to a milli-unit off every reading, always the same
     * way, which biases averages without ever looking like a bug. */
    int32_t milli = val2 / 1000;
    int32_t rem = val2 % 1000;

    if (rem >= 500) {
        milli += 1;
    } else if (rem <= -500) {
        milli -= 1;
    }

    return val1 * 1000 + milli;
}
