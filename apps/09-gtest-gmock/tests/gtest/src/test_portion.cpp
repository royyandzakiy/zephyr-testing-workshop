// tests/gtest/src/test_portion.cpp
//
// Plain GoogleTest, no mocks. `TEST(Suite, Name)` is the whole API here, and it
// is doing the same job ZTEST does in apps/07-unit-conventions.

#include <cstdint>

#include <gtest/gtest.h>

#include "feeder/portion.hpp"

namespace {

// turnsFor is constexpr, so these are checked while the file compiles and never
// run. A test that already failed the build is the cheapest one there is.
static_assert(feeder::turnsFor(0) == 0);
static_assert(feeder::turnsFor(250) == 1);
static_assert(feeder::turnsFor(251) == 2);
static_assert(feeder::turnsFor(feeder::gramsOf(feeder::Portion::Large)) == 3);

TEST(TurnsFor, NothingAskedIsNoTurns)
{
    EXPECT_EQ(feeder::turnsFor(0), 0);
}

TEST(TurnsFor, ExactMultiplesAreWholeTurns)
{
    EXPECT_EQ(feeder::turnsFor(feeder::kGramsPerTurn), 1);
    EXPECT_EQ(feeder::turnsFor(feeder::gramsOf(feeder::Portion::Large)), 3);
}

TEST(TurnsFor, AnyRemainderCostsAnotherTurn)
{
    // One pellet over a full turn is still a whole extra turn. Rounding down
    // would make a 300 g portion silently become 250 g, every time, forever.
    EXPECT_EQ(feeder::turnsFor(1), 1);
    EXPECT_EQ(feeder::turnsFor(feeder::kGramsPerTurn + 1), 2);
}

}  // namespace
