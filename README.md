#Hierachical State Pattern

State machines based on the GOF "State Pattern" https://en.wikipedia.org/wiki/State_pattern

This very small framework is used to support programming state machines using the "State Pattern".

Currently it supports: Event, States, Hierarchical State Machine, Choice point, transitions guards and history transitions.

##StateMachine

A state machine that is reacting upon event and performing actions. But depending on the current state the action taken might be different. This is implemented by delegating the event to the states.

The state machine defines the structure of the machines. It hold the states and attributes shared between them. It is also defining the hierarchy if such present.

The state machine is always acting out of its current state.

###Fsm

The Fsm class represent a state machine where no hierarchy is present. This means that no states contains substates.

A Fsm has always exactly one of its states as the currently active. 

###Hsm

The Hsm class represent a Hierarchical State Machine. All hierarchical state machines must derive from the base template class Hsm<>. Hsm<> is a template that takes one type parameter, which is a base class for all the states in the state machine.

`class AStateMachine : public Hsm<AState> {`
`...`
`..`
`}`

The constructor of Hsm takes state as parameter. This the top state of the state machine.

The base class Hsm provides two public operation onStart() and onEvent(). The operation onStart must be called before any call to onEvent(). It will initialize the state machine and make sure the top state is entered by calling onEnter() and onInit(). The invocation of onInit() could result in a recursive chain of onEnter() and onInit() until the state machine has settled into the last substate that is part of the first initialization.

The operation onEvent() is used to trigger event in the state machine. The (lowest) current state can choose to take reaction on the event or not. If no reaction is taken then the 


States



Event

Transitions

Internal transisions

Local transitions

Choicepoints

Guard conditions

HistoryStates

Orthogonal regions
