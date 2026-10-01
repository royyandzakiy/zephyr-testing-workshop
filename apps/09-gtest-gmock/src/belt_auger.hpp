// The app's shipping IAuger, not a test double. Jams every seventh turn so
// src/main.cpp has something to print. Nothing asserts on it, so unlike a mock
// it is allowed to be inaccurate.

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
