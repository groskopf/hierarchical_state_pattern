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

#include "pump_control_hsm_states.h"

#include "hsm.h"

#include <functional>

using hsp::Hsm;

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
//    [*] --> Running
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

class IPump {
public:
  virtual void on() = 0;
  virtual void off() = 0;
};

class ITimer {
public:
  virtual void start(std::function<void()> timeoutCallback) = 0;
  virtual void cancel() = 0;
};

class PumpControlHsm : public Hsm<PumpControl::PumpControlHsmState> {
public:
  PumpControlHsm(IPump &pump, ITimer &runningTimer, ITimer &pausedTimer);

  // Events triggers
  virtual bool onStandby();
  virtual bool onContinuous();
  virtual bool onPulsing();

private:
  IPump &pump;
  ITimer &runningTimer;
  ITimer &pausedTimer;

  // Internal event triggers
  virtual bool onRunningTimeout();
  virtual bool onPausedTimeout();

  // Actions
  void pumpOn();
  void pumpOff();
  void startRunningTimer();
  void cancelRunningTimer();
  void startPausedTimer();
  void cancelPausedTimer();

  // States
  StateTop top;
  StateStandby standby;
  StateContinuous continuous;
  StatePulsing pulsing;
  StateRunning running;
  StatePaused paused;

  friend StateTop;
  friend StateStandby;
  friend StateContinuous;
  friend StatePulsing;
  friend StateRunning;
  friend StatePaused;
};

} // namespace PumpControl
