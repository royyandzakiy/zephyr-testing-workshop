// tests/test_logic.cpp
//
// Plain GoogleTest, no mocks. The same assertions as apps/06-sensor's
// tests/unit, so put the two files next to each other and read what the
// framework buys you.
//
// The short version: ztest has no parametrization, so a table of cases becomes
// one test that stops at the first failure. GoogleTest has TEST_P, so the same
// table becomes one result per row with a name attached.

#include <cctype>
#include <ostream>
#include <initializer_list>
#include <string>
#include <vector>

// EXPECT_THAT and the matchers below live in gmock, not gtest, even when no
// mock object is involved. gmock_main links both, so the include is all you
// need.
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "climate/logic.hpp"

namespace {

using climate::alarmState;
using climate::toMilli;

// --------------------------------------------------------------------------
// Tests that never run.
//
// Both functions in logic.hpp are `constexpr`, so the compiler can evaluate
// them. That makes these assertions build errors rather than test failures,
// and they cost nothing at run time because there is no run time involved.
//
// The limit is that a static_assert reports one failure and stops, and it
// cannot tell you "9 cases ran, case 4 disagreed". Use it for the handful of
// properties you never want to compile without, and TEST_P below for the
// table.
// --------------------------------------------------------------------------

static_assert(toMilli(25, 0) == 25000);
static_assert(toMilli(25, 500500) == 25501, "halves round away from zero");
static_assert(toMilli(-25, -500500) == -25501, "and that includes negatives");
static_assert(toMilli(0, 999999) == 1000, "truncating here would give 999");

static_assert(!alarmState(20000, 40000, false), "cold and dry stays off");
static_assert(alarmState(climate::kTempOnMilliC, 40000, false), "trips on the point");
static_assert(alarmState(29000, 40000, true), "inside the band it holds");
static_assert(!alarmState(climate::kTempOffMilliC, 40000, true), "releases on the point");

// --------------------------------------------------------------------------
// TEST: the plain one. Suite name, test name, body.
// --------------------------------------------------------------------------

TEST(ToMilli, WholeUnitsPassThrough) {
    EXPECT_EQ(toMilli(25, 0), 25000);
}

TEST(ToMilli, RoundsHalfUp) {
    EXPECT_EQ(toMilli(25, 500500), 25501);
}

TEST(ToMilli, RoundsNegativeHalfAwayFromZero) {
    EXPECT_EQ(toMilli(-25, -500500), -25501);
}

TEST(ToMilli, TruncationWouldLoseThis) {
    // 999999 micro-units is 1000 milli-units once rounded, not 999.
    EXPECT_EQ(toMilli(0, 999999), 1000);
}

// EXPECT_ vs ASSERT_ is the first thing to get right and the most commonly
// missed. EXPECT_ records the failure and keeps going, so one run tells you
// about all four problems. ASSERT_ returns from the function immediately, and
// you use it when continuing would crash or would be meaningless.
TEST(ToMilli, ExpectKeepsGoingWhereAssertWouldNot) {
    EXPECT_EQ(toMilli(1, 0), 1000);
    EXPECT_EQ(toMilli(2, 0), 2000);
    EXPECT_EQ(toMilli(3, 0), 3000);
}

// --------------------------------------------------------------------------
// TEST_P: one body, a table of cases, and one result per row.
// --------------------------------------------------------------------------

struct AlarmCase {
    const char* name;
    std::int32_t temp_mc;
    std::int32_t hum_mrh;
    bool prev;
    bool want;
};

// Teach GoogleTest how to print an AlarmCase.
//
// Without this it falls back to a hex dump of the struct, and you get lines
// like `GetParam() = 24-byte object <93-02 10-2E ...>` in the listing. CMake's
// gtest_discover_tests uses exactly that string as the ctest test name, so a
// missing PrintTo is how you end up with a ctest report nobody can read.
// Found by running `ctest -N` and looking at it, which is worth doing once
// for any suite you inherit.
void PrintTo(const AlarmCase& c, std::ostream* os) { *os << c.name; }

class AlarmHysteresis : public testing::TestWithParam<AlarmCase> {};

TEST_P(AlarmHysteresis, MatchesTheTruthTable) {
    const AlarmCase& c = GetParam();

    EXPECT_EQ(alarmState(c.temp_mc, c.hum_mrh, c.prev), c.want)
        << "temp=" << c.temp_mc << " hum=" << c.hum_mrh << " prev=" << c.prev;
}

INSTANTIATE_TEST_SUITE_P(
    Thresholds, AlarmHysteresis,
    testing::Values(
        AlarmCase{"cold and dry stays off", 20000, 40000, false, false},
        AlarmCase{"exactly on the temp trip", climate::kTempOnMilliC, 40000, false, true},
        AlarmCase{"one below the temp trip", climate::kTempOnMilliC - 1, 40000, false, false},
        AlarmCase{"humidity alone can trip it", 20000, climate::kHumOnMilliRh, false, true},
        AlarmCase{"inside the band stays latched", 29000, 40000, true, true},
        AlarmCase{"exactly on the release point", climate::kTempOffMilliC, 40000, true, false},
        AlarmCase{"one above the release point", climate::kTempOffMilliC + 1, 40000, true, true},
        AlarmCase{"humidity alone holds it", 20000, climate::kHumOffMilliRh + 1, true, true},
        AlarmCase{"both below releases", 20000, 40000, true, false}),
    [](const testing::TestParamInfo<AlarmCase>& info) {
        // Without this lambda the cases are named Thresholds/AlarmHysteresis.0
        // through .8, and a red CI log tells you nothing. Worth the five lines
        // every single time.
        std::string s = info.param.name;
        for (char& ch : s) {
            if (!std::isalnum(static_cast<unsigned char>(ch))) {
                ch = '_';
            }
        }
        return s;
    });

// --------------------------------------------------------------------------
// TEST_F: a fixture, for setup shared across tests.
// --------------------------------------------------------------------------

class AlarmWalk : public testing::Test {
protected:
    void SetUp() override { state_ = false; }

    // Feed a series of temperatures through the hysteresis and record every
    // state it passed through.
    std::vector<bool> walk(std::initializer_list<std::int32_t> temps) {
        std::vector<bool> out;
        for (std::int32_t t : temps) {
            state_ = alarmState(t, 40000, state_);
            out.push_back(state_);
        }
        return out;
    }

    bool state_{false};
};

TEST_F(AlarmWalk, DoesNotChatterOnTheThreshold) {
    // Sitting exactly on the trip point is the case hysteresis exists for.
    const auto seen = walk({30000, 30000, 30000, 30000});

    EXPECT_THAT(seen, testing::Each(true));
}

TEST_F(AlarmWalk, RisesAndFallsOnce) {
    const auto seen = walk({20000, 31000, 29000, 27000, 20000});

    EXPECT_THAT(seen, testing::ElementsAre(false, true, true, false, false));
}

}  // namespace
