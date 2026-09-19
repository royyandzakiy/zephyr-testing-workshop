// src/main.cpp
//
// An ordinary Zephyr main(), written in C++, wiring an auger to a dispenser
// and feeding once a second.
//
// This is the composition root: the only place that knows both which auger
// implementation exists and which dispenser uses it. tests/gtest brings its
// own main and its own auger, which is why the dispenser never had to know.

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "belt_auger.hpp"
#include "feeder/dispenser.hpp"
#include "feeder/portion.hpp"

int main(void)
{
    printk("Pond feeder dispenser starting\n");

    feeder::BeltAuger auger;
    feeder::Dispenser dispenser(auger);

    while (true) {
        const int ret = dispenser.feed(feeder::gramsOf(feeder::Portion::Large));

        printk("feed -> %d | total %u g | jams %u\n",
               ret, dispenser.dispensed(), dispenser.jams());

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
