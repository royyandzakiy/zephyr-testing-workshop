// src/main.cpp
//
// A host binary, so `apps/09-gtest-gmock` is a project you can run and not
// just a test directory. There is no Zephyr here at all: no kernel, no
// devicetree, no board. That is the point of this app, and also its limit.

#include <cstdio>

#include "climate/service.hpp"
#include "sawtooth_sensor.hpp"

int main() {
    climate::SawtoothSensor sensor;
    climate::PrintingAlarm alarm;
    climate::Service svc(sensor, alarm);

    if (!sensor.ready()) {
        std::puts("sensor not ready");
        return 1;
    }

    for (int i = 0; i < 40; ++i) {
        if (svc.tick() != 0) {
            std::printf("tick failed (errors=%u)\n", svc.errors());
            continue;
        }

        std::printf("T: %d mC | H: %d m%%RH | ALARM %s\n",
                    svc.last().temp_mc, svc.last().hum_mrh,
                    svc.alarm() ? "ON" : "OFF");
    }

    return 0;
}
