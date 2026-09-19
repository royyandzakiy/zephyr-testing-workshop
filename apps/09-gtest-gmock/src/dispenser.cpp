// src/dispenser.cpp

#include "feeder/dispenser.hpp"

#include <cerrno>

#include "feeder/portion.hpp"

namespace feeder {

int Dispenser::feed(std::uint16_t grams)
{
    if (grams == 0) {
        return -EINVAL;
    }

    const std::uint8_t turns = turnsFor(grams);

    for (std::uint8_t i = 0; i < turns; ++i) {
        const int ret = auger_.run(kGramsPerTurn);

        if (ret != 0) {
            // Stop here rather than carrying on. A jam usually means the
            // hopper bridged, and the next turn would grind against the same
            // blockage. Whatever already went out stays counted, because it
            // really did go out.
            ++jams_;
            return ret;
        }

        dispensed_g_ += kGramsPerTurn;
    }

    return 0;
}

}  // namespace feeder
