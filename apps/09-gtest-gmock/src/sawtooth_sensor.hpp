// src/sawtooth_sensor.hpp
//
// A production implementation, not a test double. It walks temperature up and
// down so src/main.cpp has something to print.
//
// The distinction gets blurred constantly: a fake that ships is a simulator,
// and it is allowed to be wrong in ways a mock is not, because nobody is
// asserting on it.

#pragma once

#include <zephyr/sys/printk.h>

#include "climate/ports.hpp"

namespace climate {

class SawtoothSensor final : public ISensorPort {
public:
    [[nodiscard]] bool ready() const override { return true; }

    [[nodiscard]] int read(Reading& out) override
    {
        temp_mc_ += step_;
        hum_mrh_ += step_ * 3;
        if (temp_mc_ > 32000 || temp_mc_ < 24000) {
            step_ = -step_;
        }

        out.temp_mc = temp_mc_;
        out.hum_mrh = hum_mrh_;
        out.press_pa = 100650;
        return 0;
    }

private:
    std::int32_t temp_mc_{24000};
    std::int32_t hum_mrh_{55000};
    std::int32_t step_{500};
};

class LedAlarm final : public IAlarmPort {
public:
    void set(bool on) override { printk("alarm: %s\n", on ? "ON" : "OFF"); }
};

}  // namespace climate
