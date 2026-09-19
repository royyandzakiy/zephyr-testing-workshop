// tests/gtest/src/test_dispenser.cpp
//
// The same dispenser as apps/08-fff-mocks, and the same four questions. Open
// that file next to this one.
//
// FFF:   call the code, then read auger_run_fake.call_count and assert on it.
// gmock: say what you expect BEFORE calling, and gmock checks it for you when
//        the mock is destroyed. An unmet EXPECT_CALL fails the test on its own,
//        with no assertion written anywhere.
//
// That is the whole trade. gmock catches the call you forgot to assert on;
// FFF is a header with no C++ runtime, no exceptions and no RTTI behind it.

#include <cerrno>
#include <cstdint>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "feeder/dispenser.hpp"
#include "feeder/portion.hpp"

namespace {

using testing::_;
using testing::Return;

class MockAuger : public feeder::IAuger {
public:
    MOCK_METHOD(int, run, (std::uint16_t grams), (override));
};

TEST(Dispenser, SplitsAPortionIntoWholeTurns)
{
    MockAuger auger;

    // Three turns, each asked for a full turn's worth. The argument is part of
    // the expectation: run(100) three times would not match and the test would
    // fail without a single EXPECT_EQ in the body.
    EXPECT_CALL(auger, run(feeder::kGramsPerTurn)).Times(3).WillRepeatedly(Return(0));

    feeder::Dispenser dispenser(auger);

    EXPECT_EQ(dispenser.feed(750), 0);
    EXPECT_EQ(dispenser.dispensed(), 750u);
}

TEST(Dispenser, ZeroGramsNeverReachesTheMotor)
{
    MockAuger auger;

    // The assertion you cannot make by watching a motor, in one line.
    EXPECT_CALL(auger, run(_)).Times(0);

    feeder::Dispenser dispenser(auger);

    EXPECT_EQ(dispenser.feed(0), -EINVAL);
}

TEST(Dispenser, AJamStopsTheRemainingTurns)
{
    MockAuger auger;

    // One answer per call, in order. Two WillOnce clauses also set the
    // expected count, so a third call would fail on its own.
    EXPECT_CALL(auger, run(feeder::kGramsPerTurn))
        .WillOnce(Return(0))
        .WillOnce(Return(-EIO));

    feeder::Dispenser dispenser(auger);

    EXPECT_EQ(dispenser.feed(750), -EIO);
    EXPECT_EQ(dispenser.dispensed(), 250u) << "counted pellets that never left the hopper";
    EXPECT_EQ(dispenser.jams(), 1u);
}

}  // namespace
