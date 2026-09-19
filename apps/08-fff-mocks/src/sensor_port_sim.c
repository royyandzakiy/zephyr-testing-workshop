// src/sensor_port_sim.c
//
// A second production implementation of the same header, for builds with no
// sensor in the devicetree. Walks temperature and humidity up and down so the
// app has something to print on a laptop.
//
// Worth noticing what this is NOT: it is not a test double. It ships, it has
// no assertions, and no test links it either. Swapping it in is a CMake
// decision, which is the cheapest kind of portability there is once the seam
// exists.

#include "sensor_port.h"

static int32_t temp_mc = 24000;
static int32_t hum_mrh = 55000;
static int32_t step = 500;

bool sensor_port_ready(void)
{
	return true;
}

int sensor_port_read(struct climate_raw *out)
{
	temp_mc += step;
	hum_mrh += step * 3;

	if (temp_mc > 32000 || temp_mc < 24000) {
		step = -step;
	}

	out->temp_mc = temp_mc;
	out->hum_mrh = hum_mrh;
	out->press_pa = 100650;

	return 0;
}
