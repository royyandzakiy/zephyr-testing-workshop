// src/sensors/climate_logic.c

#include "climate_logic.h"

int32_t climate_milli(int32_t val1, int32_t val2)
{
    /* val2 is millionths, so a milli-unit is 1000 of them. Round to nearest
     * instead of truncating: truncation silently drops up to a whole
     * milli-unit off every single reading, always in the same direction,
     * which shows up months later as an averaged value that is quietly wrong
     * rather than as anything that looks like a bug. */
    int32_t milli = val2 / 1000;
    int32_t rem = val2 % 1000;

    if (rem >= 500) {
        milli += 1;
    } else if (rem <= -500) {
        milli -= 1;
    }

    return val1 * 1000 + milli;
}

bool climate_alarm(int32_t temp_mc, int32_t hum_mrh, bool prev)
{
    if (prev) {
        /* Already on: hold until BOTH readings fall back under their release
         * points. Either one still high keeps it latched. */
        return (temp_mc > CLIMATE_TEMP_OFF_MC) || (hum_mrh > CLIMATE_HUM_OFF_MRH);
    }

    /* Off: either reading reaching its trip point is enough. */
    return (temp_mc >= CLIMATE_TEMP_ON_MC) || (hum_mrh >= CLIMATE_HUM_ON_MRH);
}
