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

#include "pump_control_hsm_states.h"
#include "pump_control_hsm.h"

//!
// This state machine controls a pump running in three main states Standby, Running, Pulsing
//
// @startuml
//
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
// Standby
// @enduml
//

namespace PumpControl {

// Default behavior is to NOT handle the event
bool PumpControlHsmState::onStandby() { return false; }
bool PumpControlHsmState::onContinuous() { return false; }
bool PumpControlHsmState::onPulsing() { return false; }
bool PumpControlHsmState::onRunningTimeout() { return false; }
bool PumpControlHsmState::onPausedTimeout() { return false; }

void StateTop::onInit() { hsm.initialTransition(hsm.standby); }
bool StateTop::onStandby() {
  hsm.transition(hsm.standby);
  return true;
}
bool StateTop::onContinuous() {
  hsm.transition(hsm.continuous);
  return true;
}
bool StateTop::onPulsing() {
  hsm.transition(hsm.pulsing);
  return true;
}

void StateContinuous::onEnter() { hsm.pumpOn(); }
void StateContinuous::onExit() { hsm.pumpOff(); }

void StatePulsing::onInit() { hsm.initialTransition(hsm.running); }
void StatePulsing::onExit() {
  hsm.cancelRunningTimer();
  hsm.cancelPausedTimer();
}

void StateRunning::onEnter() {
  hsm.pumpOn();
  hsm.startRunningTimer();
}
void StateRunning::onExit() { hsm.pumpOff(); }
bool StateRunning::onRunningTimeout() {
  hsm.transition(hsm.paused);
  return true;
}

void StatePaused::onEnter() { hsm.startPausedTimer(); }
bool StatePaused::onPausedTimeout() {
  hsm.transition(hsm.running);
  return true;
}

} // namespace PumpControl
