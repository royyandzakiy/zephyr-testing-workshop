// include/climate/logic.hpp
//
// The same thresholds and the same two functions as apps/06-sensor and
// apps/08-fff-mocks, in C++.
//
// Both functions are `constexpr`, which is not decoration. It means the
// compiler can evaluate them, so some of the truth table in
// tests/gtest/src/test_logic.cpp is checked by `static_assert` at build time
// and never runs at all. A test that cannot fail at run time because it
// already failed the build is the cheapest test there is.

#pragma once

#include <cstdint>
#include <utility>

namespace climate {

enum class Threshold : std::int32_t {
    TempOnMilliC = 30000,   // 30.0 degC
    TempOffMilliC = 28000,  // 28.0 degC
    HumOnMilliRh = 70000,   // 70.0 %RH
    HumOffMilliRh = 65000,  // 65.0 %RH
};

// std::to_underlying is C++23, from <utility>. CONFIG_STD_CPP2B is what puts
// it on the menu. Before C++23 this was static_cast<std::int32_t>(t), which
// works and silently compiles even when you name the wrong type.
[[nodiscard]] constexpr std::int32_t value_of(Threshold t) noexcept
{
    return std::to_underlying(t);
}

inline constexpr std::int32_t kTempOnMilliC = value_of(Threshold::TempOnMilliC);
inline constexpr std::int32_t kTempOffMilliC = value_of(Threshold::TempOffMilliC);
inline constexpr std::int32_t kHumOnMilliRh = value_of(Threshold::HumOnMilliRh);
inline constexpr std::int32_t kHumOffMilliRh = value_of(Threshold::HumOffMilliRh);

/// Collapse a Zephyr-style (val1, val2/1e6) pair into milli-units, rounding
/// to nearest, halves away from zero.
[[nodiscard]] constexpr std::int32_t toMilli(std::int32_t val1, std::int32_t val2) noexcept
{
    std::int32_t milli = val2 / 1000;
    const std::int32_t rem = val2 % 1000;

    if (rem >= 500) {
        milli += 1;
    } else if (rem <= -500) {
        milli -= 1;
    }

    return val1 * 1000 + milli;
}

/// Should the alarm be on, given this reading and its own previous state?
[[nodiscard]] constexpr bool alarmState(std::int32_t temp_mc, std::int32_t hum_mrh,
                                        bool prev) noexcept
{
    if (prev) {
        return temp_mc > value_of(Threshold::TempOffMilliC) ||
               hum_mrh > value_of(Threshold::HumOffMilliRh);
    }

    return temp_mc >= value_of(Threshold::TempOnMilliC) ||
           hum_mrh >= value_of(Threshold::HumOnMilliRh);
}

}  // namespace climate
