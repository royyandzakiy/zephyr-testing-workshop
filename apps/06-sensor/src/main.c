#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include "hardware/button.h"
#include "hardware/led.h"
#include "sensors/bme280.h"

int main(void)
{
    printk("System Started\n");
    
    /* Initialize all hardware */
    led_init();
    button_init();
    // bme280_start();

    printk("System ready. Press button to toggle LED.\n");
    
    /* Main thread just idles - all work done by interrupt handlers and sensor thread */
    while (1) {
        k_sleep(K_FOREVER);
    }
    return 0;
}