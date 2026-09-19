// include/feeder/portion.hpp
//
// Portion sizes, and one pure function over them. `constexpr` so the compiler
// can evaluate it: part of the table in tests/gtest/src/test_portion.cpp is
// checked by static_assert while the file compiles, and never runs.

#pragma once

#include <cstdint>
#include <utility>

namespace feeder {

/// How much the auger shifts in one turn. On a real feeder this is fixed by
/// the screw pitch and the motor run time, not by what you asked for.
inline constexpr std::uint16_t kGramsPerTurn = 250;

/// The sizes the app offers. An enum class rather than loose constants, so a
/// portion cannot be passed where a gram count is expected.
enum class Portion : std::uint16_t {
    Small = 250,
    Medium = 500,
    Large = 750,
};

/// std::to_underlying is C++23, from <utility>, and CONFIG_STD_CPP2B is what
/// puts it on the menu. Before C++23 this was static_cast<std::uint16_t>(p),
/// which works and silently compiles even when you name the wrong type.
[[nodiscard]] constexpr std::uint16_t gramsOf(Portion p) noexcept
{
    return std::to_underlying(p);
}

/// How many turns to get at least @p grams out.
///
/// Rounds up, so the fish occasionally get a little extra. Rounding down would
/// mean a 300 g portion silently becomes 250 g, every time, forever.
[[nodiscard]] constexpr std::uint8_t turnsFor(std::uint16_t grams,
                                              std::uint16_t per_turn = kGramsPerTurn) noexcept
{
    if (grams == 0 || per_turn == 0) {
        return 0;
    }

    return static_cast<std::uint8_t>((grams + per_turn - 1) / per_turn);
}

}  // namespace feeder
