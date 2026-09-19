// include/feeder/auger.hpp
//
// The same seam as apps/08-fff-mocks/src/auger_port.h, expressed the way C++
// does it: an abstract base class with a pure virtual function.
//
// In C the seam was a header with no implementation attached and the
// substitution happened at link time. Here it happens at run time through a
// vtable, so two implementations can be alive in the same binary and each test
// can hand the dispenser a different one. FFF cannot do that, because a
// program has exactly one symbol called auger_run.
//
// The cost is a vtable pointer per object and an indirect call per use. On a
// Cortex-M4 that is usually nothing. Measure before assuming either way.

#pragma once

#include <cstdint>

namespace feeder {

class IAuger {
public:
    virtual ~IAuger() = default;

    /// One turn of the auger, pushing out roughly @p grams of pellets.
    /// @return 0, or -EIO if it jammed.
    [[nodiscard]] virtual int run(std::uint16_t grams) = 0;
};

}  // namespace feeder
