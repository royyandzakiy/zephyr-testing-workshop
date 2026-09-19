// include/climate/ports.hpp
//
// The same two seams as apps/08-fff-mocks, expressed the way C++ does it:
// abstract base classes with pure virtual functions.
//
// In C the seam was a header with no implementation attached, and the
// substitution happened at link time. Here it happens at run time through a
// vtable. Two different implementations can be alive in the same binary, and
// each test can hand the service a different one. FFF cannot do that, because
// a program has exactly one symbol called sensor_port_read.
//
// The cost is a vtable pointer per object and an indirect call per use. On a
// Cortex-M4 that is usually nothing. In a 1 kHz control loop it might not be.
// Measure before assuming either way.

#pragma once

#include <concepts>

#include "climate/reading.hpp"

namespace climate {

class ISensorPort {
public:
    virtual ~ISensorPort() = default;

    [[nodiscard]] virtual bool ready() const = 0;

    /// @return 0 on success and @p out is filled, negative errno otherwise.
    [[nodiscard]] virtual int read(Reading& out) = 0;
};

class IAlarmPort {
public:
    virtual ~IAlarmPort() = default;

    virtual void set(bool on) = 0;
};

// A concept saying what the service needs from a sensor, independent of the
// base class above. Both the real implementations and the gmock mocks satisfy
// it, so the static_assert in service.hpp catches a broken override at compile
// time with a readable message instead of the usual wall of template errors.
template <typename T>
concept SensorLike = requires(T t, Reading& r) {
    { t.ready() } -> std::convertible_to<bool>;
    { t.read(r) } -> std::convertible_to<int>;
};

template <typename T>
concept AlarmLike = requires(T t, bool on) {
    { t.set(on) } -> std::same_as<void>;
};

static_assert(SensorLike<ISensorPort>);
static_assert(AlarmLike<IAlarmPort>);

}  // namespace climate
