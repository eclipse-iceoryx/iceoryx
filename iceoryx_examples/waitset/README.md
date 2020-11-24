# WaitSet

## Architecture

 - **Listener** a class which can be triggered by a _Trigger_.
 - **Trigger** a class which can be used to trigger a listener. If a _Trigger_ goes
     out of scope it will detach itself from the listener. A _Trigger_ is
     logical equal to another _Trigger_ if they:
     - are attached to the same _WaitSet_ (or in other words they are using the 
       same `ConditionVariable`)
     - they have the same _TriggerOrigin_
     - they have the same callback to verify that they were triggered 
       (`hasTriggerCallback`)
     - they have the same _TriggerId_
 - **Triggerable** a class which has attached a _Trigger_ to itself to trigger
     certain events to a listener.
 - **TriggerId** a id which identifies the trigger. It does not follow any 
     restrictions and the user can choose any arbitrary `uint64_t`.
 - **TriggerOrigin** the pointer to the class where the trigger originated from, short
     pointer to the _Triggerable_.
 - **WaitSet** a listener which manages a set of _Triggers_ which can be acquired by 
     the user. The _Waitset_ listens 
     to the whole set of _Triggers_ and if one _Trigger_ is triggered it will notify
     the user. If a _WaitSet_ goes out of scope all attached _Triggers_ will be
     invalidated.

## Use cases
This example consists of 5 use cases.
 
 1. `ice_waitset_gateway.cpp`: We build a gateway together to forward data
    to another network. A list of subscribers is handled in an uniform way 
    by defining a callback and just call it for every subscriber who received data.

 2. `ice_waitset_grouping`: We would like to group multiple subscribers into 2 
    groups and handle them according to their group membership.

 3. `ice_waitset_individual`: A list of subscribers where every subscriber is 
    handled differently.

 4. `ice_waitset_sync`: We use the WaitSet to trigger a cyclic call which should 
    execute an algorithm every 100ms.

 5. `ice_waitset_trigger`: We create our own class which can be attached to a
    WaitSet and trigger events.

## Gateway