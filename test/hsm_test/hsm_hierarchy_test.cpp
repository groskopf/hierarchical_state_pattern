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

#include <hsm.h>

#include <gmock/gmock.h>

#include <string>

using std::cout;
using std::endl;
using std::string;

using hsp::Hsm;
using hsp::HsmState;

using ::testing::_;
using ::testing::InSequence;
using ::testing::Invoke;
using ::testing::StrEq;
using ::testing::Test;

//!
// This test tries to challenge the state walker to see of onEnter(), onExit() and onInit() is
// executed in the right order when taking transitions.
//
//  @startuml
//
// state Top {
//   [*] --> S1
//   state S1 {
//     S1 --> S1 : A
//     S1 --> S2 : B
//     S1 -down-> S2 : C
//     S1 --> Top : D
//     S1 --> S211 : F
//     state S11 {
//       S11 -> S211 : G
//       S11 : H
//     }
//   }
//   state S2 {
//     [*] --> S21
//     S2 --> S1 : C
//     S2 --> S11 : F
//     state S21 {
//       [*] --> S211
//       S21 --> S211 : B
//       S21 --> S21 : H
//       state S211 {
//         S211 --> S21 : D
//         S211 --> Top : G
//       }
//     }
//   }
// }
// @enduml
//
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

  virtual bool onEventA() { return false; }
  virtual bool onEventB() { return false; }
  virtual bool onEventC() { return false; }
  virtual bool onEventD() { return false; }
  virtual bool onEventE() { return false; }
  virtual bool onEventF() { return false; }
  virtual bool onEventG() { return false; }
  virtual bool onEventH() { return false; }

protected:
  HsmUnderTest &hsm;
  const string name;
};

class StateTop : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  void onInit() override;

  // Make sure that we and ALL event in the top to verify that we have none uncaught
  bool onEventA() override;
  bool onEventB() override;
  bool onEventC() override;
  bool onEventD() override;
  bool onEventE() override;
  bool onEventF() override;
  bool onEventG() override;
  bool onEventH() override;
};

class State1 : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  void onInit() override;
  bool onEventA() override;
  bool onEventB() override;
  bool onEventC() override;
  bool onEventD() override;
  bool onEventF() override;
};

class State11 : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  bool onEventD() override;
  bool onEventG() override;
};

class State2 : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  void onInit() override;
  bool onEventC() override;
  bool onEventF() override;
};

class State21 : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  void onInit() override;
  bool onEventB() override;
  bool onEventH() override;
};

class State211 : public StateUnderTest {
public:
  using StateUnderTest::StateUnderTest;

  bool onEventD() override;
  bool onEventG() override;
};

class HsmUnderTest : public Hsm<StateUnderTest> {
public:
  explicit HsmUnderTest(TransitionMock &TransitionMock)
      : Hsm(top)
      , top(*this, nullptr, "TOP")
      , s1(*this, &top, "S1")
      , s11(*this, &s1, "S11")
      , s2(*this, &top, "S2")
      , s21(*this, &s2, "S21")
      , s211(*this, &s21, "S211")
      , transition_mock(TransitionMock) {}

  bool onEventA() {
    return onEvent([](StateUnderTest &state) { return state.onEventA(); });
  }

  bool onEventB() {
    return onEvent([](StateUnderTest &state) { return state.onEventB(); });
  }

  bool onEventC() {
    return onEvent([](StateUnderTest &state) { return state.onEventC(); });
  }

  bool onEventD() {
    return onEvent([](StateUnderTest &state) { return state.onEventD(); });
  }

  bool onEventE() {
    return onEvent([](StateUnderTest &state) { return state.onEventE(); });
  }

  bool onEventF() {
    return onEvent([](StateUnderTest &state) { return state.onEventF(); });
  }

  bool onEventG() {
    return onEvent([](StateUnderTest &state) { return state.onEventG(); });
  }

  bool onEventH() {
    return onEvent([](StateUnderTest &state) { return state.onEventH(); });
  }

  TransitionMock &transition_mock;

private:
  StateTop top;
  State1 s1;
  State11 s11;
  State2 s2;
  State21 s21;
  State211 s211;

  friend StateTop;
  friend State1;
  friend State11;
  friend State2;
  friend State21;
  friend State211;
}; // namespace

StateUnderTest::StateUnderTest(HsmUnderTest &hsm, HsmState *const super_state, const string &name)
    : HsmState(super_state)
    , hsm(hsm)
    , name(name) {}

StateUnderTest::~StateUnderTest() {}

void StateUnderTest::InvokeMock(const string &event) const { hsm.transition_mock.activate(name, event); }

void StateTop::onInit() {
  InvokeMock("INIT");
  hsm.initialTransition(hsm.s1);
}

bool StateTop::onEventA() {
  InvokeMock("A");
  return true;
}

bool StateTop::onEventB() {
  InvokeMock("B");
  return true;
}

bool StateTop::onEventC() {
  InvokeMock("C");
  return true;
}

bool StateTop::onEventD() {
  InvokeMock("D");
  return true;
}

bool StateTop::onEventE() {
  InvokeMock("E");
  hsm.transition(hsm.s211);
  return true;
}

bool StateTop::onEventF() {
  InvokeMock("F");
  return true;
}

bool StateTop::onEventG() {
  InvokeMock("G");
  return true;
}

bool StateTop::onEventH() {
  InvokeMock("H");
  return true;
}

void State1::onInit() {
  InvokeMock("INIT");
  hsm.initialTransition(hsm.s11);
}

bool State1::onEventA() {
  InvokeMock("A");
  hsm.transition(hsm.s1);
  return true;
}

bool State1::onEventB() {
  InvokeMock("B");
  hsm.transition(hsm.s11);
  return true;
}

bool State1::onEventC() {
  InvokeMock("C");
  hsm.transition(hsm.s2);
  return true;
}

bool State1::onEventD() {
  InvokeMock("D");
  hsm.transition(hsm.top);
  return true;
}

bool State1::onEventF() {
  InvokeMock("F");
  hsm.transition(hsm.s211);
  return true;
}

bool State11::onEventG() {
  InvokeMock("G");
  hsm.transition(hsm.s211);
  return true;
}

bool State11::onEventD() {
  InvokeMock("D");
  hsm.transition(hsm.s11);
  return true;
}

void State2::onInit() {
  InvokeMock("INIT");
  hsm.initialTransition(hsm.s21);
}

bool State2::onEventC() {
  InvokeMock("C");
  hsm.transition(hsm.s1);
  return true;
}

bool State2::onEventF() {
  InvokeMock("F");
  hsm.transition(hsm.s11);
  return true;
}

void State21::onInit() {
  InvokeMock("INIT");
  hsm.initialTransition(hsm.s211);
}

bool State21::onEventB() {
  InvokeMock("B");
  hsm.transition(hsm.s211);
  return true;
}

bool State21::onEventH() {
  InvokeMock("H");
  hsm.transition(hsm.s21);
  return true;
}

bool State211::onEventD() {
  InvokeMock("D");
  hsm.transition(hsm.s21);
  return true;
}

bool State211::onEventG() {
  InvokeMock("G");
  hsm.transition(hsm.top);
  return true;
}

class HsmHierachyTest : public Test {
public:
  TransitionMock transition_mock;
  HsmUnderTest hsm_under_test; // DUT

  HsmHierachyTest()
      : hsm_under_test(transition_mock) {}
};

} // namespace

TEST_F(HsmHierachyTest, allEvents) {
  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("TOP"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("TOP"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("INIT")));
  }
  hsm_under_test.onStart();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("A")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("INIT")));
  }
  hsm_under_test.onEventA();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("B")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("INIT")));
  }
  hsm_under_test.onEventB();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("C")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("INIT")));
  }
  hsm_under_test.onEventC();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("D")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("INIT")));
  }
  hsm_under_test.onEventD();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("TOP"), StrEq("E")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("INIT")));
  }
  hsm_under_test.onEventE();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("F")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("INIT")));
  }
  hsm_under_test.onEventF();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("G")));
    EXPECT_CALL(transition_mock, activate(StrEq("S11"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S1"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S2"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("INIT")));
  }
  hsm_under_test.onEventG();

  {
    InSequence sec;
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("H")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("EXIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S21"), StrEq("INIT")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("ENTRY")));
    EXPECT_CALL(transition_mock, activate(StrEq("S211"), StrEq("INIT")));
  }
  hsm_under_test.onEventH();
}
