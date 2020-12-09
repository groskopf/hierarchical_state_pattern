// MIT License
//
// Copyright (c) 2020 Groskopf Embedded
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. declarationIN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "pump_control_hsm.h"

#include <gmock/gmock.h>

#include <assert.h>
#include <stdio.h>
#include <string>

using namespace PumpControl;

using ::testing::_;
using ::testing::Mock;
using ::testing::SaveArg;
using ::testing::Test;

namespace {

class PumpMock : public PumpControl::IPump {
public:
  MOCK_METHOD0(on, void());
  MOCK_METHOD0(off, void());
};

class TimerMock : public PumpControl::ITimer {
public:
  MOCK_METHOD1(start, void(std::function<void()> timeoutCallback));
  MOCK_METHOD0(cancel, void());
};

class PumpControlHsmTest : public Test {
public:
  PumpMock pumpMock;
  TimerMock runningTimerMock;
  TimerMock pausedTimerMock;

  PumpControlHsm pumpControlHsm; // DUT

  PumpControlHsmTest()
      : pumpControlHsm(pumpMock, runningTimerMock, pausedTimerMock) {}
};

} // namespace

TEST_F(PumpControlHsmTest, test) {

  // TODO add choice point and history states

  pumpControlHsm.onStart();

  EXPECT_CALL(pumpMock, on());
  pumpControlHsm.onContinuous();

  EXPECT_CALL(pumpMock, off());
  pumpControlHsm.onStandby();

  std::function<void()> timeoutCallback;
  EXPECT_CALL(pumpMock, on());
  EXPECT_CALL(runningTimerMock, start(_)).WillOnce(SaveArg<0>(&timeoutCallback));
  pumpControlHsm.onPulsing();

  EXPECT_CALL(pumpMock, off());
  EXPECT_CALL(pausedTimerMock, start(_)).WillOnce(SaveArg<0>(&timeoutCallback));
  timeoutCallback();

  EXPECT_CALL(pumpMock, on());
  EXPECT_CALL(runningTimerMock, start(_)).WillOnce(SaveArg<0>(&timeoutCallback));
  timeoutCallback();

  EXPECT_CALL(pumpMock, off());
  EXPECT_CALL(runningTimerMock, cancel());
  EXPECT_CALL(pausedTimerMock, cancel());
  pumpControlHsm.onStandby();
}
