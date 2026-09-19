// tests/gtest/src/test_portion.cpp
//
// Plain GoogleTest, no mocks. A table of cases, which is the first thing
// GoogleTest gives you that ztest does not have.
//
// In ztest a table is one test with a loop inside it, so it stops at the first
// bad row and the report says one thing failed. TEST_P makes each row its own
// result with its own name, so a run tells you every row that is wrong at once.

#include <cstdint>
#include <string>

#include <gtest/gtest.h>

#include "feeder/portion.hpp"

namespace {

// turnsFor is constexpr, so these are checked while the file compiles and
// never run. A test that already failed the build is the cheapest one there is.
static_assert(feeder::turnsFor(0) == 0);
static_assert(feeder::turnsFor(250) == 1);
static_assert(feeder::turnsFor(251) == 2);
static_assert(feeder::turnsFor(feeder::gramsOf(feeder::Portion::Large)) == 3);

struct TurnsCase {
    std::uint16_t grams;
    std::uint8_t expected;
    const char* name;
};

const TurnsCase kCases[] = {
    {0, 0, "NothingAsked"},
    {1, 1, "OnePelletStillCostsAWholeTurn"},
    {250, 1, "ExactlyOneTurn"},
    {251, 2, "OneOverRoundsUp"},
    {feeder::gramsOf(feeder::Portion::Large), 3, "ALargePortion"},
};

class TurnsFor : public testing::TestWithParam<TurnsCase> {};

TEST_P(TurnsFor, MatchesTheTable)
{
    const TurnsCase& c = GetParam();

    EXPECT_EQ(feeder::turnsFor(c.grams), c.expected);
}

INSTANTIATE_TEST_SUITE_P(Portion, TurnsFor, testing::ValuesIn(kCases),
                         [](const testing::TestParamInfo<TurnsCase>& info) {
                             return std::string(info.param.name);
                         });

}  // namespace
