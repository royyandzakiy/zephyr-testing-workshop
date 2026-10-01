// Plain GoogleTest, no mocks. TEST(Suite, Name) does the job ZTEST does in
// apps/07-unit-conventions.

#include <cstdint>

#include <gtest/gtest.h>

#include "feeder/portion.hpp"

namespace {

// turnsFor is constexpr, so these are checked at compile time and fail the
// build instead of a test run.
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
    // Rounding down would shrink a 300 g portion to 250 g, every time.
    EXPECT_EQ(feeder::turnsFor(1), 1);
    EXPECT_EQ(feeder::turnsFor(feeder::kGramsPerTurn + 1), 2);
}

}  // namespace
