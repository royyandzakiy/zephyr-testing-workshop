// The same questions as apps/08-fff-mocks/tests/fff/src/main.c, asked with gmock.
//
// FFF:   call the code, then assert on auger_run_fake.call_count.
// gmock: state the expectation before the call. An unmet EXPECT_CALL fails the
//        test when the mock is destroyed, with no assertion written.
//
// gmock catches the call you forgot to assert on; FFF needs no C++ runtime,
// exceptions or RTTI.

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

    // The argument is part of the expectation: three calls to run(100) would
    // not match, and the test would fail without any EXPECT_EQ.
    EXPECT_CALL(auger, run(feeder::kGramsPerTurn)).Times(3).WillRepeatedly(Return(0));

    feeder::Dispenser dispenser(auger);

    EXPECT_EQ(dispenser.feed(750), 0);
    EXPECT_EQ(dispenser.dispensed(), 750u);
}

TEST(Dispenser, ZeroGramsNeverReachesTheMotor)
{
    MockAuger auger;

    // No call to run() at all, which watching a motor cannot prove.
    EXPECT_CALL(auger, run(_)).Times(0);

    feeder::Dispenser dispenser(auger);

    EXPECT_EQ(dispenser.feed(0), -EINVAL);
}

TEST(Dispenser, AJamStopsTheRemainingTurns)
{
    MockAuger auger;

    // One answer per call, in order. Two WillOnce clauses also set the expected
    // count, so a third call would fail on its own.
    EXPECT_CALL(auger, run(feeder::kGramsPerTurn))
        .WillOnce(Return(0))
        .WillOnce(Return(-EIO));

    feeder::Dispenser dispenser(auger);

    EXPECT_EQ(dispenser.feed(750), -EIO);
    EXPECT_EQ(dispenser.dispensed(), 250u) << "counted pellets that never left the hopper";
    EXPECT_EQ(dispenser.jams(), 1u);
}

}  // namespace
