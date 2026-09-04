// src/main.c

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "blinky.h"

int main(void)
{
    printk("GPIO Button + LED Toggle started\n");

    if (blinky_init() != 0) {
        return 0;
    }

    printk("Ready. Press the button to toggle LED.\n");

    k_sleep(K_FOREVER);
    return 0;
}
