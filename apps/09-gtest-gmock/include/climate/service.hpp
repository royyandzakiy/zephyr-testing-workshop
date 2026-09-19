// include/climate/service.hpp
//
// Same behaviour as apps/08-fff-mocks/src/climate_service.c, line for line.
// Read them side by side; the only interesting difference is that the ports
// arrive through the constructor instead of through the linker.

#pragma once

#include <cstdint>

#include "climate/ports.hpp"
#include "climate/reading.hpp"

namespace climate {

class Service {
public:
    static constexpr int kMaxConsecutiveErrors = 3;

    /// Constructor injection. References, not pointers, because a Service
    /// without ports is not a thing that should be constructible.
    Service(ISensorPort& sensor, IAlarmPort& alarm);

    /// One cycle: read, decide, announce.
    /// @return 0, or the negative errno the sensor returned, or -EIO if the
    ///         service has already latched a fault.
    int tick();

    void clearFault();

    bool alarm() const { return alarm_; }
    bool faulted() const { return faulted_; }
    std::uint32_t reads() const { return reads_; }
    std::uint32_t errors() const { return errors_; }
    const Reading& last() const { return last_; }

private:
    ISensorPort& sensor_;
    IAlarmPort& alarm_port_;

    Reading last_{};
    bool alarm_{false};
    bool faulted_{false};
    std::uint32_t reads_{0};
    std::uint32_t errors_{0};
    int consecutive_errors_{0};
};

}  // namespace climate
