#include "bme280.h"
#include "climate_logic.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>

/* Whatever the devicetree points this label at. On nrf5340dk and the esp32s3
 * that is a real BME280 on a real I2C bus; on native_sim it is the emulated
 * one from bme280_emul.c. Nothing below knows the difference. */
#define BME280_NODE DT_NODELABEL(bme280)
static const struct device *bme280_dev = DEVICE_DT_GET(BME280_NODE);

#define SENSOR_THREAD_STACK_SIZE 1024
#define SENSOR_THREAD_PRIORITY 5

K_THREAD_STACK_DEFINE(sensor_stack, SENSOR_THREAD_STACK_SIZE);
static struct k_thread sensor_thread_data;

static bool alarm_state;

int bme280_read_once(struct climate_reading *out)
{
	struct sensor_value temp, press, humidity;
	int ret;

	if (!device_is_ready(bme280_dev)) {
		return -ENODEV;
	}

	ret = sensor_sample_fetch(bme280_dev);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(bme280_dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(bme280_dev, SENSOR_CHAN_PRESS, &press);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(bme280_dev, SENSOR_CHAN_HUMIDITY, &humidity);
	if (ret != 0) {
		return ret;
	}

	/* The driver hands back degrees C, kPa and %RH as val1 + val2/1e6.
	 * climate_milli() is the seam -- and note milli-kPa happens to be Pa. */
	out->temp_mc = climate_milli(temp.val1, temp.val2);
	out->press_mpa = climate_milli(press.val1, press.val2);
	out->hum_mrh = climate_milli(humidity.val1, humidity.val2);

	return 0;
}

bool bme280_alarm_state(void)
{
	return alarm_state;
}

static void bme280_read_and_display(void)
{
	struct climate_reading r;
	int ret;

	ret = bme280_read_once(&r);
	if (ret != 0) {
		printk("Error %d: failed to read BME280\n", ret);
		return;
	}

	alarm_state = climate_alarm(r.temp_mc, r.hum_mrh, alarm_state);

	printk("T: %d mC | P: %d Pa | H: %d m%%RH | ALARM %s\n",
	       r.temp_mc, r.press_mpa, r.hum_mrh, alarm_state ? "ON" : "OFF");
}

static void sensor_thread(void *arg1, void *arg2, void *arg3)
{
	ARG_UNUSED(arg1);
	ARG_UNUSED(arg2);
	ARG_UNUSED(arg3);

	printk("Initializing BME280 sensor...\n");

	if (!device_is_ready(bme280_dev)) {
		printk("Error: BME280 device %s is not ready\n", bme280_dev->name);
		return;
	}

	printk("BME280 sensor %s is ready!\n", bme280_dev->name);

	while (1) {
		bme280_read_and_display();
		k_sleep(K_SECONDS(2));
	}
}

void bme280_start(void)
{
	k_thread_create(&sensor_thread_data, sensor_stack,
			K_THREAD_STACK_SIZEOF(sensor_stack),
			sensor_thread, NULL, NULL, NULL,
			SENSOR_THREAD_PRIORITY, 0, K_NO_WAIT);
}
