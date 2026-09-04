#include "bme280.h"
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

#define BME280_NODE DT_NODELABEL(bme280)
static const struct device *bme280_dev = DEVICE_DT_GET(BME280_NODE);

#define SENSOR_THREAD_STACK_SIZE 1024
#define SENSOR_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
static struct k_thread sensor_thread_data;

/* Private functions */
static void bme280_init(void)
{
    printk("Initializing BME280 sensor...\n");

    if (!device_is_ready(bme280_dev)) {
        printk("Error: BME280 device %s is not ready\n", bme280_dev->name);
        return;
    }

    printk("BME280 sensor %s is ready!\n", bme280_dev->name);
}

static void bme280_read_and_display(void)
{
    int ret;
    struct sensor_value temp, press, humidity;

    ret = sensor_sample_fetch(bme280_dev);
    if (ret != 0) {
        printk("Error %d: failed to fetch BME280 sample\n", ret);
        return;
    }

    sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
    sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);

    printk("T: %d.%06d C | P: %d.%06d Pa | H: %d.%06d %%RH\n",
           temp.val1, temp.val2, press.val1, press.val2, 
           humidity.val1, humidity.val2);
}

static void sensor_thread(void *arg1, void *arg2, void *arg3)
{
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    bme280_init();
    
    /* Fetch and display initial sensor data */
    bme280_read_and_display();

    while (1) {
        bme280_read_and_display();
        k_sleep(K_SECONDS(2));
    }
}

/* Public function - starts the sensor thread */
void bme280_start(void)
{
    k_thread_create(&sensor_thread_data, sensor_stack,
                    K_THREAD_STACK_SIZEOF(sensor_stack),
                    sensor_thread, NULL, NULL, NULL,
                    SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);
}