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

namespace hsp {

/*!
 * Class to encapsulate a state in a Hsm (Hierarchical State Machine)
 */
class HsmStateBase {
  friend class HsmBase;

public:
  /*!
   * Call constructor with address of super state, top state must be given a nullptr
   */
  explicit HsmStateBase(HsmStateBase *const superState);
  virtual ~HsmStateBase();

  /*!
   * Invoked when an state enter during a transition. Override if state
   * should take an action upon enter.
   */
  virtual void onEnter();
  /*!
   * Invoked when an state exit during a transition. Override if state
   * should take an action upon exit.
   */
  virtual void onExit();
  /*!
   * Invoked when an state transition ends on a state. Override if state
   * should take an action upon initial transition. If state has sub states
   * this function must invoke InitialTransition() to define which sub state
   * should be entered.
   */
  virtual void onInit();

  // FIXME make private
  /*!
   * Pointer to super state.
   */
  HsmStateBase *const superState;
  /*!
   * Pointer to last active sub state
   */
  HsmStateBase *historySubstate = nullptr;
};

template <typename CONTEXT> class HsmState : public HsmStateBase {
public:
  using HsmStateBase::HsmStateBase;

private:
  template <typename T> friend class Hsm;

  template <typename EVENT> bool onEvent(EVENT &&event) { return (event)(static_cast<CONTEXT &>(*this)); }
}; // namespace hsp

} // namespace hsp
