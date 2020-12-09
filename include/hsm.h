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

#include <type_traits>

namespace hsp {

//! Base class for hierarchical state machines
class HsmBase {
public:
  explicit HsmBase(HsmStateBase &topHsmState);

  //! Start the state machine
  // Note: Call this before any calls onEvent().
  // Note: Call this only ones
  void onStart();

  // FIXME make a onStop()

protected:
  //! Make the state machine take a transition to another state. This will result in a chain of onExit(), onEnter()
  // and onInit() on the involved states in the hierarchy.
  // @param nextState
  void transition(HsmStateBase &nextState);

  //! Make the state machine take a transition to a sub state but exiting and entering own state before entering
  // sub states.
  // @param nextState
  void externalTransition(HsmStateBase &nextState);

  //! Sets the initial sub state.
  // Note: Must be called from state.onInit(), if the concrete state has sub states.
  void initialTransition(HsmStateBase &subState);

  //! Sets the initial sub state first time called. Next time it will used the history.
  // Note: Must be called from state.onInit(), if the concrete state has sub states.
  void initialHistoryTransition(HsmStateBase &subState);

protected:
  // Very top state in the hierarchy. This is also the state that the machine first enters.
  HsmStateBase &topState;
  //! Current state. Must be at the lowest level of the hierachy (it cannot have sub states).
  HsmStateBase *currentState = nullptr;
  //! Temporarily set if and transitions is taken. Non nullptr if transition taken
  HsmStateBase *nextState = nullptr;
  //! Temporarily set when and transition is taken. Set equal to the state from which the transition is started.
  HsmStateBase *sourceState = nullptr;

  void enterAndInitNextState();
  void enterNextState();
  void initCurrentState();
  void exitUpToLCA(HsmStateBase &target);
  unsigned levelsToLCA(HsmStateBase &target);
}; // namespace hsp

template <typename CONTEXT> class Hsm : public HsmBase {
public:
  // The CONTEXT parameter must be a HsmState derived class
  static_assert(std::is_base_of<HsmState<CONTEXT>, CONTEXT>::value);
  using HsmBase::HsmBase;

  //! Call to stimulate state machine with an event. This function will traverse the hierarchy to
  // find a state that handles the event.
  // @param eventerror
  template <typename EVENT> bool onEvent(EVENT &&event) {
    HsmStateBase *state;
    bool handled = false;

    // Walk from current state up via state hierarchy
    for (state = currentState; state; state = state->superState) {
      // Remember which state that handle the event
      sourceState = state;

      // Try if state want's to handle event
      if (not dynamic_cast<CONTEXT *>(state)->onEvent(event)) {
        continue;
      }
      handled = true;

      // Is an state transition taken, then enter next state
      if (nextState) {
        enterAndInitNextState();
      }
      break;
    }
    return handled;
  }

private:
  // Only to be used internally in the Hsm
  using HsmBase::enterAndInitNextState;
  using HsmBase::enterNextState;
  using HsmBase::exitUpToLCA;
  using HsmBase::initCurrentState;
  using HsmBase::levelsToLCA;
};

} // namespace hsp
