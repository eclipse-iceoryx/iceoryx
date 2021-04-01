# WaitSet

## Thread Safety
The WaitSet is **not** thread-safe!
- It is **not** allowed to attach or detach _Triggerable_
   classes with `attachEvent` or `detachEvent` when another thread is currently
   waiting for events with `wait` or `timedWait`.
- Do **not** call any of the WaitSet methods concurrently.

The _TriggerHandle_ on the other hand is thread-safe! Therefore you are allowed to
attach/detach a _TriggerHandle_ to a _Triggerable_ while another thread may
trigger the _TriggerHandle_.

## Introduction

The WaitSet is a set where you can attach objects so that they can signal a wide variety
of events to one single notifyable. The typical approach is that one creates a
WaitSet attaches multiple subscribers, user trigger or other _Triggerables_ to it and then wait till
one or many of the attached entities signal an event. If that happens one receives
a list of _EventInfos_ which is corresponding to all occurred events.

## Events and States

In this context we define the state of an object as a specified set of values 
to which the members of that object are set. An event on the other hand 
is defined as a state change. Usually an event changes the state of the corresponding 
object but this is not mandatory.

States and events can be attached to a WaitSet. The user will be informed only once 
by the WaitSet for every event which occurred. If the event occurred multiple times 
before the user has requested an event update from the WaitSet the user will still 
be informed only once. State changes are induced by events 
and the user will be informed about a specific state as long as the state persists.

The subscriber for instance has the state `SubscriberState::HAS_DATA` and the event 
`SubscriberEvent::DATA_RECEIVED`. If you attach the subscriber event 
`SubscriberEvent::DATA_RECEIVED` to a WaitSet you will be notified about every new 
incoming sample whenever you call `WaitSet::wait` or `WaitSet::timedWait`. If multiple
samples were sent before you called those methods you will still receive only one 
notification.

If you attach on the other hand the state `SubscriberState::HAS_DATA` you will 
be notified by `WaitSet::wait` or `WaitSet::timedWait` as long as there are received 
samples present in the subscriber.

## Expected Output

<!-- @todo Add expected output with asciinema recording before v1.0-->
<!-- @todo multiple examples described in here, expected output should be in front of every example -->

## Glossary

 - **EventCallback** a callback attached to an _EventInfo_. It must have the
     following signature `void ( EventOrigin )`. Any free function, static
     class method and non capturing lambda is allowed. You have to ensure the lifetime of that callback.
     This can become important when you would like to use lambdas.
 - **EventId** an id which is tagged to an event. It does not need to be unique
     or follow any restrictions. The user can choose any arbitrary `uint64_t`. Assigning
     the same _EventId_ to multiple _Events_ can be useful when you would like to
     group _Events_.
 - **EventInfo** a class which corresponds with _Triggers_ and is used to inform
     the user which _Event_ occurred. You can use the _EventInfo_ to acquire
     the _EventId_, call the _EventCallback_ or acquire the _EventOrigin_.
 - **EventOrigin** the pointer to the class where the _Event_ originated from, short
     pointer to the _Triggerable_.
 - **Event** a state change of an object.
 - **Events** a _Triggerable_ will signal an event via a _TriggerHandle_ to a _Notifyable_.
     For instance one can attach the subscriber event `DATA_RECEIVED` to _WaitSet_. This will cause the
     subscriber to notify the WaitSet via the _TriggerHandle_ everytime when a sample was received.
 - **Notifyable** is a class which listens to events. A _TriggerHandle_ which corresponds to a _Trigger_
     is used to notify the _Notifyable_ that an event occurred. The WaitSet is a _Notifyable_.
 - **State** a specified set of values to which the members of an object are set.
 - **Trigger** a class which is used by the _Notifyable_ to acquire the information which events were
     signalled. It corresponds to a _TriggerHandle_. If the _Notifyable_ goes out of scope the corresponding
     _TriggerHandle_ will be invalidated and if the _Triggerable_ goes out of scope the corresponding
     _Trigger_ will be invalidated.
 - **Triggerable** a class which has attached a _TriggerHandle_ to itself to signal
     certain _Events_ to a _Notifyable_.
 - **TriggerHandle** a thread-safe class which can be used to trigger a _Notifyable_.
     If a _TriggerHandle_ goes out of scope it will detach itself from the _Notifyable_. A _TriggerHandle_ is
     logical equal to another _Trigger_ if they:
     - are attached to the same _Notifyable_ (or in other words they are using the
       same `ConditionVariable`)
     - they have the same _EventOrigin_
     - they have the same callback to verify that they were triggered
       (`hasEventCallback`)
     - they have the same _EventId_
 - **WaitSet** a _Notifyable_ which manages a set of _Triggers_ which are corresponding to _Events_.
     A user may attach or detach events. The _Waitset_ listens
     to the whole set of _Triggers_ and if one or more _Triggers_ are triggered by an event it will notify
     the user. If a _WaitSet_ goes out of scope all attached _Triggers_ will be
     invalidated.

## Quick Overview
**Events** or **States** can be attached to a **Notifyable** like the **WaitSet**.
The **WaitSet** will listen on **Triggers** for a signal that an **Event** has occurred and it hands out
**TriggerHandles** to **Triggerable** objects. The **TriggerHandle** is used to inform the **WaitSet**
about the occurrence of an **Event**. When returning from `WaitSet::wait()` the user is provided with a vector of **EventInfos**
associated with **Events** which had occurred and **States** which persists. The **EventOrigin**, **EventId** and **EventCallback**
are stored inside of the **EventInfo** and can be acquired by the user.

!!! attention 
    Please be aware about the thread-safety restrictions of the _WaitSet_ and 
    read the [Thread Safety](#thread-safety) chapter carefully.

## Reference

| task | call |
|:-----|:-----|
|attach subscriber event to a WaitSet|`waitset.attachEvent(subscriber, iox::popo::SubscriberEvent::DATA_RECEIVED, 123, &mySubscriberCallback)`|
|attach subscriber state to a WaitSet|`waitset.attachState(subscriber, iox::popo::SubscriberState::HAS_DATA, 123, &mySubscriberCallback)`|
|attach user trigger to a WaitSet|`waitset.attachEvent(userTrigger, 456, &myUserTriggerCallback)`|
|wait for triggers           |`auto triggerVector = myWaitSet.wait();`  |
|wait for triggers with timeout |`auto triggerVector = myWaitSet.timedWait(1_s);`  |
|check if event originated from some object|`event->doesOriginateFrom(ptrToSomeObject)`|
|get id of the event|`event->getEventId()`|
|call eventCallback|`(*event)()`|
|acquire _EventOrigin_|`event->getOrigin<OriginType>();`|

## Use Cases
This example consists of 5 use cases.
 
 1. `ice_waitset_gateway.cpp`: We build a gateway to forward data
    to another network. A list of subscriber events are handled in an uniform way
    by defining a callback which is executed for every subscriber who
    has received data.

 2. `ice_waitset_grouping`: We would like to group multiple subscribers into 2 distinct
    groups and handle them whenever they have a specified state according to their group membership.

 3. `ice_waitset_individual`: A list of subscribers where every subscriber is
    handled differently.

 4. `ice_waitset_sync`: We use the WaitSet to trigger a cyclic call which should
    execute an algorithm every 100ms.

 5. `ice_waitset_trigger`: We create our own class which can be attached to a
    WaitSet to signal states and events.

## Examples

All our examples require a running `iox-roudi` and some data to receive which will be
send by `iox-ex-waitset-publisher`. The publisher does not contain any _WaitSet_ specific
logic and is explained in detail in the [icedelivery example](../icedelivery/).

<!-- @todo Add expected output with asciinema recording before v1.0-->

### Gateway

We have a list of subscribers which can be subscribed to any arbitrary topic
and everytime we received a sample we would like to send the bytestream to a socket,
write it into a file or print it to the console. But whatever we choose to do
we perform the same task for all the subscribers. And since we process all incoming 
data right away we attach the `SubscriberEvent::DATA_RECEIVED` which notifies us 
only once.

Let's start by implementing our callback which prints the subscriber pointer, the
payload size and the payload pointer to the console. We have to process all samples
as long as there are samples in the subscriber since we attached an event which notifies
us only once. But it is impossible to miss samples since the notification is reset 
right after `wait` or `timedWait` is returned - this means if a sample arrives after 
those calls we will be notified again.
```cpp
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber)
{
    while (subscriber->hasData())
    {
        subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
            std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                      << sample.getHeader()->payloadSize << " ptr: " << std::hex << sample.getHeader()->payload()
                      << std::endl;
        });
    }
}
```
An _Event_ always requires a callback which has the following signature
`void (EventOrigin)`. In our example the _EventOrigin_ is a
`iox::popo::UntypedSubscriber` pointer which we use to acquire the latest sample by calling
`take()`. When `take()` was successful we print our message to
the console inside of the `and_then` lambda.

In our `main` function we create a _WaitSet_ which has storage capacity for 5 events,
4 subscribers and one shutdown trigger, after we registered us at our central
broker RouDi. Then we attach our `shutdownTrigger` to handle `CTRL+c` events.
```cpp
iox::popo::WaitSet waitset<NUMBER_OF_SUBSCRIBERS + ONE_SHUTDOWN_TRIGGER>;

waitset.attachEvent(shutdownTrigger).or_else([](auto) {
    std::cerr << "failed to attach shutdown trigger" << std::endl;
    std::terminate();
});
```

After that we create a vector to hold our subscribers, we create and then
attach them to our _WaitSet_ with the `SubscriberEvent::DATA_RECEIVED` event and the `subscriberCallback`.
Everytime one 
of the subscribers is receiving a new sample it will trigger the _WaitSet_.
```cpp
iox::cxx::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
for (auto i = 0; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    auto& subscriber = subscriberVector.back();

    waitset.attachEvent(subscriber, iox::popo::SubscriberEvent::DATA_RECEIVED, 0, &subscriberCallback)
        .or_else([&](auto) {
            std::cerr << "failed to attach subscriber" << i << std::endl;
            std::terminate();
        });
}
```
`attachEvent` is returning a `cxx::expected` which informs us if attaching the event
succeeded. In the `.or_else([&](auto){/*...*/})` part we perform the error handling 
whenever `attachEvent` failed.

Now our system is prepared and ready to work. We enter the event loop which
starts with a call to our _WaitSet_ (`waitset.wait()`). This call will block until
one or more events triggered the _WaitSet_. After the call returned we get a
vector filled with _EventInfos_ which are corresponding to all the events which
triggered the _WaitSet_.

We iterate through this vector, if an _Event_ originated from the `shutdownTrigger`
we exit the program otherwise we just call the assigned callback by calling
the trigger. This will then call `subscriberCallback` with the _EventOrigin_
(the pointer to the untyped subscriber) as parameter.
```cpp
while (true)
{
    auto eventVector = waitset.wait();

    for (auto& event : eventVector)
    {
        if (event->doesOriginateFrom(&shutdownTrigger))
        {
            return (EXIT_SUCCESS);
        }
        else
        {
            (*event)();
        }
    }
```

### Grouping
In our next use case we would like to divide the subscribers into two groups
and we do not want to attach a callback to them. Instead we perform the calls on the
subscribers directly. Additionally, we would like to be notified as long as there 
are samples in the subscriber queue therefore we have to attach the `SubscriberState::HAS_DATA`.

We again start by creating a _WaitSet_ with a capacity of 5 (4 subscribers and 1 shutdownTrigger),
and attach the `shutdownTrigger` to handle `CTRL+c`.
```cpp
iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS + ONE_SHUTDOWN_TRIGGER> waitset;

waitset.attachEvent(shutdownTrigger).or_else([](auto) {
    std::cerr << "failed to attach shutdown trigger" << std::endl;
    std::terminate();
});
```

Now we create a vector of 4 subscribers.
```cpp
iox::cxx::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
for (auto i = 0; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    auto& subscriber = subscriberVector.back();
}
```

After that we define our two groups with the ids `FIRST_GROUP_ID` and `SECOND_GROUP_ID`
and attach the first two subscribers with the state `SubscriberState::HAS_DATA` to the first group and the remaining subscribers
to the second group.
```cpp
for (auto i = 0; i < NUMBER_OF_SUBSCRIBERS / 2; ++i)
{
    waitset.attachEvent(subscriberVector[i], iox::popo::SubscriberState::HAS_DATA, FIRST_GROUP_ID)
        .or_else([&](auto) {
            std::cerr << "failed to attach subscriber" << i << std::endl;
            std::terminate();
        });
}

for (auto i = NUMBER_OF_SUBSCRIBERS / 2; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    waitset.attachEvent(subscriberVector[i], iox::popo::SubscriberState::HAS_DATA, SECOND_GROUP_ID)
        .or_else([&](auto) {
            std::cerr << "failed to attach subscriber" << i << std::endl;
            std::terminate();
        });
}
```

The event loop calls `auto eventVector = waitset.wait()` in a blocking call to
receive a vector of all the _EventInfos_ which are corresponding to the occurred events.
If the _Event_ originated from the `shutdownTrigger` we terminate the program.
```cpp
while (true)
{
    auto eventVector = waitset.wait();

    for (auto& event : eventVector)
    {
        if (event->doesOriginateFrom(&shutdownTrigger))
        {
            return (EXIT_SUCCESS);
        }
```

The remaining part of the loop is handling the subscribers. In the first group
we would like to print the received data to the console and in the second group
we just dismiss the received data. 
```cpp
    else if (event->getEventId() == FIRST_GROUP_ID)
    {
        auto subscriber = event->getOrigin<iox::popo::UntypedSubscriber>();
        subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
            const CounterTopic* data = reinterpret_cast<const CounterTopic*>(sample.get());
            std::cout << "received: " << std::dec << data->counter << std::endl;
        });
    }
    else if (event->getEventId() == SECOND_GROUP_ID)
    {
        std::cout << "dismiss data\n";
        auto subscriber = event->getOrigin<iox::popo::UntypedSubscriber>();
        subscriber->releaseQueuedData();
    }
```
!!! attention 
    In the second group we would not dismiss the data because we would be 
    notified by the WaitSet immediately again since the subscriber has still the state `HAS_DATA`.

### Individual
When every _Triggerable_ requires a different reaction we need to know the
origin of an _Event_. We can call `event.doesOriginateFrom(EventOrigin)`
which will return true if the event originated from _EventOrigin_ and
otherwise false.

We start this example by creating a _WaitSet_ with the default capacity and
attaching the `shutdownTrigger` to handle `CTRL-c`.
```cpp
iox::popo::WaitSet waitset<>;

waitset.attachEvent(shutdownTrigger).or_else([](auto) {
        std::cerr << "failed to attach shutdown trigger" << std::endl;
        std::terminate();
    });
```

Additionally, we create two subscribers and attach them with the state `SubscriberState::HAS_DATA`
to the waitset to let them inform us whenever they have samples in their queue.
```cpp
iox::popo::Subscriber<CounterTopic> subscriber1({"Radar", "FrontLeft", "Counter"});
iox::popo::Subscriber<CounterTopic> subscriber2({"Radar", "FrontLeft", "Counter"});

waitset.attachEvent(subscriber1, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
    std::cerr << "failed to attach subscriber1" << std::endl;
    std::terminate();
});
waitset.attachEvent(subscriber2, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
    std::cerr << "failed to attach subscriber2" << std::endl;
    std::terminate();
});
```

With that set up we enter the event loop and handle the program termination
first.
```cpp
while (true)
{
    auto eventVector = waitset.wait();

    for (auto& event : eventVector)
    {
        if (event->doesOriginateFrom(&shutdownTrigger))
        {
            return (EXIT_SUCCESS);
        }
```

When the origin is `subscriber1` we would like to state that subscriber 1 has received the
following number X. But for `subscriber2` we just dismiss the received samples.
We accomplish this by asking the `event` if it originated from the
corresponding subscriber. If so we act.
```cpp
        else if (event->doesOriginateFrom(&subscriber1))
        {
            subscriber1.take().and_then([&](iox::popo::Sample<const CounterTopic>& sample) {
                std::cout << " subscriber 1 received: " << sample->counter << std::endl;
            });
        }
        if (event->doesOriginateFrom(&subscriber2))
        {
            subscriber2.releaseQueuedData();
            std::cout << "subscriber 2 received something - dont care\n";
        }
```

### Sync
Let's say we have `SomeClass` and would like to execute a cyclic static method `cyclicRun`
every second. We could execute any arbitrary algorithm in there
but for now we just print `activation callback`. The class could look like
```cpp
class SomeClass
{
  public:
    static void cyclicRun(iox::popo::UserTrigger*)
    {
        std::cout << "activation callback\n";
    }
};
```
!!! attention 
    The user trigger is event based and always reset after the WaitSet 
    has acquired all triggered objects.

We begin as always, by creating a _WaitSet_ with the default capacity and by
attaching the `shutdownTrigger` to 
it. In this case we do not set an event id when calling `attachEvent` which means
the default event id  `EventInfo::INVALID_ID` is set.
```cpp
iox::popo::WaitSet<> waitset;

// attach shutdownTrigger to handle CTRL+C
waitset.attachEvent(shutdownTrigger).or_else([](auto) {
    std::cerr << "failed to attach shutdown trigger" << std::endl;
    std::terminate();
});
```

After that we require a `cyclicTrigger` to trigger our
`cyclicRun` every second. Therefore, we attach it to the `waitset` with
eventId `0` and the callback `SomeClass::cyclicRun`
```cpp
iox::popo::UserTrigger cyclicTrigger;
waitset.attachEvent(cyclicTrigger, 0U, &SomeClass::cyclicRun).or_else([](auto) {
    std::cerr << "failed to attach cyclic trigger" << std::endl;
    std::terminate();
});
```

The next thing we need is something which will trigger our `cyclicTrigger`
every second. We use a infinite loop packed inside of a thread.
```cpp
std::thread cyclicTriggerThread([&] {
    while (keepRunning.load())
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cyclicTrigger.trigger();
    }
});
```

Everything is set up and we can implement the event loop. As usual we handle
`CTRL-c` which is indicated by the `shutdownTrigger`.
```cpp
while (keepRunning.load())
{
    auto eventVector = waitset.wait();
    
    for (auto& event : eventVector)
    {
        if (event->doesOriginateFrom(&shutdownTrigger))
        {
            keepRunning.store(false);
        }
```

The `cyclicTrigger` callback is called in the else part.
```cpp
        else
        {
            (*event)();
        }
```

### Trigger
In this example we describe how you would implement a _Triggerable_ class which
can be attached to a _WaitSet_ or a [Listener](../callbacks). Our class in this example will be called
`MyTriggerClass` and it can signal the _WaitSet_ the two states `HAS_PERFORMED_ACTION` and 
`IS_ACTIVATED`. Furthermore, we can also attach the two corresponding events
`PERFORM_ACTION_CALLED` and `ACTIVATE_CALLED`.
The `PERFORMED_ACTION_CALLED` event is triggered whenever the method `performAction`
is called and the state `HAS_PERFORMED_ACTION` persists until someone resets the state
with the method `reset()`. The same goes for the event `ACTIVATE_CALLED` which is 
triggered by an `activate()` call and the corresponding state `IS_ACTIVATED` which
stays until someone resets it with `reset()`.

#### MyTriggerClass

##### Attaching States

A class which would like to attach states to a _WaitSet_ has to implement the 
following methods.
 
 1. `void enableState(iox::popo::TriggerHandle&&, const UserDefinedStateEnum )`

    Used by the _WaitSet_ to attach a trigger handle to the object so that the 
    object can notify the _WaitSet_ that it entered a certain state.

 2. `void disableState(const UserDefinedStateEnum)`

    Called whenever the user detaches the state from the _WaitSet_.

 3. `void invalidateTrigger(const uint64_t uniqueTriggerId)`

    If the _WaitSet_ goes out of scope it calls this method to invalidate the loan 
    trigger.

 4. `iox::popo::WaitSetIsConditionSatisfiedCallback getCallbackForIsStateConditionSatisfied(const UserDefinedStateEnum)`

    With every iteration the _WaitSet_ has to ask the object if the attached state 
    still persists. This is done with the `isStateConditionSatisfied` callback which 
    will be returned here.

The `UserDefinedStateEnum` can be some arbitrary enum class which requires 
`iox::popo::StateEnumIdentifier` as underlying type so that it can be identified as 
an enum which describes certain states. In our example it is called `MyTriggerClassStates`.
```cpp
enum class MyTriggerClassStates : iox::popo::StateEnumIdentifier
{
    HAS_PERFORMED_ACTION,
    IS_ACTIVATED
};
```

##### Attaching Events

Events can be attached to _WaitSets_ and [Listeners](../callbacks). For this to work 
the class has to implement the following methods.

 1. `void enableEvent(iox::popo::TriggerHandle&&, const UserDefinedEvenEnum)`

    Used by the _WaitSet_ or the _Listener_ to attach a trigger handle which signals 
    certain events to them.

 2. `void disableEvent(const UserDefinedStateEnum)`

    Called whenever the user detaches the event from the _WaitSet_ or the _Listener_.

 3. `void invalidateTrigger(const uint64_t uniqueTriggerId)`

    Used to cleanup all loan trigger handles when the _WaitSet_ or _Listener_ goes 
    out of scope.

Like with the state enum the event enum can be also any arbitrary enum class which 
has `iox::popo::EventEnumIdentifier` as an underlying type. In our example it is called 
`MyTriggerClassEvents`
```cpp
enum class MyTriggerClassEvents : iox::popo::EventEnumIdentifier
{
    PERFORM_ACTION_CALLED,
    ACTIVATE_CALLED
};
```

##### Further Requirements

 1. `friend iox::popo::EventAttorney`

    Methods like `enableEvent`, `disableEvent` etc. should never be accessible 
    via the public API and should be therefore private. To avoid that every class 
    has to befriend the _WaitSet_, _Listener_ and other internal structures we 
    implemented the client attorney pattern and the class has only to befriend 
    the `iox::popo::EventAttorney`.

 2. Deleted move and copy operations

At the moment the _WaitSet_ does not support _Triggerable_ classes which are movable
or copyable. This is caused by the `resetCallback` and the `isStateConditionSatisfied` callback
which are pointing to the _Triggerable_. After a move the callbacks inside of the _WaitSet_
would point to the wrong memory location and a copy could lead to an unattached object
if there is no more space left in the _WaitSet_. Therefore we have to delete the move
and copy operations for now.
```cpp
    MyTriggerClass(const MyTriggerClass&) = delete;
    MyTriggerClass(MyTriggerClass&&) = delete;
    MyTriggerClass& operator=(const MyTriggerClass&) = delete;
    MyTriggerClass& operator=(MyTriggerClass&&) = delete;
```

##### Implementation

The method implementation of the two actions `activate` and `performAction` which trigger an 
event and causing a state change look like the following.
```cpp
class MyTriggerClass
{
  public:
    void activate(const uint64_t activationCode) noexcept
    {
        m_activationCode = activationCode;
        m_isActivated = true;
        m_activateTrigger.trigger();
    }

    void performAction() noexcept
    {
        m_hasPerformedAction = true;
        m_onActionTrigger.trigger();
    }
```

As you can see we perform some internal action and when they are finished we
signal the corresponding _Trigger_ via our stored _TriggerHandle_ that we performed the task. Internally we
just set a boolean to signal that the method was called.

Every state based _Trigger_ requires a corresponding class method which returns a boolean
stating if the state which led to the trigger still persists. In our case these are
the two const methods `hasPerformedAction` and `isActivated`.
```cpp
    bool hasPerformedAction() const noexcept
    {
        return m_hasPerformedAction;
    }

    bool isActivated() const noexcept
    {
        return m_isActivated;
    }
```

Since the following methods should not be accessible by the public but must be 
accessible by any _Notifyable_ like the _WaitSet_ and to avoid that 
we have to befriend every possible _Notifyable_ we created the `EventAttorney`.
Every _Triggerable_ has to befriend the `EventAttorney` which provides access 
to the private methods `enableEvent`/`enableState`, `disableEvent`/`disableState`, `invalidateTrigger` and 
`getCallbackForIsStateConditionSatisfied` to all _Notifyables_.

```cpp
    friend iox::popo::EventAttorney;
```

The method `enableEvent` is called by the _WaitSet_ when a `MyTriggerClass` event
is being attached to it. During that process the _WaitSet_ creates a `triggerHandle`
and forwards the `event` to which this handle belongs. 

In the switch case statement we assign the `triggerHandle` to the corresponding
internal trigger handle.

```cpp
    void enableEvent(iox::popo::TriggerHandle&& triggerHandle,
                     const MyTriggerClassEvents event) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORM_ACTION_CALLED:
            m_onActionTrigger = std::move(triggerHandle);
            break;
        case MyTriggerClassEvents::ACTIVATE_CALLED:
            m_activateTrigger = std::move(triggerHandle);
            break;
        }
    }
```

Attaching a state works in a similar fashion.
```cpp
    void enableState(iox::popo::TriggerHandle&& triggerHandle, const MyTriggerClassStates state) noexcept
    {
        switch (state)
        {
        case MyTriggerClassStates::HAS_PERFORMED_ACTION:
            m_onActionTrigger = std::move(triggerHandle);
            break;
        case MyTriggerClassStates::IS_ACTIVATED:
            m_activateTrigger = std::move(triggerHandle);
            break;
        }
    }
```

It is possible to use the same trigger for either a state or an event attachment 
but then we loose the ability to attach the state and the corresponding event 
at the same time to a _WaitSet_. In most cases it is not a problem and when you 
attach an event when the corresponding state is already attached you will get 
a warning message on the terminal and the already attached event is detached so that 
the state can be attached. This is realized via the RAII idiom.

The next thing on our checklist is the `invalidateTrigger` method used by the WaitSet
to reset the _Trigger_ when it goes out of scope. Therefore we look up the
correct unique trigger id first and then `invalidate` it to make them unusable 
in the future.
```cpp
    void invalidateTrigger(const uint64_t uniqueTriggerId)
    {
        if (m_onActionTrigger.getUniqueId() == uniqueTriggerId)
        {
            m_onActionTrigger.invalidate();
        }
        else if (m_activateTrigger.getUniqueId() == uniqueTriggerId)
        {
            m_activateTrigger.invalidate();
        }
    }
```

Detaching an event in the _WaitSet_ will lead to a call to `disableEvent` in 
our class. In this case we have to `reset` the corresponding trigger to invalidate 
and release it from the _WaitSet_. Like before we use a switch case statement to 
find the to the event corresponding trigger.
```cpp
    void disableEvent(const MyTriggerClassEvents event) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORM_ACTION_CALLED:
            m_onActionTrigger.reset();
            break;
        case MyTriggerClassEvents::ACTIVATE_CALLED:
            m_activateTrigger.reset();
            break;
        }
    }
```

The same idea is used when detaching a state.
```cpp
    void disableState(const MyTriggerClassStates state) noexcept
    {
        switch (state)
        {
        case MyTriggerClassStates::HAS_PERFORMED_ACTION:
            m_onActionTrigger.reset();
            break;
        case MyTriggerClassStates::IS_ACTIVATED:
            m_activateTrigger.reset();
            break;
        }
    }
```

The last method we have to implement is `getCallbackForIsStateConditionSatisfied`. The
_WaitSet_ can handle state based attachments and therefore it requires, beside the condition variable
which only states that something has happened, a callback to find the object 
where it happened. This is the `isStateConditionSatisfied` callback. In our case we either return 
the method pointer to `hasPerformedAction` or `isActivated` depending on which 
state was requested.
```cpp
    iox::popo::WaitSetIsConditionSatisfiedCallback
    getCallbackForIsStateConditionSatisfied(const MyTriggerClassStates event) const noexcept
    {
        switch (event)
        {
        case MyTriggerClassStates::HAS_PERFORMED_ACTION:
            return {*this, &MyTriggerClass::hasPerformedAction};
        case MyTriggerClassStates::IS_ACTIVATED:
            return {*this, &MyTriggerClass::isActivated};
        }
        return {};
    }
```

#### Using MyTriggerClass

The next thing we define is a free function, our `eventLoop`, which will handle
all events of our _WaitSet_. Since we would like to attach the `IS_ACTIVATED` state
we have to reset the state whenever it occurs otherwise the _WaitSet_ will 
notify us right away since the state still persists. The second attachment will 
be an event attachment and the _WaitSet_ informs us just once that the event 
has occurred which makes the `reset` call obsolete.

```cpp
void eventLoop()
{
    while (true)
    {
        auto eventVector = waitset->wait();
        for (auto& event : eventVector)
        {
            if (event->getEventId() == ACTIVATE_ID)
            {
                event->getOrigin<MyTriggerClass>()->reset(MyTriggerClassStates::IS_ACTIVATED);
                (*event)();
            }
            else if (event->getEventId() == ACTION_ID)
            {
                (*event)();
            }
        }
    }
}
```

We start like in every other example by creating the `waitset` first. In this
case the `waitset` and the `triggerClass` are stored inside of two global
`optional`'s and have to be created with an `emplace` call.
```cpp
waitset.emplace();
triggerClass.emplace();
```

After that we can attach the `IS_ACTIVATED` state and `PERFORM_ACTION_CALLED` event
to the waitset and provide a callback for them.
```cpp
    waitset->attachState(*triggerClass, MyTriggerClassStates::IS_ACTIVATED, ACTIVATE_ID, &callOnActivate)
        .or_else([](auto) {
            std::cerr << "failed to attach MyTriggerClassStates::IS_ACTIVATED state " << std::endl;
            std::terminate();
        });
    waitset
        ->attachEvent(
            *triggerClass, MyTriggerClassEvents::PERFORM_ACTION_CALLED, ACTION_ID, &MyTriggerClass::callOnAction)
        .or_else([](auto) {
            std::cerr << "failed to attach MyTriggerClassEvents::PERFORM_ACTION_CALLED event " << std::endl;
            std::terminate();
        });
```

Now that everything is set up we can start our `eventLoop` in a new thread.
```cpp
    std::thread eventLoopThread(eventLoop);
```

A thread which will trigger an event every second is started with the following
lines.
```cpp
    std::thread triggerThread([&] {
        uint64_t activationCode = 1U;
        for (auto i = 0; i < 10; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->activate(activationCode++);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->performAction();
        }
    });
```
