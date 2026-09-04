// src/main.c

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

int main(void)
{
    printk("Hello from the Zephyr testing workshop!\n");
    printk("Board: %s\n", CONFIG_BOARD_TARGET);

    return 0;
}
