// include/climate/reading.hpp
#pragma once

#include <cstdint>

namespace climate {

struct Reading {
    std::int32_t temp_mc{};
    std::int32_t press_pa{};
    std::int32_t hum_mrh{};
};

}  // namespace climate
