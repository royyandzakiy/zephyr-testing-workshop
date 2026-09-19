// tests/test_service.cpp
//
// GoogleMock, against the same Service that apps/08-fff-mocks tests with FFF.
// The two suites assert the same things, so the difference you are looking at
// is purely the framework.
//
// The big one: FFF records and you assert afterwards. gmock declares the
// expectation up front with EXPECT_CALL, and the mock itself fails the test at
// destruction if the expectation was not met. That reads better, and it has a
// real cost attached, which the StrictMock and NiceMock section below is
// about.

#include <cerrno>
#include <vector>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "climate/logic.hpp"
#include "climate/service.hpp"

namespace {

using climate::Reading;
using climate::Service;

using testing::_;
using testing::AtLeast;
using testing::DoAll;
using testing::Field;
using testing::InSequence;
using testing::IsTrue;
using testing::NiceMock;
using testing::Return;
using testing::SetArgReferee;
using testing::StrictMock;

// --------------------------------------------------------------------------
// The mocks. Four lines each, and MOCK_METHOD generates the override, the
// expectation plumbing and the failure reporting.
// --------------------------------------------------------------------------

class MockSensorPort : public climate::ISensorPort {
public:
    MOCK_METHOD(bool, ready, (), (const, override));
    MOCK_METHOD(int, read, (Reading & out), (override));
};

class MockAlarmPort : public climate::IAlarmPort {
public:
    MOCK_METHOD(void, set, (bool on), (override));
};

Reading quiet() { return Reading{25000, 100000, 45000}; }
Reading hot() { return Reading{climate::kTempOnMilliC + 500, 100000, 45000}; }
Reading cool() { return Reading{climate::kTempOffMilliC - 500, 100000, 40000}; }

// --------------------------------------------------------------------------
// 1. The basic shape: expectation, action, verification on destruction.
// --------------------------------------------------------------------------

TEST(ServiceMock, TickReadsTheSensorExactlyOnce) {
    MockSensorPort sensor;
    NiceMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    // SetArgReferee<0> writes the out-parameter; Return(0) is the return
    // value. DoAll chains them. This pair is the gmock answer to the custom
    // fake you had to hand-write in apps/08-fff-mocks.
    EXPECT_CALL(sensor, read(_))
        .Times(1)
        .WillOnce(DoAll(SetArgReferee<0>(quiet()), Return(0)));

    EXPECT_EQ(svc.tick(), 0);

    // No assertion needed at the end. `sensor` verifies itself when it goes
    // out of scope, and reports the failure against this test.
}

TEST(ServiceMock, QuietReadingsNeverTouchTheAlarm) {
    NiceMock<MockSensorPort> sensor;
    StrictMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    ON_CALL(sensor, read(_)).WillByDefault(DoAll(SetArgReferee<0>(quiet()), Return(0)));

    for (int i = 0; i < 5; ++i) {
        EXPECT_EQ(svc.tick(), 0);
    }

    // No EXPECT_CALL on `alarm` at all. Because it is a StrictMock, any call
    // to set() is an immediate failure. That is a stronger statement than
    // FFF's `zassert_equal(call_count, 0)`: it fails at the moment of the
    // call, with a stack trace, instead of at the end with a number.
}

// --------------------------------------------------------------------------
// 2. Argument matchers and call counts.
// --------------------------------------------------------------------------

TEST(ServiceMock, AlarmIsRaisedWithTrueWhenItGetsHot) {
    NiceMock<MockSensorPort> sensor;
    MockAlarmPort alarm;
    Service svc(sensor, alarm);

    ON_CALL(sensor, read(_)).WillByDefault(DoAll(SetArgReferee<0>(hot()), Return(0)));

    EXPECT_CALL(alarm, set(true)).Times(1);

    EXPECT_EQ(svc.tick(), 0);
}

TEST(ServiceMock, AlarmIsDrivenOnlyOnTheEdge) {
    NiceMock<MockSensorPort> sensor;
    MockAlarmPort alarm;
    Service svc(sensor, alarm);

    ON_CALL(sensor, read(_)).WillByDefault(DoAll(SetArgReferee<0>(hot()), Return(0)));

    // Four hot ticks, one call. Get the edge detection wrong and this fails
    // on the second tick, not at the end of the test.
    EXPECT_CALL(alarm, set(_)).Times(1);

    for (int i = 0; i < 4; ++i) {
        EXPECT_EQ(svc.tick(), 0);
    }
}

TEST(ServiceMock, AlarmGoesOnThenOffInThatOrder) {
    NiceMock<MockSensorPort> sensor;
    MockAlarmPort alarm;
    Service svc(sensor, alarm);

    // InSequence: ordering is part of the expectation. In FFF you would read
    // arg0_history[0] and arg0_history[1] and compare. Here the ordering is
    // the assertion.
    {
        InSequence seq;
        EXPECT_CALL(alarm, set(true));
        EXPECT_CALL(alarm, set(false));
    }

    EXPECT_CALL(sensor, read(_))
        .WillOnce(DoAll(SetArgReferee<0>(hot()), Return(0)))
        .WillOnce(DoAll(SetArgReferee<0>(cool()), Return(0)));

    EXPECT_EQ(svc.tick(), 0);
    EXPECT_EQ(svc.tick(), 0);
}

TEST(ServiceMock, HysteresisHoldsBetweenTheThresholds) {
    NiceMock<MockSensorPort> sensor;
    MockAlarmPort alarm;
    Service svc(sensor, alarm);

    const Reading between{(climate::kTempOnMilliC + climate::kTempOffMilliC) / 2,
                          100000, 40000};

    EXPECT_CALL(sensor, read(_))
        .WillOnce(DoAll(SetArgReferee<0>(hot()), Return(0)))
        .WillOnce(DoAll(SetArgReferee<0>(between), Return(0)));

    EXPECT_CALL(alarm, set(true)).Times(1);
    EXPECT_CALL(alarm, set(false)).Times(0);

    svc.tick();
    svc.tick();

    EXPECT_TRUE(svc.alarm());
}

// --------------------------------------------------------------------------
// 3. Making the dependency misbehave.
// --------------------------------------------------------------------------

TEST(ServiceMock, ReadErrorIsCountedAndReturned) {
    MockSensorPort sensor;
    StrictMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    EXPECT_CALL(sensor, read(_)).WillOnce(Return(-EIO));

    EXPECT_EQ(svc.tick(), -EIO);
    EXPECT_EQ(svc.errors(), 1u);
    EXPECT_EQ(svc.reads(), 0u);
}

TEST(ServiceMock, ReadErrorDoesNotClearARaisedAlarm) {
    NiceMock<MockSensorPort> sensor;
    MockAlarmPort alarm;
    Service svc(sensor, alarm);

    EXPECT_CALL(sensor, read(_))
        .WillOnce(DoAll(SetArgReferee<0>(hot()), Return(0)))
        .WillOnce(Return(-EIO));

    EXPECT_CALL(alarm, set(true)).Times(1);
    EXPECT_CALL(alarm, set(false)).Times(0);

    EXPECT_EQ(svc.tick(), 0);
    EXPECT_EQ(svc.tick(), -EIO);

    EXPECT_TRUE(svc.alarm());
}

TEST(ServiceMock, ThreeConsecutiveErrorsLatchAFault) {
    MockSensorPort sensor;
    NiceMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    // Exactly three. The fourth tick must not reach the sensor, and Times(3)
    // is what proves it: a fourth call fails the test the moment it happens.
    EXPECT_CALL(sensor, read(_)).Times(3).WillRepeatedly(Return(-EIO));

    for (int i = 0; i < Service::kMaxConsecutiveErrors; ++i) {
        EXPECT_EQ(svc.tick(), -EIO);
    }

    EXPECT_TRUE(svc.faulted());
    EXPECT_EQ(svc.tick(), -EIO);
}

TEST(ServiceMock, AGoodReadResetsTheConsecutiveCounter) {
    NiceMock<MockSensorPort> sensor;
    NiceMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    EXPECT_CALL(sensor, read(_))
        .WillOnce(Return(-EIO))
        .WillOnce(Return(-EIO))
        .WillOnce(DoAll(SetArgReferee<0>(quiet()), Return(0)))
        .WillOnce(Return(-EIO));

    for (int i = 0; i < 4; ++i) {
        svc.tick();
    }

    EXPECT_FALSE(svc.faulted());
    EXPECT_EQ(svc.errors(), 3u);
}

TEST(ServiceMock, ClearingAFaultLetsReadsResume) {
    NiceMock<MockSensorPort> sensor;
    NiceMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    EXPECT_CALL(sensor, read(_))
        .WillOnce(Return(-EIO))
        .WillOnce(Return(-EIO))
        .WillOnce(Return(-EIO))
        .WillOnce(DoAll(SetArgReferee<0>(quiet()), Return(0)));

    for (int i = 0; i < Service::kMaxConsecutiveErrors; ++i) {
        svc.tick();
    }
    ASSERT_TRUE(svc.faulted());

    svc.clearFault();

    EXPECT_EQ(svc.tick(), 0);
    EXPECT_EQ(svc.reads(), 1u);
}

// --------------------------------------------------------------------------
// 4. NiceMock, naggy and strict: the knob most people never touch.
// --------------------------------------------------------------------------

TEST(ServiceMock, PlainMockNagsAboutUninterestingCalls) {
    // A bare MockSensorPort with no EXPECT_CALL prints a GMOCK WARNING for
    // every call and still passes. That default is "naggy", and it is why a
    // large gmock suite ends up with hundreds of warnings nobody reads.
    //
    // NiceMock silences them. StrictMock turns them into failures. Pick per
    // mock, based on whether that dependency is what the test is about:
    //   - the mock you are asserting on   -> plain or StrictMock
    //   - a dependency that just has to exist -> NiceMock
    NiceMock<MockSensorPort> sensor;
    NiceMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    ON_CALL(sensor, read(_)).WillByDefault(DoAll(SetArgReferee<0>(quiet()), Return(0)));

    for (int i = 0; i < 3; ++i) {
        EXPECT_EQ(svc.tick(), 0);
    }

    EXPECT_EQ(svc.reads(), 3u);
}

// --------------------------------------------------------------------------
// 5. A matcher on a struct field, and the limit of all of this.
// --------------------------------------------------------------------------

TEST(ServiceMock, StoresTheReadingItWasGiven) {
    NiceMock<MockSensorPort> sensor;
    NiceMock<MockAlarmPort> alarm;
    Service svc(sensor, alarm);

    const Reading r{26123, 99000, 51000};
    EXPECT_CALL(sensor, read(_)).WillOnce(DoAll(SetArgReferee<0>(r), Return(0)));

    svc.tick();

    // Field() composes matchers over members, which beats three separate
    // EXPECT_EQ lines because a failure prints the whole object.
    EXPECT_THAT(svc.last(), Field(&Reading::temp_mc, 26123));
    EXPECT_THAT(svc.last(), Field(&Reading::hum_mrh, 51000));
}

TEST(ServiceMock, MocksSayNothingAboutTheRealImplementation) {
    // Same caveat as apps/08-fff-mocks, and it is worth repeating in a
    // different language: every test in this file passes with no hardware
    // anywhere, which means none of them can tell you the BME280 is on the
    // right I2C address, or that it is soldered on at all.
    //
    // apps/06-sensor answers that half. This app answers the other half very
    // quickly and in a debugger you already know how to use.
    NiceMock<MockSensorPort> sensor;
    NiceMock<MockAlarmPort> alarm;

    EXPECT_CALL(sensor, ready()).Times(AtLeast(1)).WillRepeatedly(Return(true));

    EXPECT_THAT(sensor.ready(), IsTrue());
}

}  // namespace
