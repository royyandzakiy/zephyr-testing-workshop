// src/climate_service.h
//
// The unit under test. It reads through sensor_port, decides with
// climate_logic, and announces through alarm_port. It includes no Zephyr
// header, binds no devicetree node and opens no bus.
//
// That is not an accident of this example. It is the shape you have to give a
// module before fakes are of any use at all: if climate_service.c called
// sensor_sample_fetch() directly there would be nothing to substitute, short
// of linker tricks.

#ifndef CLIMATE_SERVICE_H_
#define CLIMATE_SERVICE_H_

#include <stdbool.h>
#include <stdint.h>

#include "sensor_port.h"

/** Give up and latch an error after this many consecutive read failures. */
#define CLIMATE_MAX_CONSECUTIVE_ERRORS 3

struct climate_service {
	struct climate_raw last;
	bool alarm;
	bool faulted;
	uint32_t reads;
	uint32_t errors;
	uint8_t consecutive_errors;
};

/** Zero the service. Does not touch the ports. */
void climate_service_init(struct climate_service *svc);

/**
 * One cycle: read, decide, announce.
 *
 * @retval 0        a sample was taken and acted on
 * @retval negative whatever sensor_port_read() returned
 * @retval -EIO     already faulted; no read was attempted
 */
int climate_service_tick(struct climate_service *svc);

/** Clear a latched fault and the consecutive error count. */
void climate_service_clear_fault(struct climate_service *svc);

#endif /* CLIMATE_SERVICE_H_ */
