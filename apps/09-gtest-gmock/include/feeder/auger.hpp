// The auger seam from apps/08-fff-mocks/src/auger_port.h as a C++ interface.
//
// FFF substitutes at link time, so a binary has exactly one auger_run(). A
// virtual call substitutes at run time, so each test can hand the dispenser its
// own auger. The cost, a vtable pointer per object and an indirect call per use,
// is usually negligible on a Cortex-M4; measure before assuming either way.

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
