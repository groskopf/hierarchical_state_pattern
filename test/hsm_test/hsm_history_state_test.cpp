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
// 	 state Disabled
// 	 state Enabled {
// 	   [H] --> A
// 	   state A
// 	   state B
// 	   Enabled --> A : eventA
// 	   Enabled --> B : eventB
// 	 }
//   [*] --> Disabled
//   Disabled --> Enabled : On
//   Enabled --> Disabled : Off
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

  virtual bool onEventOn() { return false; }
  virtual bool onEventOff() { return false; }
  virtual bool onEventA() { return false; }
  virtual bool onEventB() { return false; }

protected:
  HsmUnderTest &hsm;
  const string name;
};

class StateTop : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  void onInit() override;
};

class StateDisabled : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  bool onEventOn() override;
};

class StateEnabled : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;
  void onInit() override;
  bool onEventOff() override;
  bool onEventA() override;
  bool onEventB() override;
};

class StateEnabledSub : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;
};

class HsmUnderTest : public Hsm<StateUnderTest> {
public:
  explicit HsmUnderTest(TransitionMock &TransitionMock)
      : Hsm(top)
      , top(*this, nullptr, "TOP")
      , disabled(*this, &top, "DISABLED")
      , enabled(*this, &top, "ENABLED")
      , a(*this, &enabled, "A")
      , b(*this, &enabled, "B")
      , transitionMock(TransitionMock) {}

  bool onEventOn() {
    return onEvent([](StateUnderTest &state) { return state.onEventOn(); });
  }

  bool onEventOff() {
    return onEvent([](StateUnderTest &state) { return state.onEventOff(); });
  }

  bool onEventA() {
    return onEvent([](StateUnderTest &state) { return state.onEventA(); });
  }

  bool onEventB() {
    return onEvent([](StateUnderTest &state) { return state.onEventB(); });
  }

  TransitionMock &transitionMock;

private:
  StateTop top;
  StateDisabled disabled;
  StateEnabled enabled;
  StateEnabledSub a;
  StateEnabledSub b;

  friend StateTop;
  friend StateDisabled;
  friend StateEnabled;
};

StateUnderTest::StateUnderTest(HsmUnderTest &hsm, HsmState *const super_state, const string &name)
    : HsmState(super_state)
    , hsm(hsm)
    , name(name) {}

StateUnderTest::~StateUnderTest() {}

void StateUnderTest::InvokeMock(const string &event) const { hsm.transitionMock.activate(name, event); }

void StateTop::onInit() {
  InvokeMock("INIT");
  hsm.initialTransition(hsm.disabled);
}

bool StateDisabled::onEventOn() {
  InvokeMock("ON");
  hsm.transition(hsm.enabled);
  return true;
}

void StateEnabled::onInit() {
  InvokeMock("INIT");
  hsm.initialHistoryTransition(hsm.a);
}

bool StateEnabled::onEventOff() {
  InvokeMock("OFF");
  hsm.transition(hsm.disabled);
  return true;
}

bool StateEnabled::onEventA() {
  InvokeMock("A");
  hsm.transition(hsm.a);
  return true;
}

bool StateEnabled::onEventB() {
  InvokeMock("B");
  hsm.transition(hsm.b);
  return true;
}

class HsmHistoryStateTest : public Test {
public:
  TransitionMock transitionMock;
  HsmUnderTest hsm_under_test; // DUT

  HsmHistoryStateTest()
      : hsm_under_test(transitionMock) {}
};

} // namespace

TEST_F(HsmHistoryStateTest, test) {
  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("TOP"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("TOP"), StrEq("INIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onStart();
  Mock::VerifyAndClearExpectations(&transitionMock);

  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("ON"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("INIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("A"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("A"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onEventOn();

  Mock::VerifyAndClearExpectations(&transitionMock);

  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("B"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("A"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("B"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("B"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onEventB();
  Mock::VerifyAndClearExpectations(&transitionMock);

  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("OFF"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("B"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onEventOff();
  Mock::VerifyAndClearExpectations(&transitionMock);

  // Now enter the history state
  {
    InSequence sec;
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("ON"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("DISABLED"), StrEq("EXIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("ENABLED"), StrEq("INIT"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("B"), StrEq("ENTRY"))).RetiresOnSaturation();
    EXPECT_CALL(transitionMock, activate(StrEq("B"), StrEq("INIT"))).RetiresOnSaturation();
  }
  hsm_under_test.onEventOn();
  Mock::VerifyAndClearExpectations(&transitionMock);
}
