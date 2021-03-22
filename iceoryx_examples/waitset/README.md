# WaitSet

The WaitSet is a set where you can attach objects so that they can signal a wide variety
of events to one single notifyable. The typical approach is that one creates a
WaitSet attaches multiple subscribers, user trigger or other _Triggerables_ to it and then wait till
one or many of the attached entities signal an event. If that happens one receives
a list of _EventInfos_ which is corresponding to all occurred events.

WaitSet events can be state based, this means that the WaitSet will notify you
till you reset the state. The `HAS_DATA` event of the subscriber for instance
will notify you as long as there are samples. But it is also possible that one
attaches one shot events. These are events which will trigger the WaitSet only once.

## Threadsafety
The WaitSet is **not** threadsafe!
- It is **not** allowed to attach or detach _Triggerable_
   classes with `attachEvent` or `detachEvent` when another thread is currently
   waiting for events with `wait`.

The _TriggerHandle_ on the other hand is threadsafe! Therefore you are allowed to
attach/detach a _TriggerHandle_ to a _Triggerable_ while another thread may
trigger the _TriggerHandle_.

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
 - **Events** a _Triggerable_ will signal an event via a _TriggerHandle_ to a _Notifyable_.
     For instance one can attach the subscriber event `HAS_DATA` to _WaitSet_. This will cause the
     subscriber to notify the WaitSet via the _TriggerHandle_ everytime when a sample was received.
 - **Notifyable** is a class which listens to events. A _TriggerHandle_ which corresponds to a _Trigger_
     is used to notify the _Notifyable_ that an event occurred. The WaitSet is a _Notifyable_.
 - **Trigger** a class which is used by the _Notifyable_ to acquire the information which events were
     signalled. It corresponds to a _TriggerHandle_. If the _Notifyable_ goes out of scope the corresponding
     _TriggerHandle_ will be invalidated and if the _Triggerable_ goes out of scope the corresponding
     _Trigger_ will be invalidated.
 - **Triggerable** a class which has attached a _TriggerHandle_ to itself to signal
     certain _Events_ to a _Notifyable_.
 - **TriggerHandle** a threadsafe class which can be used to trigger a _Notifyable_.
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
To a **Notifyable** like the **WaitSet** **Events** can be attached or detached.
The **WaitSet** will listen on **Triggers** for a signal that an **Event** has occurred and it hands out
**TriggerHandles** to **Triggerable** objects. The **TriggerHandle** is used to inform the **WaitSet**
about the occurrence of an **Event**. When returning from `WaitSet::wait()` the user is provided with a vector of **EventInfos**
associated with **Events** which had occurred. The **EventOrigin**, **EventId** and **EventCallback**
are stored inside of the **EventInfo** and can be acquired by the user.

## Reference

| task | call |
|:-----|:-----|
|attach subscriber to a WaitSet|`waitset.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_DATA, 123, &mySubscriberCallback)`|
|attach user trigger to a WaitSet|`waitset.attachEvent(userTrigger, 456, &myUserTriggerCallback)`|
|wait for triggers           |`auto triggerVector = myWaitSet.wait();`  |
|wait for triggers with timeout |`auto triggerVector = myWaitSet.timedWait(1_s);`  |
|check if event originated from some object|`event.doesOriginateFrom(ptrToSomeObject)`|
|get id of the event|`event.getEventId()`|
|call eventCallback|`event()`|
|acquire _EventOrigin_|`event.getOrigin<OriginType>();`|

## Use cases
This example consists of 5 use cases.
 
 1. `ice_waitset_gateway.cpp`: We build a gateway to forward data
    to another network. A list of subscribers is handled in an uniform way
    by defining a callback and which is executed for every subscriber who
    has received data.

 2. `ice_waitset_grouping`: We would like to group multiple subscribers into 2 distinct
    groups and handle them according to their group membership.

 3. `ice_waitset_individual`: A list of subscribers where every subscriber is
    handled differently.

 4. `ice_waitset_sync`: We use the WaitSet to trigger a cyclic call which should
    execute an algorithm every 100ms.

 5. `ice_waitset_trigger`: We create our own class which can be attached to a
    WaitSet to signal events.

## Examples

All our examples require a running `iox-roudi` and some data to receive which will be
send by `iox-ex-waitset-publisher`. The publisher does not contain any _WaitSet_ specific
logic and is explained in detail in the [icedelivery example](../icedelivery/).

<!-- @todo Add expected output with asciinema recording before v1.0-->

### Gateway

We have a list of subscribers which can be subscribed to any arbitrary topic
and everytime we received a sample we would like to send the bytestream to a socket,
write it into a file or print it to the console. But whatever we choose to do
we perform the same task for all the subscribers.

Let's start by implementing our callback which prints the subscriber pointer, the
payload size and the payload pointer to the console.
```cpp
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber)
{
    subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
        std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                  << sample.getHeader()->payloadSize << " ptr: " << std::hex << sample.getHeader()->payload()
                  << std::endl;
    });
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

waitset.attachEvent(shutdownTrigger);
```

After that we create a vector to hold our subscribers, we create and then
attach them to a _WaitSet_ with the `HAS_DATA` event and the `subscriberCallback`.
Everytime one 
of the subscribers is receiving a new sample it will trigger the _WaitSet_.
```cpp
iox::cxx::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
for (auto i = 0; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    auto& subscriber = subscriberVector.back();

    waitset.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_DATA, &subscriberCallback);
}
```

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
subscribers directly.

We again start by creating a _WaitSet_ with a capacity of 5 (4 subscribers and 1 shutdownTrigger),
and attach the `shutdownTrigger` to handle `CTRL+c`.
```cpp
iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS + ONE_SHUTDOWN_TRIGGER> waitset;

waitset.attachEvent(shutdownTrigger);
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
and attach the first two subscribers to the first group and the remaining subscribers
to the second group.
```cpp
for (auto i = 0; i < NUMBER_OF_SUBSCRIBERS / 2; ++i)
{
    waitset.attachEvent(subscriberVector[i], iox::popo::SubscriberEvent::HAS_DATA, FIRST_GROUP_ID);
}

for (auto i = NUMBER_OF_SUBSCRIBERS / 2; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    waitset.attachEvent(subscriberVector[i], iox::popo::SubscriberEvent::HAS_DATA, SECOND_GROUP_ID);
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
**Important** The second group needs to release all queued samples otherwise
the WaitSet would notify the user again and again that the subscriber from the second
group has new samples.

### Individual
When every _Triggerable_ requires a different reaction we need to know the
origin of an _Event_. We can call `event.doesOriginateFrom(EventOrigin)`
which will return true if the event originated from _EventOrigin_ and
otherwise false.

We start this example by creating a _WaitSet_ with the default capacity and
attaching the `shutdownTrigger` to handle `CTRL-c`.
```cpp
iox::popo::WaitSet waitset<>;

waitset.attachEvent(shutdownTrigger);
```

Additionally, we create two subscribers and attach
them to the waitset to let them inform us whenever they receive a new sample.
```cpp
iox::popo::Subscriber<CounterTopic> subscriber1({"Radar", "FrontLeft", "Counter"});
iox::popo::Subscriber<CounterTopic> subscriber2({"Radar", "FrontLeft", "Counter"});

waitset.attachEvent(subscriber1, iox::popo::SubscriberEvent::HAS_DATA);
waitset.attachEvent(subscriber2, iox::popo::SubscriberEvent::HAS_DATA);
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
        trigger->resetTrigger();
    }
};
```
**Important** We need to reset the user trigger otherwise the _WaitSet_ would notify
us immediately again since the user trigger is state based.

We begin as always, by creating a _WaitSet_ with the default capacity and by
attaching the `shutdownTrigger` to 
it. In this case we do not set an event id when calling `attachEvent` which means
the default event id  `EventInfo::INVALID_ID` is set.
```cpp
iox::popo::WaitSet<> waitset;

// attach shutdownTrigger to handle CTRL+C
waitset.attachEvent(shutdownTrigger);
```

After that we require a `cyclicTrigger` to trigger our
`cyclicRun` every second. Therefore, we attach it to the `waitset` with
eventId `0` and the callback `SomeClass::cyclicRun`
```cpp
iox::popo::UserTrigger cyclicTrigger;
waitset.attachEvent(cyclicTrigger, 0U, &SomeClass::cyclicRun);
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
while (true)
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
can be attached to a _WaitSet_. Our class in this example will be called
`MyTriggerClass` and it signals the _WaitSet_ two events.
The `PERFORMED_ACTION` event which is triggered whenever the method `performAction`
is called and the
`ACTIVATE` event which is triggered when `activate` is called with an `activationCode`.

#### MyTriggerClass

At the moment the _WaitSet_ does not support _Triggerable_ classes which are movable
or copyable. This is caused by the `resetCallback` and the `hasEventCallback`
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

The class implementation of these two methods could look like the following.
```cpp
class MyTriggerClass
{
  public:
    void activate(const int activationCode) noexcept
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

Every _Trigger_ requires a corresponding class method which returns a boolean
stating if the _Trigger_ was actually triggered or not. In our case these are
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
to the private methods `enableEvent`, `disableEvent`, `invalidateTrigger` and 
`getHasTriggeredCallbackForEvent` to all _Notifyables_.

```cpp
    friend iox::popo::EventAttorney;
```

The method `enableEvent` is called by the _WaitSet_ when `MyTriggerClass`
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
        case MyTriggerClassEvents::PERFORMED_ACTION:
            m_onActionTrigger = std::move(triggerHandle);
            break;
        case MyTriggerClassEvents::ACTIVATE:
            m_activateTrigger = std::move(triggerHandle);
            break;
        }
    }
```

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
        case MyTriggerClassEvents::PERFORMED_ACTION:
            m_onActionTrigger.reset();
            break;
        case MyTriggerClassEvents::ACTIVATE:
            m_activateTrigger.reset();
            break;
        }
    }
```

The last method we have to implement is `getHasTriggeredCallbackForEvent`. The
_WaitSet_ is state based and therefore it requires, beside the condition variable
which only states that something has happened, a callback to find the object 
where it happened. This is the `hasTriggerCallback`. In our case we either return 
the method pointer to `hasPerformedAction` or `isActivated` depending on which 
event was requested.
```cpp
    iox::popo::WaitSetHasTriggeredCallback 
    getHasTriggeredCallbackForEvent(const MyTriggerClassEvents event) const noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
            return {*this, &MyTriggerClass::hasPerformedAction};
        case MyTriggerClassEvents::ACTIVATE:
            return {*this, &MyTriggerClass::isActivated};
        }
        return {};
    }
```

#### Using MyTriggerClass

The next thing we define is a free function, our `eventLoop`, which will handle
all events of our waitset. The action is for every trigger the same, resetting
the `MyTriggerClass` event and then call the callback which is attached to the
trigger.
```cpp
void eventLoop()
{
    while (true)
    {
        auto triggerStateVector = waitset->wait();
        for (auto& triggerState : triggerStateVector)
        {
            if (triggerState->getEventId() == ACTIVATE_ID)
            {
                triggerState->getOrigin<MyTriggerClass>()->reset(MyTriggerClassEvents::ACTIVATE);
                (*triggerState)();
            }
            else if (triggerState->getEventId() == ACTION_ID)
            {
                triggerState->getOrigin<MyTriggerClass>()->reset(MyTriggerClassEvents::PERFORMED_ACTION);
                (*triggerState)();
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

After that we can attach both `triggerClass` events to the waitset and provide
also a callback for them.
```cpp
    waitset->attachEvent(*triggerClass, MyTriggerClassEvents::ACTIVATE, ACTIVATE_ID, &callOnActivate);
    waitset->attachEvent(
        *triggerClass, MyTriggerClassEvents::PERFORMED_ACTION, ACTION_ID, &MyTriggerClass::callOnAction);
```

Now that everything is set up we can start our `eventLoop` in a new thread.
```cpp
    std::thread eventLoopThread(eventLoop);
```

A thread which will trigger an event every second is started with the following
lines.
```cpp
    std::thread triggerThread([&] {
        int activationCode = 1;
        for (auto i = 0; i < 10; ++i)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->activate(activationCode++);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->performAction();
        }
    });
```
