// src/service.cpp

#include "climate/service.hpp"

#include <cerrno>

#include "climate/logic.hpp"

namespace climate {

Service::Service(ISensorPort& sensor, IAlarmPort& alarm)
    : sensor_(sensor), alarm_port_(alarm) {}

void Service::clearFault() {
    faulted_ = false;
    consecutive_errors_ = 0;
}

int Service::tick() {
    if (faulted_) {
        return -EIO;
    }

    Reading raw{};
    const int ret = sensor_.read(raw);

    if (ret != 0) {
        ++errors_;
        ++consecutive_errors_;

        if (consecutive_errors_ >= kMaxConsecutiveErrors) {
            faulted_ = true;
        }

        // Deliberately leaves the alarm alone. A sensor that stopped
        // answering is not a sensor saying everything is fine.
        return ret;
    }

    consecutive_errors_ = 0;
    ++reads_;
    last_ = raw;

    const bool next = alarmState(raw.temp_mc, raw.hum_mrh, alarm_);

    // Edge only. This is the line the mock test is really about.
    if (next != alarm_) {
        alarm_ = next;
        alarm_port_.set(next);
    }

    return 0;
}

}  // namespace climate
