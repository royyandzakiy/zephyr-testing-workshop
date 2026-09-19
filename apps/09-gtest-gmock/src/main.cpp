// src/main.cpp
//
// The application. An ordinary Zephyr main(), written in C++, wiring the
// sawtooth sensor to the service and ticking once a second.
//
// This file is the composition root: the only place that knows both which
// port implementations exist and which service uses them. tests/gtest brings
// its own main and its own implementations, which is why the service never had
// to know.

#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "climate/service.hpp"
#include "sawtooth_sensor.hpp"

int main(void)
{
    printk("Climate service starting\n");

    climate::SawtoothSensor sensor;
    climate::LedAlarm alarm;
    climate::Service svc(sensor, alarm);

    if (!sensor.ready()) {
        printk("Error: sensor port not ready\n");
        return 0;
    }

    while (true) {
        const int ret = svc.tick();

        if (ret != 0) {
            printk("tick failed: %d (errors=%u)\n", ret, svc.errors());
        } else {
            printk("T: %d mC | P: %d Pa | H: %d m%%RH | ALARM %s\n",
                   svc.last().temp_mc, svc.last().press_pa, svc.last().hum_mrh,
                   svc.alarm() ? "ON" : "OFF");
        }

        k_sleep(K_SECONDS(1));
    }

    return 0;
}
