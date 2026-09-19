// src/belt_auger.hpp
//
// A shipping implementation, not a test double. It jams every seventh turn so
// src/main.cpp has something to print.
//
// The distinction gets blurred constantly: a fake that ships is a simulator,
// and it is allowed to be wrong in ways a mock is not, because nobody is
// asserting on it.

#pragma once

#include <cerrno>

#include <zephyr/sys/printk.h>

#include "feeder/auger.hpp"

namespace feeder {

class BeltAuger final : public IAuger {
public:
    [[nodiscard]] int run(std::uint16_t grams) override
    {
        if (++turns_ % 7 == 0) {
            printk("auger: JAM\n");
            return -EIO;
        }

        printk("auger: %u g\n", grams);
        return 0;
    }

private:
    std::uint32_t turns_{0};
};

}  // namespace feeder
