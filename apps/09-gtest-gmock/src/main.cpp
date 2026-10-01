// Composition root: wires a BeltAuger to a Dispenser and feeds once a second.
// tests/gtest brings its own main() and auger, so the dispenser never has to know.

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
