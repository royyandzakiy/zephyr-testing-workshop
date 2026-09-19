// include/climate/ports.hpp
//
// The same two seams as apps/08-fff-mocks, expressed the way C++ does it:
// abstract base classes with pure virtual functions.
//
// In C the seam was a header with no implementation attached, and the
// substitution happened at link time. Here it happens at run time through a
// vtable, and the consequence is worth stating plainly: you can have two
// different implementations alive in the same binary, and you can hand a
// different one to each test. FFF cannot do that, because there is exactly
// one symbol called sensor_port_read() in a program.
//
// The cost is a vtable pointer per object and an indirect call per use. On a
// Cortex-M4 that is usually nothing; in a 1 kHz control loop it might not be.
// Measure before you assume either way.

#pragma once

#include "climate/reading.hpp"

namespace climate {

class ISensorPort {
public:
    virtual ~ISensorPort() = default;

    virtual bool ready() const = 0;

    /// @return 0 on success and @p out is filled, negative errno otherwise.
    virtual int read(Reading& out) = 0;
};

class IAlarmPort {
public:
    virtual ~IAlarmPort() = default;

    virtual void set(bool on) = 0;
};

}  // namespace climate
