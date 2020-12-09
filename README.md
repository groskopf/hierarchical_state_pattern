#Hierachical State Pattern

State machines based on the GOF "State Pattern" https://en.wikipedia.org/wiki/State_pattern

This very small framework is used to support programming state machines using the "State Pattern". Currently it supports: Event, States, Hierarchical State Machine, Choice point, transitions guards and history transitions.

##StateMachine

A state machine is reacting upon event and performing actions. But depending on the current state the action taken might be different. This is implemented by delegating the event to the states. The state machine defines the structure of the machines. It hold the states and attributes shared between them. It is also defining the hierarchy if such present. The state machine is always acting out of its current state.

###Hsm

The Hsm class represent a Hierarchical State Machine. All hierarchical state machines must derive from the base template class Hsm<>. Hsm<> is a template that takes one type parameter, which is a base class for all the states in the state machine. This is using the CRTP pattern.

`class AStateMachine : public Hsm<AState> {`  
`  ...`  
`  ...`  
`}`  

The constructor of Hsm takes a state as parameter. This the top state of the state machine.

The base class Hsm provides two public operation `onStart()` and `onEvent()` used to trigger the events. 

The operation `onStart()` must be called before any call to `onEvent()`. It will initialize the state machine and make sure the top state is entered by calling `onEnter()` and `onInit()`. The invocation of `onInit()` could result in a recursive chain of `onEnter()` and `onInit()` until the state machine has settled into the last substate that is part of the first initialization.

The operation `onEvent()` is used to trigger event in the state machine. The (lowest) current state can choose to take reaction on the event or not. If no reaction is taken then the event is passed to the super state recurrently until the event is handled or no more super states are found.
 
###State

Each state is implemented by a class derived from HsmState<>. This template take the state it self as parameter and is using the CRTP pattern here by. The state class must implement a function for each event. If the event carries parameters the parameters must be passed to the event handler operations. Each event handler is virtual and must be overriden by the concrete state classes to define the behavior of the state machine.

`class AState : public Hsm<AState> {`  
`  virtual bool onEventA() { return false; }`  
`  virtual bool onEventB(int i) { return false; }`  
`  virtual bool onEventC() { return false; }`  
`  ...`  
`}`  

This Hsm base state defines the event that any state in the system could handle. Then there must be defined a derived class for each concrete state.  

`class ConcreteState1 : public AState {`  
`  bool onEventA() override { actionA(); return true; }`  
`  bool onEventB(int i) override { actionB(i); return true; }`  
`  bool onEventC() override { actionC(); return true; }`  
`  ...`  
`}`  

All event handler functions must return true or false depending on if they react on the event or not.  
The HsmState<> base class defines a set of common event handlers that the Hsm class will make sure is properly invoked. It should not be invoked directly in any means.

`class ConcreteState2 : public AState {`  
`  void onEnter() override { actionEnter(); }`  
`  void onExit() override { actionExit(); }`  
`  void onInit() override { }`  
`  ...`  
`}`  

###Event

Events is supposed to invoke an operation on a state according to the state pattern. This is implemented object with a function operator. This function operator takes a state as parameter and invokes one of the state handlers in the states. The Hsm will use this functional object to invoke each state in the hierarchy to check if the event is handled. The event could be lambda expressions:

`hsm.onEvent([](AState &state) { return state.onEventA(); return true; });`  

If the event must carry parameter this can utilized via the captures in the lambda.


`hsm.onEvent([=](AState &state) { return state.onEventB(i); return true; });`  


###Transitions

If a state react on the event it can then choose to take a transition to another state. This is done by invoking the `Hsm::transition(HsmState& nextState)` operation. This will make sure current state is exited and next state is entered.

`void onEventC() override { hsm.transition(hsm.concreteState2); return true; }`  

When the transition is taken the hsm will make sure that `onEnter()`, `onExit()` and `onInit()` is invoked correctly according to the state hierarchy.

###Initial transitions

After a state has been entered (`onEnter()` invoked) the state must take a transition to one of its sub state if such exits accordingly to normal behavior of hierarchical state machines. This done by invoking the `Hsm::initialTransition()` inside of a `onInit()` event handler.

`void onInit() override { hsm.initialTransition(hsm.subState); }`  

##HistoryStates

Sometimes the state must not enter the same sub state each time it is initialized, but rather the last active sub state when it was last exited. This is implement by calling `hsm.initialHistoryTransition()` inside the `onInit()` event handler.

###External transitions

Sometimes expected behavior is that the transition exit the least common super state of the source and targe states in a state machine. This special case is handled by calling the special operation `Hsm::externalTransition()` 

###Internal event

Internal event is another special case where a state is handling an event but no transitions is taken. This implemented by just perform the required action omit calling `Hsm::transition()` and return true.

### Choice points

Choice pointer are implemented by simply do the required logical statements inside the even handlers.

`void onEventC(int i) override {`  
`  if( i%2 ) {  // choice point`  
`    hsm.transition(hsm.evenState);`  
`  } else {`  
`    hsm.transition(hsm.oddState);`  
`  }`  
`  return true;`  
`}`  

###Guard conditions

Event guards are implemented by simply do the required logical statements inside the even handlers.

`void onEventC(int i) override {`  
`  if( i > 10 ) {  // Guard`  
`    return false`  
`  }`  
`  hsm.transition(hsm.concreteState3);`  
`  return true;`  
`}`  

###Orthogonal regions

Not supported yes
