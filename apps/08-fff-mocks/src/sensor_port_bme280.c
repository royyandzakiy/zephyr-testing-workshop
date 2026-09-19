// src/sensor_port_bme280.c
//
// The production implementation of sensor_port.h. Built only where there is a
// BME280 in the devicetree -- see CMakeLists.txt.
//
// Nothing in tests/fff ever links this file. That is the whole arrangement:
// the bus, the driver and the devicetree all live on this side of the seam,
// and the tests live on the other.

#include <errno.h>

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

#include "sensor_port.h"
#include "climate_logic.h"

#define BME280_NODE DT_NODELABEL(bme280)
static const struct device *const dev = DEVICE_DT_GET(BME280_NODE);

bool sensor_port_ready(void)
{
	return device_is_ready(dev);
}

int sensor_port_read(struct climate_raw *out)
{
	struct sensor_value temp, press, hum;
	int ret;

	ret = sensor_sample_fetch(dev);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_AMBIENT_TEMP, &temp);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_PRESS, &press);
	if (ret != 0) {
		return ret;
	}

	ret = sensor_channel_get(dev, SENSOR_CHAN_HUMIDITY, &hum);
	if (ret != 0) {
		return ret;
	}

	out->temp_mc = climate_milli(temp.val1, temp.val2);
	out->press_pa = climate_milli(press.val1, press.val2);
	out->hum_mrh = climate_milli(hum.val1, hum.val2);

	return 0;
}
