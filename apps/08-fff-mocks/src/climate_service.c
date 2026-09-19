// src/climate_service.c

#include <errno.h>
#include <string.h>

#include "climate_service.h"
#include "climate_logic.h"
#include "alarm_port.h"

void climate_service_init(struct climate_service *svc)
{
	memset(svc, 0, sizeof(*svc));
}

void climate_service_clear_fault(struct climate_service *svc)
{
	svc->faulted = false;
	svc->consecutive_errors = 0;
}

int climate_service_tick(struct climate_service *svc)
{
	/* Zero-initialised on purpose. sensor_port_read() only promises to fill
	 * this on success, and a partially-written struct read back after a
	 * half-failed driver call is a genuinely nasty bug to chase. */
	struct climate_raw raw = {0};
	bool next;
	int ret;

	if (svc->faulted) {
		return -EIO;
	}

	ret = sensor_port_read(&raw);
	if (ret != 0) {
		svc->errors++;
		svc->consecutive_errors++;

		if (svc->consecutive_errors >= CLIMATE_MAX_CONSECUTIVE_ERRORS) {
			svc->faulted = true;
		}

		/* Deliberately does NOT touch the alarm. A sensor that stopped
		 * answering is not a sensor reporting "everything is fine", and
		 * clearing the alarm here would be exactly that mistake. There
		 * is a test named after this paragraph. */
		return ret;
	}

	svc->consecutive_errors = 0;
	svc->reads++;
	svc->last = raw;

	next = climate_alarm(raw.temp_mc, raw.hum_mrh, svc->alarm);

	/* Only on a change. Calling alarm_port_set() every cycle would work
	 * and would also hammer a GPIO, or an MQTT publish, at the sample
	 * rate. Whether this edge detection is correct is the single most
	 * useful thing a fake can tell you here, and no amount of staring at
	 * an LED will. */
	if (next != svc->alarm) {
		svc->alarm = next;
		alarm_port_set(next);
	}

	return 0;
}
