// Portion sizes and the turn arithmetic over them. constexpr so
// tests/gtest/src/test_portion.cpp can check part of its table with static_assert.

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

/// std::to_underlying is C++23 (CONFIG_STD_CPP2B). It always yields the enum's
/// own underlying type, where static_cast compiles silently with the wrong one.
[[nodiscard]] constexpr std::uint16_t gramsOf(Portion p) noexcept
{
    return std::to_underlying(p);
}

/// How many turns to get at least @p grams out.
///
/// Rounds up, because rounding down would shrink a 300 g portion to 250 g every time.
[[nodiscard]] constexpr std::uint8_t turnsFor(std::uint16_t grams,
                                              std::uint16_t per_turn = kGramsPerTurn) noexcept
{
    if (grams == 0 || per_turn == 0) {
        return 0;
    }

    return static_cast<std::uint8_t>((grams + per_turn - 1) / per_turn);
}

}  // namespace feeder
