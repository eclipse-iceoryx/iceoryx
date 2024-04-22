# WaitSet

## Thread Safety

The WaitSet is **not** thread-safe!

- It is **not** allowed to attach or detach _Triggerable_
   classes with `attachEvent` or `detachEvent` when another thread is currently
   waiting for notifications with `wait` or `timedWait`.
- Do **not** call any of the WaitSet methods concurrently.

The _TriggerHandle_ on the other hand, is thread-safe! Therefore you are allowed to
attach/detach a _TriggerHandle_ to a _Triggerable_ while another thread may
trigger the _TriggerHandle_.

## Introduction

The WaitSet is a set to which you can attach objects so that they can signal a wide variety
of events to one single notifiable. The typical approach is that one creates a
WaitSet, attaches multiple subscribers, user triggers or other _Triggerables_ to it and
then wait until one or many of the attached entities signal an event. If that happens one receives
a list of _NotificationInfos_ which is corresponding to all occurred events.

## Events and States

In this context, we define the state of an object as a specified set of values
to which the members of that object are set. An event on the other hand
is defined as a state change. Usually, an event changes the state of the corresponding
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

If you attach the state `SubscriberState::HAS_DATA` you will
be notified by `WaitSet::wait` or `WaitSet::timedWait` as long as there are received
samples present in the subscriber queue.

## Expected Output

[![asciicast](https://asciinema.org/a/RmfrWRQIULiFGt7dC8PamKGeK.svg)](https://asciinema.org/a/RmfrWRQIULiFGt7dC8PamKGeK)

## Glossary

- **Event** a state change of an object; a _Triggerable_ will signal an event via a _TriggerHandle_ to
     a _Notifyable_. For instance one can attach the subscriber event `DATA_RECEIVED` to _WaitSet_.
     This will cause the subscriber to notify the WaitSet via the _TriggerHandle_ everytime a
     sample was received.
- **NotificationCallback** a callback attached to a _NotificationInfo_. It must have the
    following signature `void ( NotificationOrigin )`. Any free function, static
    class method and non capturing lambda expression is allowed. You have to ensure the lifetime of that callback.
    This can become important when you would like to use lambda expressions.
- **NotificationId** an id which is tagged to an event. It does not need to be unique
     or follow any restrictions. The user can choose any arbitrary `uint64_t`. Assigning
     the same _NotificationId_ to multiple _Events_ can be useful when you would like to
     group _Events_.
- **NotificationInfo** a class that corresponds to _Triggers_ and is used to inform
     the user which _Event_ occurred. You can use the _NotificationInfo_ to acquire
     the _NotificationId_, call the _NotificationCallback_ or acquire the _NotificationOrigin_.
- **NotificationOrigin** the pointer to the class where the _Event_ originated from, short
     pointer to the _Triggerable_.
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
  - they have the same _NotificationOrigin_
  - they have the same callback to verify that they were triggered
       (`hasNotificationCallback`)
  - they have the same _NotificationId_
- **WaitSet** a _Notifyable_ which manages a set of _Triggers_ which are corresponding to _Events_.
     A user may attach or detach events. The _Waitset_ is listening
     to the whole set of _Triggers_ and if one or more _Triggers_ are triggered by an event it will notify
     the user. If a _WaitSet_ goes out of scope all attached _Triggers_ will be
     invalidated.

## Quick Overview

**Events** or **States** can be attached to a **Notifyable** like the **WaitSet**.
The **WaitSet** will listen on **Triggers** for a signal that an **Event** has occurred and it hands out
**TriggerHandles** to **Triggerable** objects. The **TriggerHandle** is used to inform the **WaitSet**
about the occurrence of an **Event**. When returning from `WaitSet::wait()` the user is provided with a vector of **NotificationInfos**
associated with **Events** which had occurred and **States** which persists. The **NotificationOrigin**, **NotificationId** and **NotificationCallback**
are stored inside of the **NotificationInfo** and can be acquired by the user.

!!! warning
    Please be aware of the thread-safety restrictions of the _WaitSet_ and
    read the [Thread Safety](#thread-safety) chapter carefully.

## Reference

| task | call |
|:-----|:-----|
|attach subscriber event to a WaitSet|`waitset.attachEvent(subscriber, iox::popo::SubscriberEvent::DATA_RECEIVED, 123, &mySubscriberCallback)`|
|attach subscriber state to a WaitSet|`waitset.attachState(subscriber, iox::popo::SubscriberState::HAS_DATA, 123, &mySubscriberCallback)`|
|attach user trigger to a WaitSet|`waitset.attachEvent(userTrigger, 456, &myUserTriggerCallback)`|
|wait for triggers           |`auto triggerVector = myWaitSet.wait();`  |
|wait for triggers with timeout |`auto triggerVector = myWaitSet.timedWait(1_s);`  |
|check if event/state originated from some object|`notification->doesOriginateFrom(ptrToSomeObject)`|
|get id of the event/state|`notification->getNotificationId()`|
|call eventCallback|`(*notification)()`|
|acquire _NotificationOrigin_|`notification->getOrigin<OriginType>();`|

## Use Cases

This example consists of 6 use cases.

 1. `ice_waitset_basic`: A single subscriber is notified by the WaitSet if data arrives.

 2. `ice_waitset_gateway.cpp`: We build a gateway to forward data
    to another network. A list of subscriber events are handled in an uniform way
    by defining a callback which is executed for every subscriber who
    has received data.

 3. `ice_waitset_grouping`: We would like to group multiple subscribers into 2 distinct
    groups and handle them whenever they have a specified state according to their group membership.

 4. `ice_waitset_individual`: A list of subscribers where every subscriber is
    handled differently.

 5. `ice_waitset_timer_driven_execution`: We use the WaitSet to trigger a cyclic call which should
    execute an algorithm every 1 s.

 6. `ice_waitset_trigger`: We create our own class which can be attached to a
    WaitSet to signal states and events.

## Examples

All our examples require a running `iox-roudi` and some data to receive which will be
send by `iox-cpp-waitset-publisher`. The publisher does not contain any _WaitSet_ specific
logic and is explained in detail in the
[icedelivery example](../icedelivery).

### Basic

We create one subscriber and attach it to the WaitSet. Afterwards we wait for data in
a loop and process it on arrival. To leave the loop and exit the application
we have to register a signal handler that calls `waitset.markForDestruction()`
which wakes up the blocking `waitset->wait()` whenever Ctrl+C is pressed.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_basic.cpp][sig handler]-->
```cpp
volatile bool keepRunning{true};

using WaitSet = iox::popo::WaitSet<>;
volatile WaitSet* waitsetSigHandlerAccess{nullptr};

static void sigHandler(int sig [[maybe_unused]])
{
    keepRunning = false;
    if (waitsetSigHandlerAccess)
    {
        waitsetSigHandlerAccess->markForDestruction();
    }
}
```

In the beginning we create the WaitSet. It is important to construct it only after the runtime has already been initialized since it internally depends on facilities set up by the runtime.

Afterwards we register our signal handler which will unblock the WaitSet. Finally we attach the subscriber to the WaitSet stating that we want to be notified when it has data (indicated by `iox::popo::SubscriberState::HAS_DATA`).

It is good practice to handle potential failure while attaching, otherwise warnings will emerge since the return value `expected` is marked to require handling.
In our case no errors should occur since the WaitSet can accommodate the two triggers we want to attach.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_basic.cpp][create waitset]-->
```cpp
WaitSet waitset;
waitsetSigHandlerAccess = &waitset;

// create subscriber
iox::popo::Subscriber<CounterTopic> subscriber({"Radar", "FrontLeft", "Counter"});

// attach subscriber to waitset
waitset.attachState(subscriber, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
    std::cerr << "failed to attach subscriber" << std::endl;
    std::exit(EXIT_FAILURE);
});
```

We create a loop which we will exit as soon as someone presses CTRL+C and our
signal handler sets `keepRunning` to false. If this happens `markForDestruction` turns
the `waitset->wait()` into an empty non-blocking method and makes sure that we do
not wait indefinitely.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_basic.cpp][mainloop]-->
```cpp
while (keepRunning)
{
    // We block and wait for samples to arrive.
    auto notificationVector = waitset.wait();

    for (auto& notification : notificationVector)
    {
        // We woke up and hence there must be at least one sample. When the sigHandler has called
        // markForDestruction the notificationVector is empty otherwise we know which subscriber received samples
        // since we only attached one.
        // Best practice is to always acquire the notificationVector and iterate over all elements and then react
        // accordingly. When this is not done and more elements are attached to the WaitSet it can cause
        // problems since we either miss events or handle events for objects which never occurred.
        if (notification->doesOriginateFrom(&subscriber))
        {
            // Consume a sample
            subscriber.take()
                .and_then([](auto& sample) { std::cout << " got value: " << sample->counter << std::endl; })
                .or_else([](auto& reason) {
                    std::cout << "got no data, return code: " << static_cast<uint64_t>(reason) << std::endl;
                });
            // We could consume all samples but do not need to.
            // If there is more than one sample we will wake up again since the state of the subscriber is still
            // iox::popo::SubscriberState::HAS_DATA in this case.
        }
    }
}
```

Processing just one sample even if more might have arrived will cause `wait` to
unblock again immediately to process the next sample (or shut down if requested).
Due to the overhead of the `wait` call it may still be more efficient to process
all samples in a loop until there are none left before waiting again, but it is
not required. It would be required if we attach via `attachEvent` instead of
`attachState`, since we might wake up due to the arrival of a second sample,
only process the first and will not receive a wake up until a third sample
arrives (which could be much later or never).

### Gateway

We have a list of subscribers which can be subscribed to any arbitrary topic
and everytime we receive a sample we would like to send the bytestream to a socket,
write it into a file or print it to the console. But whatever we choose to do
we perform the same task for all the subscribers. And since we process all incoming
data right away we attach the `SubscriberEvent::DATA_RECEIVED` which notifies us
only once.

Let's start by implementing our callback which prints the subscriber pointer, the
payload size and the payload pointer to the console. We have to process all samples
as long as there are samples in the subscriber queue since we attached an event that notifies
us only once. But it is impossible to miss samples since the notification is reset
right after `wait` or `timedWait` is returned - this means if a sample arrives after
those calls we will be notified again.
Additionally, since we would like to count the sum of all processed samples, we
add a second argument called `sumOfAllSamples` to the user defined context data.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_gateway.cpp][subscriber callback]-->
```cpp
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber, uint64_t* const sumOfAllSamples)
{
    while (subscriber->hasData())
    {
        subscriber->take().and_then([&](auto& userPayload) {
            auto chunkHeader = iox::mepoo::ChunkHeader::fromUserPayload(userPayload);
            auto flags = std::cout.flags();
            std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                      << chunkHeader->userPayloadSize() << " ptr: " << std::hex << chunkHeader->userPayload()
                      << std::dec << std::endl;
            std::cout.setf(flags);
        });
        // no nullptr check required since it is guaranteed != nullptr
        ++(*sumOfAllSamples);
    }
}
```

The _Event_ callback requires a signature of either `void (NotificationOrigin)` or
`void(NotificationOrigin, ContextDataType *)` when one would like to provide an additional
data pointer to the callback.
In our example the _NotificationOrigin_ is a
`iox::popo::UntypedSubscriber` pointer which we use to acquire the latest sample by calling
`take()` and the `ContextDataType` is an `uint64_t` used to count the processed
samples. When `take()` was successful we print our message to
the console inside of the `and_then` lambda.

To save some mileage on the keyboard, a type alias for _WaitSet_ with the storage capacity
for 2 events is defined.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_gateway.cpp][waitset type alias]-->
```cpp
using WaitSet = iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS>;
```

In our `main` function we create the _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_gateway.cpp][create waitset]-->
```cpp
WaitSet waitset;
waitsetSigHandlerAccess = &waitset;
```

After that we define our `sumOfAllSamples` variable and create a vector to hold our subscribers. We create and then
attach the subscribers to our _WaitSet_ with the `SubscriberEvent::DATA_RECEIVED` event and the `subscriberCallback`.
Everytime one of the subscribers is receiving a new sample it will trigger the _WaitSet_.

!!! attention
    The user has to ensure that the contextData (`sumOfAllSamples`) in `attachEvent`
    lives as long as the attachment, with its callback, is attached otherwise
    the callback context data pointer is dangling.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_gateway.cpp][configure]-->
```cpp
uint64_t sumOfAllSamples = 0U;

// create subscribers and subscribe them to our service
iox::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    auto& subscriber = subscriberVector.back();

    /// important: the user has to ensure that the 'contextData' (here 'sumOfAllSamples') lives as long as
    ///            the subscriber with its callback when attached to the 'waitset'
    waitset
        .attachEvent(subscriber,
                     iox::popo::SubscriberEvent::DATA_RECEIVED,
                     0,
                     createNotificationCallback(subscriberCallback, sumOfAllSamples))
        .or_else([&](auto) {
            std::cerr << "failed to attach subscriber" << i << std::endl;
            std::exit(EXIT_FAILURE);
        });
}
```

`attachEvent` is returning a `expected` which informs us if attaching the event
succeeded. In the `.or_else([&](auto){/*...*/})` part we perform the error handling
whenever `attachEvent` fails.

Now our system is prepared and ready to work. We enter the event loop which
starts with a call to our _WaitSet_ (`waitset.wait()`). This call will block until
one or more events triggered the _WaitSet_. After the call returned we get a
vector filled with _NotificationInfos_ which are corresponding to all the events which
triggered the _WaitSet_.

We iterate through this vector and call the assigned callback by calling
the trigger. This will then call the `subscriberCallback` with the _NotificationOrigin_
(the pointer to the untyped subscriber) and the contextData (`sumOfAllSamples`) as parameters.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_gateway.cpp][event loop]-->
```cpp
while (keepRunning)
{
    auto notificationVector = waitset.wait();

    for (auto& notification : notificationVector)
    {
        // call the callback which was assigned to the notification
        (*notification)();
    }

    auto flags = std::cout.flags();
    std::cout << "sum of all samples: " << std::dec << sumOfAllSamples << std::endl;
    std::cout.setf(flags);
}
```

### Grouping

In our next use case we would like to divide the subscribers into two groups
and we do not want to attach a callback to them. Instead we perform the calls on the
subscribers directly. Additionally, we would like to be notified as long as there
are samples in the subscriber queue therefore we have to attach the `SubscriberState::HAS_DATA`.

We again start by creating a _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_grouping.cpp][create waitset]-->
```cpp
WaitSet waitset;
waitsetSigHandlerAccess = &waitset;
```

Now we create a vector of 4 subscribers.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_grouping.cpp][create subscribers]-->
```cpp
iox::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
}
```

After that, we define our two groups with the ids `FIRST_GROUP_ID` and `SECOND_GROUP_ID`
and attach the first two subscribers with the state `SubscriberState::HAS_DATA` to the first group and the remaining subscribers to the second group.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_grouping.cpp][configure subscribers]-->
```cpp
// attach the first two subscribers to waitset with a id of FIRST_GROUP_ID
for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS / 2; ++i)
{
    waitset.attachState(subscriberVector[i], iox::popo::SubscriberState::HAS_DATA, FIRST_GROUP_ID)
        .or_else([&](auto) {
            std::cerr << "failed to attach subscriber" << i << std::endl;
            std::exit(EXIT_FAILURE);
        });
}

// attach the remaining subscribers to waitset with a id of SECOND_GROUP_ID
for (auto i = NUMBER_OF_SUBSCRIBERS / 2; i < NUMBER_OF_SUBSCRIBERS; ++i)
{
    waitset.attachState(subscriberVector[i], iox::popo::SubscriberState::HAS_DATA, SECOND_GROUP_ID)
        .or_else([&](auto) {
            std::cerr << "failed to attach subscriber" << i << std::endl;
            std::exit(EXIT_FAILURE);
        });
}
```

The event loop calls `auto notificationVector = waitset.wait()` in a blocking call to
receive a vector of all the _NotificationInfos_ which are corresponding to the occurred events.

The remaining part of the loop is handling the subscribers. In the first group
we would like to print the received data to the console and in the second group
we just dismiss the received data.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_grouping.cpp][data path]-->
```cpp
// we print the received data for the first group
if (notification->getNotificationId() == FIRST_GROUP_ID)
{
    auto subscriber = notification->getOrigin<iox::popo::UntypedSubscriber>();
    subscriber->take().and_then([&](auto& userPayload) {
        const CounterTopic* data = static_cast<const CounterTopic*>(userPayload);
        auto flags = std::cout.flags();
        std::cout << "received: " << std::dec << data->counter << std::endl;
        std::cout.setf(flags);
        subscriber->release(userPayload);
    });
}
// dismiss the received data for the second group
else if (notification->getNotificationId() == SECOND_GROUP_ID)
{
    std::cout << "dismiss data\n";
    auto subscriber = notification->getOrigin<iox::popo::UntypedSubscriber>();
    // We need to release the data to reset the trigger hasData
    // otherwise the WaitSet would notify us in 'waitset.wait()' again
    // instantly.
    subscriber->releaseQueuedData();
}
```

!!! attention
    For the second group we have to call `releaseQueuedData` to release the
    unread data. Otherwise we would be notified by the WaitSet immediately again
    since the subscriber has still the state `HAS_DATA`.

### Individual

When every _Triggerable_ requires a different reaction we need to know the
origin of an _Event_. We can call `event.doesOriginateFrom(NotificationOrigin)`
which will return true if the event originated from _NotificationOrigin_ and
otherwise false.

We start this example by creating a _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_individual.cpp][create waitset]-->
```cpp
WaitSet waitset;
waitsetSigHandlerAccess = &waitset;
```

Additionally, we create two subscribers and attach them with the state `SubscriberState::HAS_DATA`
to the _WaitSet_ to let them inform us whenever they have samples in their queue.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_individual.cpp][create subscribers]-->
```cpp
iox::popo::Subscriber<CounterTopic> subscriber1({"Radar", "FrontLeft", "Counter"});
iox::popo::Subscriber<CounterTopic> subscriber2({"Radar", "FrontLeft", "Counter"});

waitset.attachState(subscriber1, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
    std::cerr << "failed to attach subscriber1" << std::endl;
    std::exit(EXIT_FAILURE);
});
waitset.attachState(subscriber2, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
    std::cerr << "failed to attach subscriber2" << std::endl;
    std::exit(EXIT_FAILURE);
});
```

With that set up we enter the event loop and handle subscriber events.
When the origin is `subscriber1` we would like to print the received data to the
console. But for `subscriber2` we just dismiss the received samples.
We accomplish this by asking the `event` if it originated from the
corresponding subscriber. If so, we act.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_individual.cpp][[event loop][data path]]-->
```cpp
while (keepRunning)
{
    auto notificationVector = waitset.wait();

    for (auto& notification : notificationVector)
    {
        // process sample received by subscriber1
        if (notification->doesOriginateFrom(&subscriber1))
        {
            subscriber1.take().and_then(
                [&](auto& sample) { std::cout << "subscriber 1 received: " << sample->counter << std::endl; });
        }
        // dismiss sample received by subscriber2
        if (notification->doesOriginateFrom(&subscriber2))
        {
            // We need to release the samples to reset the trigger hasSamples
            // otherwise the WaitSet would notify us in 'waitset.wait()' again
            // instantly.
            subscriber2.releaseQueuedData();
            std::cout << "subscriber 2 received something - dont care\n";
        }
    }

    std::cout << std::endl;
}
```

### Timer Driven Execution

Let's say we have `SomeClass` and would like to execute a cyclic static method `cyclicRun`
every second. We could execute any arbitrary algorithm in there
but for now we just print `activation callback`. The class could look like

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_timer_driven_execution.cpp][cyclic run]-->
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
    The user trigger is event based and always reset after the _WaitSet_
    has acquired all triggered objects.

As always, we begin by creating a _WaitSet_.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_timer_driven_execution.cpp][create waitset]-->
```cpp
iox::popo::WaitSet<> waitset;
waitsetSigHandlerAccess = &waitset;
```

After that we require a `cyclicTrigger` to trigger our
`cyclicRun` every second. Therefore, we attach it to the `waitset` with
eventId `0` and the callback `SomeClass::cyclicRun`

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_timer_driven_execution.cpp][create trigger]-->
```cpp
iox::popo::UserTrigger cyclicTrigger;
waitset.attachEvent(cyclicTrigger, 0U, createNotificationCallback(SomeClass::cyclicRun)).or_else([](auto) {
    std::cerr << "failed to attach cyclic trigger" << std::endl;
    std::exit(EXIT_FAILURE);
});
```

The next thing we need is something which will trigger our `cyclicTrigger`
every second. We use an infinite loop packed inside of a thread.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_timer_driven_execution.cpp][cyclic thread]-->
```cpp
std::thread cyclicTriggerThread([&] {
    while (keepRunning.load())
    {
        cyclicTrigger.trigger();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
});
```

The `cyclicTrigger` callback is called in the loop.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_timer_driven_execution.cpp][[event loop][data path]]-->
```cpp
while (keepRunning.load())
{
    auto notificationVector = waitset.wait();

    for (auto& notification : notificationVector)
    {
        // call SomeClass::cyclicRun
        (*notification)();
    }

    std::cout << std::endl;
}
```

### Trigger

In this example we describe how you would implement a _Triggerable_ class which
can be attached to a _WaitSet_ or a
[Listener](../callbacks).
Our class in this example will be called `MyTriggerClass` and it can signal the _WaitSet_
the two states `HAS_PERFORMED_ACTION` and `IS_ACTIVATED`. Furthermore, we can also attach the
two corresponding events `PERFORM_ACTION_CALLED` and `ACTIVATE_CALLED`. The
`PERFORM_ACTION_CALLED` event is triggered whenever the method `performAction` is called and
the state `HAS_PERFORMED_ACTION` persists until someone resets the state with the method
`reset()`. The same applies to the event `ACTIVATE_CALLED` which is triggered by an `activate()`
call and the corresponding state `IS_ACTIVATED` which stays until someone resets it with
`reset()`.

#### MyTriggerClass

##### Attaching States

A class that would like to attach states to a _WaitSet_ has to implement the
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

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][state enum]-->
```cpp
enum class MyTriggerClassStates : iox::popo::StateEnumIdentifier
{
    HAS_PERFORMED_ACTION,
    IS_ACTIVATED
};
```

##### Attaching Events

Events can be attached to _WaitSets_ and
[Listeners](../callbacks).
For this to work the class has to implement the following methods.

 1. `void enableEvent(iox::popo::TriggerHandle&&, const UserDefinedEventEnum)`

    Used by the _WaitSet_ or the _Listener_ to attach a trigger handle which signals
    certain events to them.

 2. `void disableEvent(const UserDefinedEventEnum)`

    Called whenever the user detaches the event from the _WaitSet_ or the _Listener_.

 3. `void invalidateTrigger(const uint64_t uniqueTriggerId)`

    Used to clean up all loan trigger handles when the _WaitSet_ or _Listener_ goes
    out of scope.

Like with the state enum the event enum can be also any arbitrary enum class which
has `iox::popo::EventEnumIdentifier` as an underlying type. In our example it is called
`MyTriggerClassEvents`.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][event enum]-->
```cpp
enum class MyTriggerClassEvents : iox::popo::EventEnumIdentifier
{
    PERFORM_ACTION_CALLED,
    ACTIVATE_CALLED
};
```

##### Further Requirements

 1. `friend iox::popo::NotificationAttorney`

    Methods like `enableEvent`, `disableEvent` etc. should never be accessible
    via the public API and should be therefore private. To avoid that every class
    has to befriend the _WaitSet_, _Listener_ and other internal structures we
    implemented the client attorney pattern and the class has only to befriend
    the `iox::popo::NotificationAttorney`.

 2. Deleted move and copy operations

At the moment the _WaitSet_ does not support _Triggerable_ classes which are movable
or copyable. This is caused by the `resetCallback` and the `isStateConditionSatisfied` callback
which are pointing to the _Triggerable_. After a move the callbacks inside of the _WaitSet_
would point to the wrong memory location and a copy could lead to an unattached object
if there is no more space left in the _WaitSet_. Therefore we have to delete the move
and copy operations for now.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][no move and copy]-->
```cpp
MyTriggerClass(const MyTriggerClass&) = delete;
MyTriggerClass(MyTriggerClass&&) = delete;
MyTriggerClass& operator=(const MyTriggerClass&) = delete;
MyTriggerClass& operator=(MyTriggerClass&&) = delete;
```

##### Implementation

The method implementation of the two actions `activate` and `performAction` which trigger an
event and causing a state change look like the following.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][activate and performAction]-->
```cpp
// When you call this method you will trigger the ACTIVATE event
void activate(const uint64_t activationCode) noexcept
{
    m_activationCode = activationCode;
    m_isActivated = true;
    m_activateTrigger.trigger();
}

// Calling this method will trigger the PERFORMED_ACTION event
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

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][state checks]-->
```cpp
// required by the m_onActionTrigger to ask the class if it was triggered
bool hasPerformedAction() const noexcept
{
    return m_hasPerformedAction;
}

// required by the m_activateTrigger to ask the class if it was triggered
bool isActivated() const noexcept
{
    return m_isActivated;
}
```

Since the following methods should not be accessible by the public but must be
accessible by any _Notifyable_ like the _WaitSet_ and to avoid that
we have to befriend every possible _Notifyable_ we created the `NotificationAttorney`.
Every _Triggerable_ has to befriend the `NotificationAttorney` which provides access
to the private methods `enableEvent`/`enableState`, `disableEvent`/`disableState`, `invalidateTrigger` and
`getCallbackForIsStateConditionSatisfied` to all _Notifyables_.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][attorney]-->
```cpp
friend iox::popo::NotificationAttorney;
```

The method `enableEvent` is called by the _WaitSet_ when a `MyTriggerClass` event
is being attached to it. During that process the _WaitSet_ creates a `triggerHandle`
and forwards the `event` to which this handle belongs.

In the switch case statement we assign the `triggerHandle` to the corresponding
internal trigger handle.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][enableEvent]-->
```cpp
void enableEvent(iox::popo::TriggerHandle&& triggerHandle, const MyTriggerClassEvents event) noexcept
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

Attaching a state works in a similar way.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][enableState]-->
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

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][invalidateTrigger]-->
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
find the trigger corresponding to the event.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][disableEvent]-->
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

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][disableState]-->
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

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][condition satisfied]-->
```cpp
iox::popo::WaitSetIsConditionSatisfiedCallback
getCallbackForIsStateConditionSatisfied(const MyTriggerClassStates event) const noexcept
{
    switch (event)
    {
    case MyTriggerClassStates::HAS_PERFORMED_ACTION:
        return iox::popo::WaitSetIsConditionSatisfiedCallback(
            iox::in_place, *this, &MyTriggerClass::hasPerformedAction);
    case MyTriggerClassStates::IS_ACTIVATED:
        return iox::popo::WaitSetIsConditionSatisfiedCallback(iox::in_place, *this, &MyTriggerClass::isActivated);
    }
    return iox::nullopt;
}
```

#### Using MyTriggerClass

The next thing we define is a free function, our `eventLoop`, which will handle
all events of our _WaitSet_. Since we would like to attach the `IS_ACTIVATED` state
we have to reset the state whenever it occurs otherwise the _WaitSet_ will
notify us right away since the state still persists. The second attachment will
be an event attachment and the _WaitSet_ informs us just once that the event
has occurred which makes the `reset` call obsolete.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][event loop]-->
```cpp
void eventLoop(WaitSet& waitset)
{
    while (keepRunning)
    {
        auto notificationVector = waitset.wait();
        for (auto& notification : notificationVector)
        {
            if (notification->getNotificationId() == ACTIVATE_ID)
            {
                // reset MyTriggerClass instance state
                notification->getOrigin<MyTriggerClass>()->reset(MyTriggerClassStates::IS_ACTIVATED);
                // call the callback attached to the trigger
                (*notification)();
            }
            else if (notification->getNotificationId() == ACTION_ID)
            {
                // reset is not required since we attached an notification here. we will be notified once
                (*notification)();
            }
        }
    }
}
```

We start like in every other example by creating the `waitset` first.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][create]-->
```cpp
WaitSet waitset;
MyTriggerClass triggerClass;
```

After that we can attach the `IS_ACTIVATED` state and `PERFORM_ACTION_CALLED` event
to the waitset and provide a callback for them.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][attach]-->
```cpp
// attach the IS_ACTIVATED state to the waitset and assign a callback
waitset
    .attachState(triggerClass,
                 MyTriggerClassStates::IS_ACTIVATED,
                 ACTIVATE_ID,
                 iox::popo::createNotificationCallback(callOnActivate))
    .or_else([](auto) {
        std::cerr << "failed to attach MyTriggerClassStates::IS_ACTIVATED state " << std::endl;
        std::exit(EXIT_FAILURE);
    });
// attach the PERFORM_ACTION_CALLED event to the waitset and assign a callback
waitset
    .attachEvent(triggerClass,
                 MyTriggerClassEvents::PERFORM_ACTION_CALLED,
                 ACTION_ID,
                 iox::popo::createNotificationCallback(MyTriggerClass::callOnAction))
    .or_else([](auto) {
        std::cerr << "failed to attach MyTriggerClassEvents::PERFORM_ACTION_CALLED event " << std::endl;
        std::exit(EXIT_FAILURE);
    });
```

Now that everything is set up we can start our `eventLoop` in a new thread.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][start event loop]-->
```cpp
std::thread eventLoopThread(eventLoop, std::ref(waitset));
```

A thread which will trigger an event every second is started with the following
lines.

<!--[geoffrey][iceoryx_examples/waitset/ice_waitset_trigger.cpp][start trigger]-->
```cpp
std::thread triggerThread([&] {
    uint64_t activationCode = 1U;
    for (auto i = 0U; i < 10; ++i)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        triggerClass.activate(activationCode++);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        triggerClass.performAction();
    }

    std::cout << "Sending final trigger" << std::endl;
    keepRunning = false;
    triggerClass.activate(activationCode++);
    triggerClass.performAction();
});
```

<center>
[Check out waitset on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/waitset){ .md-button } <!--NOLINT github url required for website-->
</center>
