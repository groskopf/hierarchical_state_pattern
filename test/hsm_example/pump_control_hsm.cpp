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
//
// @enduml
//

namespace PumpControl {

PumpControlHsm::PumpControlHsm(IPump &pump, ITimer &runningTimer, ITimer &pausedTimer)
    : Hsm(top)
    , pump(pump)
    , runningTimer(runningTimer)
    , pausedTimer(pausedTimer)
    , top(*this, nullptr)
    , standby(*this, &top)
    , continuous(*this, &top)
    , pulsing(*this, &top)
    , running(*this, &pulsing)
    , paused(*this, &pulsing) {}

// Events
bool PumpControlHsm::onStandby() {
  return onEvent([](PumpControlHsmState &state) { return state.onStandby(); });
}
bool PumpControlHsm::onContinuous() {
  return onEvent([](PumpControlHsmState &state) { return state.onContinuous(); });
}
bool PumpControlHsm::onPulsing() {
  return onEvent([](PumpControlHsmState &state) { return state.onPulsing(); });
}
bool PumpControlHsm::onRunningTimeout() {
  return onEvent([](PumpControlHsmState &state) { return state.onRunningTimeout(); });
}
bool PumpControlHsm::onPausedTimeout() {
  return onEvent([](PumpControlHsmState &state) { return state.onPausedTimeout(); });
}

// Actions
void PumpControlHsm::pumpOn() { pump.on(); }
void PumpControlHsm::pumpOff() { pump.off(); }
void PumpControlHsm::startRunningTimer() {
  runningTimer.start([&]() { onRunningTimeout(); });
}
void PumpControlHsm::cancelRunningTimer() { runningTimer.cancel(); }
void PumpControlHsm::startPausedTimer() {
  pausedTimer.start([&]() { onPausedTimeout(); });
}
void PumpControlHsm::cancelPausedTimer() { pausedTimer.cancel(); }

} // namespace PumpControl
