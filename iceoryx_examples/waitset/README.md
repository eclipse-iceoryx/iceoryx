# WaitSet

The WaitSet is a set where you can attach Trigger to signal a wide variety
of events to one single listener. The typical approach is that one creates a
WaitSet attaches multiple subscriber or other Trigger to it and then wait till
one or many of the attach classes signal an event. If that happens one receives
a list of all the Triggers which were triggered and the program can act accordingly.

The WaitSet is state based which means that it will trigger until the state which
caused the trigger changed. For instance, when the subscriber reports to the WaitSet
that a new sample has been received, the WaitSet will notify the user until the
sample has been taken and released.

## Glossary

 - **Listener** a class which can be triggered by a _Trigger_, for instance a _WaitSet_.
 - **Trigger** a class which can be used to trigger a listener. If a _Trigger_ goes
     out of scope it will detach itself from the listener. A _Trigger_ is
     logical equal to another _Trigger_ if they:
     - are attached to the same _Listener_ (or in other words they are using the 
       same `ConditionVariable`)
     - they have the same _TriggerOrigin_
     - they have the same callback to verify that they were triggered 
       (`hasTriggerCallback`)
     - they have the same _TriggerId_
 - **Triggerable** a class which has attached a _Trigger_ to itself to trigger
     certain events to a _Listener_.
 - **TriggerCallback** a callback attached to a _Trigger_. It must have the 
     following signature `void ( TriggerOrigin )`. Any free function and static
     class method is allowed. You have to ensure the lifetime of that callback.
     This can become important when you would like to use lambdas.
 - **TriggerId** a id which identifies the trigger. It does not follow any 
     restrictions and the user can choose any arbitrary `uint64_t`.
 - **TriggerOrigin** the pointer to the class where the trigger originated from, short
     pointer to the _Triggerable_.
 - **WaitSet** a _Listener_ which manages a set of _Triggers_ which can be acquired by 
     the user. The _Waitset_ listens 
     to the whole set of _Triggers_ and if one or more _Trigger_ are triggered it will notify
     the user. If a _WaitSet_ goes out of scope all attached _Triggers_ will be
     invalidated.

## Reference

| task | call |
|:-----|:-----|
|attach subscriber to waitset (simple)|`subscriber.attachToWaitset(myWaitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);`|
|attach subscriber to waitset (full)|`subscriber.attachToWaitset(myWaitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, someTriggerId, myCallback);`|
|attach user trigger to waitset (simple)|`userTrigger.attachToWaitset(myWaitSet)`|
|attach user trigger to waitset (full)|`userTrigger.attachToWaitset(myWaitSet, someTriggerId, myCallback)`|
|wait for triggers           |`auto triggerVector = myWaitSet.wait();`  |
|wait for triggers with timeout |`auto triggerVector = myWaitSet.timedWait(1_s);`  |
|check if trigger originated from some class|`trigger.doesOriginateFrom(ptrToSomeClass)`|
|get id of trigger|`trigger.getTriggerId()`|
|call triggerCallback|`trigger()`|
|acquire _TriggerOrigin|`trigger.getOrigin<OriginType>();`|
|check if 2 trigger are logical equal|`trigger.isLogicalEqualTo(anotherTrigger)`|

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

## Examples

All our examples require a running `iox-roudi` and a running `iox-ex-waitset-publisher`
which is sending the data. The publisher does not contain any _WaitSet_ specific
logic and is explained in detail in the [icedelivery example](../icedelivery/).

### Gateway

We have a list of subscribers which can be subscribed to any arbitrary service
and everytime we received a sample we would like to send the bytestream to a socket,
write it into a file or print it to the console. But whatever we choose to do
we perform the same task for all the subscribers.

Lets start by implementing our callback which prints the subscriber pointer, the
payload size and the payload pointer to the console.
```cpp
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber)
{
    subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
        std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                  << sample.getHeader()->m_info.m_payloadSize << " ptr: " << std::hex << sample.getHeader()->payload()
                  << std::endl;
    });
}
```
A _Trigger_ always requires a callback which has the following signature 
`void (TriggerOrigin)`. In our example the _TriggerOrigin_ is a
`iox::popo::UntypedSubscriber*` which we use to acquire the latest sample which the subscriber
has received (`take()`). When `take()` was successful we print our message to
the console inside of the `and_then` lambda.

In our `main` function we create a _WaitSet_ after we registered at our central
broker RouDi and attach our `shutdownGuard` to it to handle `CTRL+c` events. 
```cpp
iox::popo::WaitSet waitset;

shutdownGuard.attachToWaitset(waitset);
```

After that we create a vector of 2 subscribers, subscribe and attach them to a
_WaitSet_ with the event `HAS_NEW_SAMPLES` and the `subscriberCallback`. Everytime one 
of the subscribers is receiving a new sample it will trigger the _WaitSet_.
```cpp
iox::cxx::vector<iox::popo::UntypedSubscriber, 2> subscriberVector;
for (auto i = 0; i < subscriberVector.capacity(); ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    auto& subscriber = subscriberVector.back();

    subscriber.subscribe();
    subscriber.attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, 1, subscriberCallback);
}
```

Now our system is prepared and ready to work. We enter the event loop which 
starts with a call to our _WaitSet_ (`waitset.wait()`). This call we block until
one or more events triggered the _WaitSet_. After the call returned we get a
vector filled with all the _Triggers_ which were triggered.

We iterate through this vector, if a _Trigger_ originated from the `shutdownGuard`
we exit the program otherwise we just call the assigned callback by calling
the trigger. This will then call `subscriberCallback` with the _TriggerOrigin_
as parameter.
```cpp
while (true)
{
    auto triggerVector = waitset.wait();

    for (auto& trigger : triggerVector)
    {
        if (trigger.doesOriginateFrom(&shutdownGuard))
        {
            // CTRL+c was pressed -> exit
            return (EXIT_SUCCESS);
        }
        else
        {
            // call the callback which was assigned to the trigger
            trigger();
        }
// .... 
```

### Grouping
In our next use case we would like to divide the subscribers into two groups
and we do not want to attach a callback to them. Instead we perform the calls on the
subscribers directly.

We again start by creating a _WaitSet_ and attach the `shutdownGuard` to handle
`CTRL+c`.
```cpp
iox::popo::WaitSet waitset;

shutdownGuard.attachToWaitset(waitset);
```

Now we create a vector of 4 subscribers and subscribe them to our service.
```cpp
iox::cxx::vector<iox::popo::UntypedSubscriber, 4> subscriberVector;
for (auto i = 0; i < 4; ++i)
{
    subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    auto& subscriber = subscriberVector.back();

    subscriber.subscribe();
}
```

After that we define our two groups with the ids `FIRST_GROUP_ID` and `SECOND_GROUP_ID`
and attach the first two subscribers to the first group and the remaining subscribers
to the second group.
```cpp
for (auto i = 0; i < 2; ++i)
{
    subscriberVector[i].attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, FIRST_GROUP_ID);
}

for (auto i = 2; i < 4; ++i)
{
    subscriberVector[i].attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, SECOND_GROUP_ID);
}
```

The event loop calls `auto triggerVector = waitset.wait()` in a blocking call to
receive a vector of all the _Triggers_ which were triggered. If the _Trigger_
did originate from the `shutdownGuard` we terminate the program.
```cpp
while (true)
{
    auto triggerVector = waitset.wait();

:W
    for (auto& trigger : triggerVector)
    {
        if (trigger.doesOriginateFrom(&shutdownGuard))
        {
            return (EXIT_SUCCESS);
        }
```

The remaining part of the loop is handling the subscribers. For the first group
we would like to print the received data to the console and for the second group
we just dismiss the received data.
```cpp
else if (trigger.getTriggerId() == FIRST_GROUP_ID)
{
    auto subscriber = trigger.getOrigin<iox::popo::UntypedSubscriber>();
    subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
        const CounterTopic* data = reinterpret_cast<const CounterTopic*>(sample.get());
        std::cout << "received: " << std::dec << data->counter << std::endl;
    });
}
else if (trigger.getTriggerId() == SECOND_GROUP_ID)
{
    std::cout << "dismiss data\n";
    auto subscriber = trigger.getOrigin<iox::popo::UntypedSubscriber>();
    subscriber->releaseQueuedSamples();
}
```
**Important** The second group needs to release all queued samples otherwise
the WaitSet would notify the user again that the subscriber from the second 
group has new samples.

### Individual
When every _Triggerable_ requires a different reaction we need to know the 
origin of a _Trigger_. We can call `trigger.doesOriginateFrom(TriggerOrigin)`
which will return true if the trigger originates from _TriggerOrigin_ and
otherwise false.

We start this example by creating a _WaitSet_ and attaching the `shutdownGuard`
to handle `CTRL-c`.
```cpp
iox::popo::WaitSet waitset;

shutdownGuard.attachToWaitset(waitset);
```

Additionally, we create two subscriber, subscribe them to our service and attach
them to the waitset to let them inform us whenever they receive a new sample.
```cpp
iox::popo::TypedSubscriber<CounterTopic> subscriber1({"Radar", "FrontLeft", "Counter"});
iox::popo::TypedSubscriber<CounterTopic> subscriber2({"Radar", "FrontLeft", "Counter"});

subscriber1.subscribe();
subscriber2.subscribe();

subscriber1.attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
subscriber2.attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
```

With that set up we enter the event loop and handle the program termination
first.
```cpp
while (true)
{
    auto triggerVector = waitset.wait();

    for (auto& trigger : triggerVector)
    {
        if (trigger.doesOriginateFrom(&shutdownGuard))
        {
            return (EXIT_SUCCESS);
        }
```

If `subscriber1` we would like to state that subscriber 1 has received the 
following number X. But for `subscriber2` we just dismiss the received samples.
We accomplish this by asking the `trigger` if it originated from the 
corresponding subscriber. If so we act.
```cpp
        else if (trigger.doesOriginateFrom(&subscriber1))
        {
            subscriber1.take().and_then([&](iox::popo::Sample<const CounterTopic>& sample) {
                std::cout << " subscriber 1 received: " << sample->counter << std::endl;
            });
        }
        if (trigger.doesOriginateFrom(&subscriber2))
        {
            subscriber2.releaseQueuedSamples();
            std::cout << "subscriber 2 received something - dont care\n";
        }
```

### Sync
Lets say we have `SomeClass` and would like to execute a cyclic static call 
`cyclicRun`
in that class every second. We could execute any arbitrary algorithm in there
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
**Important** We need to reset the trigger otherwise the WaitSet would notify
us immediately again since it is state based.

We begin as always, by creating a _WaitSet_ and attaching the `shutdownGuard` to 
it.
```cpp
iox::popo::WaitSet waitset;

// attach shutdownGuard to handle CTRL+C
shutdownGuard.attachToWaitset(waitset);
```

After that we require a `cyclicTrigger` to trigger our 
`cyclicRun` every second. Therefore, we attach it to the `waitset` with 
triggerId `0` and the callback `SomeClass::cyclicRun`
```cpp
iox::popo::UserTrigger cyclicTrigger;
cyclicTrigger.attachToWaitset(waitset, 0, SomeClass::cyclicRun);
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
`CTRL-c` which is indicated by the `shutdownGuard`.
```cpp
while (true)
{
    auto triggerVector = waitset.wait();

    for (auto& trigger : triggerVector)
    {
        if (trigger.doesOriginateFrom(&shutdownGuard))
        {
            // CTRL+c was pressed -> exit
            keepRunning.store(false);
            cyclicTriggerThread.join();
            return (EXIT_SUCCESS);
        }
```

The `cyclicTrigger` callback is called in the else part.
```cpp
        else
        {
            // call SomeClass::myCyclicRun
            trigger();
        }
```

### Trigger
In this example we describe how you would implement a _Triggerable_ class which 
can be attached to a _WaitSet_. Our class in this example we be called 
`MyTriggerClass` and it signals the _WaitSet_ two events. 
The `PERFORMED_ACTION` event which is triggered whenever someone calls and
`ACTIVATE` which is triggered when `activate` is called with an `activationCode`.

#### MyTriggerClass

At the moment the WaitSet does not support _Triggerable_ classes which are move 
or copyable. This is caused by the `resetCallback` and the `hasTriggerCallback`
which are pointing to the _Triggerable_. The callbacks inside of the WaitSet 
would point to the wrong memory location. Therefore we have to delete move 
and copy operations.
```cpp
    MyTriggerClass(const MyTriggerClass&) = delete;
    MyTriggerClass(MyTriggerClass&&) = delete;
    MyTriggerClass& operator=(const MyTriggerClass&) = delete;
    MyTriggerClass& operator=(MyTriggerClass&&) = delete;
```

The class implementation of these two methods could like the following.
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
        m_actionTrigger.trigger();
    }
```

As you can see we perform some internal action and when they are finished we
signal the corresponding _Trigger_ that we performed the task. Internally we
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

The method `attachToWaitset` attaches our class to a WaitSet but the user has
to specify which event they would like to attach. Additionally, they can
set a `triggerId` and a `callback`.

If the parameter event was set to `PERFORMED_ACTION` we call `acquireTrigger`
on the waitset which will return an `cxx::expected`. The following parameters
have to be provided.
 
 1. The origin of the trigger, e.g. `this`
 2. A method of which can be called by the trigger to ask if it was triggered.
 3. A method which resets the trigger. Used when the WaitSet goes out of scope. 
 4. The id of the trigger.
 5. A callback with the signature `void (MyTriggerClass * )`.
 
```cpp
    iox::cxx::expected<iox::popo::WaitSetError>
    attachToWaitset(iox::popo::WaitSet& waitset,
                    const MyTriggerClassEvents event,
                    const uint64_t triggerId,
                    const iox::popo::Trigger::Callback<MyTriggerClass> callback) noexcept
    {
        switch (event)
        {
        case MyTriggerClassEvents::PERFORMED_ACTION:
        {
            return waitset
                .acquireTrigger(this,
                                {this, &MyTriggerClass::hasPerformedAction},
                                {this, &MyTriggerClass::unsetTrigger},
                                triggerId,
                                callback)
                .and_then([this](iox::popo::Trigger& trigger) { 
                    m_actionTrigger = std::move(trigger); });
        }
```

When the parameter event has the value `ACTIVATE` we use the `isActivated` method
for the trigger.
```cpp
        case MyTriggerClassEvents::ACTIVATE:
        {
            return waitset
                .acquireTrigger(this,
                                {this, &MyTriggerClass::isActivated},
                                {this, &MyTriggerClass::unsetTrigger},
                                triggerId,
                                callback)
                .and_then([this](iox::popo::Trigger& trigger) { 
                    m_activateTrigger = std::move(trigger); });
        }
```

The next thing on our checklist is the `unsetTrigger` method used by the WaitSet
to reset the _Trigger_ when it goes out of scope. Therefore we look up the
correct trigger first by calling `isLogicalEqualTo` and then `reset` it.
```cpp
    void unsetTrigger(const iox::popo::Trigger& trigger)
    {
        if (trigger.isLogicalEqualTo(m_actionTrigger))
        {
            m_actionTrigger.reset();
        }
        else if (trigger.isLogicalEqualTo(m_activateTrigger))
        {
            m_activateTrigger.reset();
        }
    }
```

#### Using MyTriggerClass

The next thing we define is a free function, our `eventLoop`, which will handle
all events of our waitset. The action is for every trigger the same, resetting
the `MyTriggerClass` instance and call the callback which is attached to the
trigger.
```cpp
void eventLoop()
{
    while (true)
    {
        auto triggerStateVector = waitset->wait();
        for (auto& triggerState : triggerStateVector)
        {
            if (triggerState.getTriggerId() == ACTIVATE_ID)
            {
                triggerState.getOrigin<MyTriggerClass>()->reset();
                triggerState();
            }
            else if (triggerState.getTriggerId() == ACTION_ID)
            {
                triggerState.getOrigin<MyTriggerClass>()->reset();
                triggerState();
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
    triggerClass->attachToWaitset(*waitset, MyTriggerClassEvents::ACTIVATE, ACTIVATE_ID, callOnActivate);
    triggerClass->attachToWaitset(
        *waitset, MyTriggerClassEvents::PERFORMED_ACTION, ACTION_ID, MyTriggerClass::callOnAction);
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
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->activate(activationCode++);
            std::this_thread::sleep_for(std::chrono::seconds(1));
            triggerClass->performAction();
        }
    });
```
