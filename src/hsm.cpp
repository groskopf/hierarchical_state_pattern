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

#include <cassert>

namespace hsp {

HsmBase::HsmBase(HsmStateBase &topHsmState)
    : topState(topHsmState) {}

///! Call to initialize the state machine. It will make sure that current state is set
// by calling onInit() on the top state and then traverse down initial transitions.
void HsmBase::onStart() {
  assert(topState.superState == nullptr && "Top state must have nullptr for super state");
  assert(currentState == nullptr && "onStart must only be called only ones");

  currentState = &topState;

  nextState = nullptr;

  currentState->onEnter();

  initCurrentState();
}

///!
// Used to change current state to a new state.
// @param target_state
void HsmBase::transition(HsmStateBase &targetState) {
  assert(currentState != nullptr && "onStart must be called before any transitions can be taken");
  // FIXME check that we are not calling this function twice
  // FIXME check that we are not calling this function inside a onInit()

  exitUpToLCA(targetState);

  nextState = &targetState;
}

//!
// Used to change current state to a new state.
// @param target_state
void HsmBase::externalTransition(HsmStateBase &targetState) {
  assert(currentState != nullptr && "onStart must be called before any transitions can be taken");
  // FIXME check that we are not calling this function twice
  // FIXME check that we are not calling this function inside a onInit()

  exitUpToLCA(targetState);

  // Exit and enter own state
  currentState->onExit();
  currentState->onEnter();

  nextState = &targetState;
}

/// Note: Must be called from state.onInit(), if the concrete state has sub states.
void HsmBase::initialTransition(HsmStateBase &subState) {
  // FIXME check that we are only calling this within a initialTransition
  // FIXME check that we are only calling this within a direct sub state
  nextState = &subState;
}

/// Note: Must be called from state.onInit(), if the concrete state has sub states.
void HsmBase::initialHistoryTransition(HsmStateBase &subState) {
  if (currentState->historySubstate) {
    initialTransition(*currentState->historySubstate);
  } else {
    initialTransition(subState);
  }
}

//!
// Makes the Hsm entering a state and invoke the initial transition
//
void HsmBase::enterAndInitNextState() {
  enterNextState();

  currentState = nextState;
  nextState = nullptr;

  initCurrentState();
}

//!
// Makes the Hsm enters the next state
//
void HsmBase::enterNextState() {
  // FIXME make this a vector
  constexpr int MAX_STATE_NESTING = 7;
  HsmStateBase *entry_path[MAX_STATE_NESTING];
  HsmStateBase **trace;
  HsmStateBase *state = nullptr;

  entry_path[0] = nullptr;
  trace = &entry_path[0];

  // Trace path to target state
  for (state = nextState; state != currentState; state = state->superState) {
    /// TODO Consider range check to avoid exceeding MAX_STATE_NESTING?
    *(++trace) = state;
  }

  // Invoke onEnter from LCA to next state
  while ((state = *trace--) != nullptr) {
    state->onEnter();
  }
}

//!
// Make the Hsm intialize current state
//
void HsmBase::initCurrentState() {
  while (true) {
    currentState->onInit();

    // If we have reached last substate
    if (nullptr == nextState)
      break;

    assert(nextState->superState == currentState && "Sub state do have super state set correctly");

    enterNextState();

    currentState = nextState;
    nextState = nullptr;
  }
}

//!
// Exit states up the state that is common least super state to current state and target state.
// @param target State that is the target of the transition.
//
void HsmBase::exitUpToLCA(HsmStateBase &target) {
  HsmStateBase *state = currentState;

  // Exit up to source state
  while (state != sourceState) {
    state->onExit();
    state->superState->historySubstate = state; // remember last substate
    state = state->superState;
  }

  // Exit up to LCA
  for (unsigned toLca = levelsToLCA(target); toLca != 0; toLca--) {
    state->onExit();
    state->superState->historySubstate = state; // remember last substate
    state = state->superState;
  }

  // Current state is now LCA
  currentState = state;
}

//!
// Calculate the levels up to the least common super state of current state and transition
// target state.
// @param target Target state of the transition.
// @return state levels
//
unsigned HsmBase::levelsToLCA(HsmStateBase &target) {
  HsmStateBase *state, *stateTarget;
  unsigned to_lca = 0;

  if (sourceState == &target) {
    return 1;
  }

  /// TODO Add check for toLca < MAX_STATE_NESTING? Otherwise include checks of state-tree depth
  /// in the State constructor(s)
  for (state = sourceState; state; ++to_lca, state = state->superState) {
    for (stateTarget = &target; stateTarget; stateTarget = stateTarget->superState) {
      if (state == stateTarget) {
        return to_lca;
      }
    }
  }

  return 0;
}

} // namespace hsp
