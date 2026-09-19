// src/main.c
//
// Wires the three pieces together and ticks. This file is the composition
// root: the only place that knows both which port implementations exist and
// which service uses them.
//
// tests/fff replaces this file with its own main, which is why the service
// never had to know.

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "climate_service.h"
#include "sensor_port.h"

int main(void)
{
	struct climate_service svc;

	printk("Climate service starting\n");

	if (!sensor_port_ready()) {
		printk("Error: sensor port not ready\n");
		return 0;
	}

	climate_service_init(&svc);

	while (1) {
		int ret = climate_service_tick(&svc);

		if (ret != 0) {
			printk("tick failed: %d (errors=%u faulted=%d)\n",
			       ret, svc.errors, svc.faulted);
			if (svc.faulted) {
				printk("clearing latched fault\n");
				climate_service_clear_fault(&svc);
			}
		} else {
			printk("T: %d mC | P: %d Pa | H: %d m%%RH | ALARM %s\n",
			       svc.last.temp_mc, svc.last.press_pa, svc.last.hum_mrh,
			       svc.alarm ? "ON" : "OFF");
		}

		k_sleep(K_SECONDS(1));
	}

	return 0;
}
