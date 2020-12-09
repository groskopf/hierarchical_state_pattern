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
#pragma once

#include <hsm_state.h>

using hsp::HsmState;

//!
// This state machine controls a pump running in three main states Standby, Running, Pulsing
//
// @startuml
// error
// state Top {
//   [*] --> Standby
//   state Standby {
//   }
//   state Pulsing {
//   [*] --> Running
//     state Running {
//	     Running : onEnter / pumpOn(), startRunningTimer()
//	     Running : onExit / pumpOff()
//       Running --> Paused : onRunningTimeout()
//     }
//     state Paused {
//	     Paused : onEnter / startPauseTimer()
//       Paused --> Running : onRunningTimeout()
//     }
//   }
//   state Continuous {
//	   Continuous : onEnter() / pumpOn()
//	   Continuous : onExit() / pumpOff()
//   }
//   Top --> Standby : onStandby()
//   Top --> Pulsing : onPulsing()
//   Top --> Continuous : onContinuous()
// }
//
// @enduml
//

namespace PumpControl {

class PumpControlHsm;

class PumpControlHsmState : public HsmState<PumpControlHsmState> {
public:
  PumpControlHsmState(PumpControlHsm &hsm, HsmState<PumpControlHsmState> *superState)
      : HsmState(superState)
      , hsm(hsm) {}

  virtual bool onStandby();
  virtual bool onContinuous();
  virtual bool onPulsing();
  virtual bool onRunningTimeout();
  virtual bool onPausedTimeout();

protected:
  PumpControlHsm &hsm;
};

class StateTop : public PumpControlHsmState {
public:
  using PumpControlHsmState::PumpControlHsmState;
  void onInit() override;
  bool onStandby() override;
  bool onContinuous() override;
  bool onPulsing() override;
};

class StateStandby : public PumpControlHsmState {
public:
  using PumpControlHsmState::PumpControlHsmState;
};

class StateContinuous : public PumpControlHsmState {
public:
  using PumpControlHsmState::PumpControlHsmState;
  void onEnter() override;
  void onExit() override;
};

class StatePulsing : public PumpControlHsmState {
public:
  using PumpControlHsmState::PumpControlHsmState;
  void onInit() override;
  void onExit() override;
};

class StateRunning : public PumpControlHsmState {
public:
  using PumpControlHsmState::PumpControlHsmState;
  void onEnter() override;
  void onExit() override;
  bool onRunningTimeout() override;
};

class StatePaused : public PumpControlHsmState {
public:
  using PumpControlHsmState::PumpControlHsmState;
  void onEnter() override;
  bool onPausedTimeout() override;
};

} // namespace PumpControl
