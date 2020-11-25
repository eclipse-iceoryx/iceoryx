# WaitSet

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
     following signature `void ( TriggerOrigin )`.
 - **TriggerId** a id which identifies the trigger. It does not follow any 
     restrictions and the user can choose any arbitrary `uint64_t`.
 - **TriggerOrigin** the pointer to the class where the trigger originated from, short
     pointer to the _Triggerable_.
 - **WaitSet** a _Listener_ which manages a set of _Triggers_ which can be acquired by 
     the user. The _Waitset_ listens 
     to the whole set of _Triggers_ and if one or more _Trigger_ are triggered it will notify
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
_WaitSet_ with the event `HAS_NEW_SAMPLES`. Everytime one of the subscribers is
receiving a new sample it will trigger the _WaitSet_.
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