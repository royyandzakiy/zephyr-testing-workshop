// include/feeder/dispenser.hpp
//
// The same job as apps/08-fff-mocks/src/dispenser.c. Read them side by side.
// The interesting difference is that the auger arrives through the constructor
// instead of through the linker.

#pragma once

#include <cstdint>

#include "feeder/auger.hpp"

namespace feeder {

class Dispenser {
public:
    /// Constructor injection. A reference, not a pointer, because a Dispenser
    /// with no auger is not a thing that should be constructible.
    explicit Dispenser(IAuger& auger) : auger_(auger) {}

    /// Put one portion in the water, in as many turns as it takes.
    /// @return 0, -EIO if a turn jammed, -EINVAL if @p grams was zero.
    int feed(std::uint16_t grams);

    [[nodiscard]] std::uint32_t dispensed() const { return dispensed_g_; }
    [[nodiscard]] std::uint32_t jams() const { return jams_; }

private:
    IAuger& auger_;

    std::uint32_t dispensed_g_{0};
    std::uint32_t jams_{0};
};

}  // namespace feeder
