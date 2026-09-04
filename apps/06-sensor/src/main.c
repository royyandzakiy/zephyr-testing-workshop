// src/main.c

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

/* Button definition - works for BOTH physical and emulated */
#define BUTTON_NODE DT_ALIAS(sw0)
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/* BME280 sensor definition */
#define BME280_NODE DT_NODELABEL(bme280)
static const struct device *bme280_dev = DEVICE_DT_GET(BME280_NODE);

static struct gpio_callback button_cb_data;

static bool led_state = false;

void button_pressed_cb(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&led);
    led_state = !led_state;
    printk("Button pressed! LED is now %s\n", led_state ? "ON" : "OFF");
}

int main(void)
{
    int ret;

    printk("GPIO Button + LED Toggle started\n");

    /* Check button device */
    if (!device_is_ready(button.port)) {
        printk("Error: button device %s is not ready\n", button.port->name);
        return 0;
    }

    if (led.port == NULL || !device_is_ready(led.port)) {
        printk("Error: LED device %s is not ready\n", led.port->name);
        return 0;
    }

    /* Configure LED as output */
    ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure LED\n", ret);
        return 0;
    }

    /* Configure button as input with interrupt */
    ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
    if (ret != 0) {
        printk("Error %d: failed to configure button\n", ret);
        return 0;
    }

    ret = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_TO_ACTIVE);
    if (ret != 0) {
        printk("Error %d: failed to configure interrupt\n", ret);
        return 0;
    }

    /* Set up callback */
    gpio_init_callback(&button_cb_data, button_pressed_cb, BIT(button.pin));
    ret = gpio_add_callback(button.port, &button_cb_data);
    if (ret != 0) {
        printk("Error %d: failed to add callback\n", ret);
        return 0;
    }

    /* Initialize BME280 sensor */
    printk("Initializing BME280 sensor...\n");

    if (!device_is_ready(bme280_dev)) {
        printk("Error: BME280 device %s is not ready\n", bme280_dev->name);
        return 0;
    }

    printk("BME280 sensor %s is ready!\n", bme280_dev->name);

    // /* Fetch and display initial sensor data */
    // ret = sensor_sample_fetch(bme280_dev);
    // if (ret != 0) {
    //     printk("Error %d: failed to fetch BME280 sample\n", ret);
    //     return 0;
    // }

    // struct sensor_value temp, press, humidity;

    // sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
    // sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
    // sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);

    // printk("BME280 initial readings:\n");
    // printk("  Temperature: %d.%06d °C\n", temp.val1, temp.val2);
    // printk("  Pressure:    %d.%06d Pa\n", press.val1, press.val2);
    // printk("  Humidity:    %d.%06d %%RH\n", humidity.val1, humidity.val2);

    // printk("Ready. Press the button to toggle LED.\n");

    // while (1) {
        // /* Read sensor every 2 seconds */
        // sensor_sample_fetch(bme280_dev);
        // sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
        // sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
        // sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);
        
        // printk("T: %d.%06d C | P: %d.%06d Pa | H: %d.%06d %%RH\n",
        //     temp.val1, temp.val2, press.val1, press.val2, humidity.val1, humidity.val2);
        
    //     k_sleep(K_SECONDS(2));
    // }
    return 0;
}