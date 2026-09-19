// include/climate/logic.hpp
//
// Free functions, no state, no dependencies. The C version of this file is
// apps/06-sensor/src/sensors/climate_logic.h and the behaviour is identical,
// which makes the two test suites directly comparable.

#pragma once

#include <cstdint>

namespace climate {

inline constexpr std::int32_t kTempOnMilliC = 30000;   // 30.0 degC
inline constexpr std::int32_t kTempOffMilliC = 28000;  // 28.0 degC
inline constexpr std::int32_t kHumOnMilliRh = 70000;   // 70.0 %RH
inline constexpr std::int32_t kHumOffMilliRh = 65000;  // 65.0 %RH

/// Collapse a Zephyr-style (val1, val2/1e6) pair into milli-units,
/// rounding to nearest.
std::int32_t toMilli(std::int32_t val1, std::int32_t val2);

/// Should the alarm be on, given this reading and its own previous state?
bool alarmState(std::int32_t temp_mc, std::int32_t hum_mrh, bool prev);

}  // namespace climate
