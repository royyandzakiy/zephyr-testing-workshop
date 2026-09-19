// src/logic.cpp

#include "climate/logic.hpp"

namespace climate {

std::int32_t toMilli(std::int32_t val1, std::int32_t val2) {
    std::int32_t milli = val2 / 1000;
    const std::int32_t rem = val2 % 1000;

    if (rem >= 500) {
        milli += 1;
    } else if (rem <= -500) {
        milli -= 1;
    }

    return val1 * 1000 + milli;
}

bool alarmState(std::int32_t temp_mc, std::int32_t hum_mrh, bool prev) {
    if (prev) {
        return temp_mc > kTempOffMilliC || hum_mrh > kHumOffMilliRh;
    }

    return temp_mc >= kTempOnMilliC || hum_mrh >= kHumOnMilliRh;
}

}  // namespace climate
