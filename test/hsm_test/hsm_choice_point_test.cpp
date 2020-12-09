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
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "hsm.h"

#include <gmock/gmock.h>

#include <assert.h>
#include <stdio.h>
#include <string>

using std::cout;
using std::endl;
using std::string;

using hsp::Hsm;
using hsp::HsmState;

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::Mock;
using ::testing::StrEq;
using ::testing::Test;

//!
// This is a simple example of how to use the Hsm base class to create a hierarchical state machine
//
// @startuml
//
// state Top {
//   [*] --> Idle
//   Top --> Event:  Value [Is even]
//   Top --> Odd: Reset [Is odd]
// }
//
// @enduml
//

namespace {

class TransitionMock {
public:
  TransitionMock() {
    ON_CALL(*this, activate(_, _)).WillByDefault(Invoke([](const string &state, const string &event) { cout << state << " - " << event << endl; }));
  }
  MOCK_METHOD2(activate, void(const string &, const string &));
};

class HsmUnderTest;

class StateUnderTest : public HsmState<StateUnderTest> {
public:
  StateUnderTest(HsmUnderTest &hsm, HsmState *const super_state, const string &name);
  virtual ~StateUnderTest();

  void InvokeMock(const string &event) const;

  void onEnter() override { InvokeMock("ENTRY"); }
  void onExit() override { InvokeMock("EXIT"); }
  void onInit() override { InvokeMock("INIT"); }

  virtual bool onEventValue(int value) { return false; }

protected:
  HsmUnderTest &hsm;
  const string name;
};

class StateTop : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  void onInit() override;
  bool onEventValue(int value) override;
};

class StateResult : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;
};

class HsmUnderTest : public Hsm<StateUnderTest> {
public:
  explicit HsmUnderTest(TransitionMock &TransitionMock)
      : Hsm(top)
      , top(*this, nullptr, "TOP")
      , even(*this, &top, "EVEN")
      , odd(*this, &top, "ODD")
      , transitionMock(TransitionMock) {}

  bool onEventValue(int value) {
    return onEvent([&](StateUnderTest &state) { return state.onEventValue(value); });
  }

  TransitionMock &transitionMock;

private:
  bool hasHappened = false;
  StateTop top;
  StateResult even;
  StateResult odd;

  friend StateTop;
  friend StateResult;
};

StateUnderTest::StateUnderTest(HsmUnderTest &hsm, HsmState *const super_state, const string &name)
    : HsmState<StateUnderTest>(super_state)
    , hsm(hsm)
    , name(name) {}

StateUnderTest::~StateUnderTest() {}

void StateUnderTest::InvokeMock(const string &event) const { hsm.transitionMock.activate(name, event); }

void StateTop::onInit() {
  InvokeMock("INIT");
  hsm.initialTransition(hsm.even);
}

bool StateTop::onEventValue(int value) {
  InvokeMock("VALUE");
  // Choice point
  if (0 == (value % 2)) {
    hsm.transition(hsm.even);
  } else {
    hsm.transition(hsm.odd);
  }
  return true;
}

class HsmChoicePointTest : public Test {
public:
  TransitionMock transitionMock;
  HsmUnderTest hsm_under_test; // DUT

  HsmChoicePointTest()
      : hsm_under_test(transitionMock) {}
};

} // namespace

TEST_F(HsmChoicePointTest, test) {
  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("TOP"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("TOP"), StrEq("INIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("EVEN"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("EVEN"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onStart();
  Mock::VerifyAndClearExpectations(&transitionMock);

  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("TOP"), StrEq("VALUE"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("EVEN"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ODD"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ODD"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onEventValue(1);
  Mock::VerifyAndClearExpectations(&transitionMock);

  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("TOP"), StrEq("VALUE"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ODD"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("EVEN"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("EVEN"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onEventValue(2);
  Mock::VerifyAndClearExpectations(&transitionMock);
}
