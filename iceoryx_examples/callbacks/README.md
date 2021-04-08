# Listener (or how to use callbacks with iceoryx)

## Thread Safety
The Listener is thread-safe and can be used without restrictions.
But be aware that all provided callbacks are executed concurrently 
in the background thread of the Listener. If you access structures
inside this callback you have to either ensure that you are the only
one accessing it or that it is accessed with a guard like a `std::mutex`.

## Introduction

For an introduction into the terminology please read the Glossary in the
[WaitSet C++ example](../waitset).

The Listener is a completely thread-safe construct which reacts to events by 
executing registered callbacks in a background thread. Events can be emitted by 
_EventOrigins_ like a subscriber or a user trigger. Some of the _EventOrigins_ 
like the subscriber can hereby emit more than one event type.

The interface of a listener consists of two methods: `attachEvent` to attach a 
new event with a callback and `detachEvent`. These two methods can be called 
concurrently, even from inside a callback which was triggered by an event!

## Expected Output

<!-- @todo Add expected output with asciinema recording before v1.0-->

## Code Walkthrough

!!! attention 
    Please be aware about the thread-safety restrictions of the _Listener_ and 
    read the [Thread Safety](#thread-safety) chapter carefully.

Let's say we have an application which offers us two distinct services:
`Radar.FrontLeft.Counter` and `Rader.FrontRight.Counter`. Every time we have 
received a sample from left and right we would like to calculate the sum with 
the newest values and print it out. If we have received only one of the samples 
we store it until we received the other side.

### ice_callbacks_publisher.cpp

The publisher of this example does not contain any new features but if you have 
some questions take a look at the [icedelivery example](../icedelivery).

### ice_callbacks_subscriber.cpp
#### int main()
The subscriber main function starts as usual and after registering the runtime 
we create the listener which starts a background thread.
```cpp
iox::popo::Listener listener;
```

Because it is fun we also create a heartbeat trigger which will be triggered 
every 4 seconds so that `heartbeat received` can be printed to the console.
Furthermore, we have to create two subscriber to receive samples for the two 
services.
```cpp
iox::popo::UserTrigger heartbeat;
iox::popo::Subscriber<CounterTopic> subscriberLeft({"Radar", "FrontLeft", "Counter"});
iox::popo::Subscriber<CounterTopic> subscriberRight({"Radar", "FrontRight", "Counter"});
```

Next thing is a `heartbeatThread` which will trigger our heartbeat trigger every 
4 seconds.
```cpp
std::thread heartbeatThread([&] {
    while (keepRunning)
    {
        heartbeat.trigger();
        std::this_thread::sleep_for(std::chrono::seconds(4));
    }
});
```

Now that everything is setup we can attach the subscribers to the listener so that
everytime a new sample (`iox::popo::SubscriberEvent::DATA_RECEIVED`) is received our callback 
(`onSampleReceivedCallback`) will be called. We also attach 
our `heartbeat` user trigger to print the hearbeat message to the console via another
callback (`heartbeatCallback`).
```cpp
listener.attachEvent(heartbeat, heartbeatCallback).or_else([](auto) {
    std::cerr << "unable to attach heartbeat event" << std::endl;
    std::terminate();
});
listener.attachEvent(subscriberLeft, iox::popo::SubscriberEvent::DATA_RECEIVED, onSampleReceivedCallback)
    .or_else([](auto) {
        std::cerr << "unable to attach subscriberLeft" << std::endl;
        std::terminate();
    });
// it is possible to attach any callback here with the required signature. to simplify the
// example we attach the same callback onSampleReceivedCallback again
listener.attachEvent(subscriberRight, iox::popo::SubscriberEvent::DATA_RECEIVED, onSampleReceivedCallback)
    .or_else([](auto) {
        std::cerr << "unable to attach subscriberRight" << std::endl;
        std::terminate();
    });
```
Since a user trigger has only one event we do not have to specify an event when we attach 
it to the listener. `attachEvent` returns a `cxx::expected` to inform us if the attachment
succeeded. When this is not the case the error handling is performed in the `.or_else([](auto){` part 
after each `attachEvent` call.
In this example we choose to attach the same callback twice to make things easier 
but you are free to attach any callback with the signature `void(iox::popo::Subscriber<CounterTopic> *)`.

The setup is complete but it would terminate right away since we have no blocker which
waits until SIGINT or SIGTERM was send. In the other examples we hadn't have that problem
since we pulled all the events in a while true loop but working only with callbacks 
requires something like our `shutdownSemaphore`, a semaphore on which we wait until 
the signal callback increments it.
```cpp
shutdownSemaphore.wait();
```

When the `shutdownSemaphore` unblocks we clean up all resources and terminate the process 
gracefully.
```cpp
listener.detachEvent(heartbeat);
listener.detachEvent(subscriberLeft, iox::popo::SubscriberEvent::DATA_RECEIVED);
listener.detachEvent(subscriberRight, iox::popo::SubscriberEvent::DATA_RECEIVED);

heartbeatThread.join();
```

Hint: You do not have to detach an _EventOrigin_ like a subscriber or user trigger 
before it goes out of scope. This also goes for the _Listener_, the implemented
RAII based design takes care of the resource cleanup.

#### The Callbacks
The callbacks must have a signature like `void(PointerToEventOrigin*)`.
Our `heartbeatCallback` for instance just prints the message `heartbeat received`.
```cpp
void heartbeatCallback(iox::popo::UserTrigger*)
{
    std::cout << "heartbeat received " << std::endl;
}
```

The `onSampleReceivedCallback` is more complex. We first acquire the received 
sample and check which subscriber signaled the event by acquiring the subscriber's
service description. If the instance is equal to `FrontLeft` we store the sample
in the `leftCache` otherwise in the `rightCache`.
```cpp
void onSampleReceivedCallback(iox::popo::Subscriber<CounterTopic>* subscriber)
{
    subscriber->take().and_then([subscriber](auto& sample) {
        auto instanceString = subscriber->getServiceDescription().getInstanceIDString();

        // store the sample in the corresponding cache
        if (instanceString == iox::capro::IdString_t("FrontLeft"))
        {
            leftCache.emplace(*sample);
        }
        else if (instanceString == iox::capro::IdString_t("FrontRight"))
        {
            rightCache.emplace(*sample);
        }
```

In a next step we check if both caches are filled. If this is the case we print 
an extra message which states the result of the sum of both received values.
Afterwards we reset both caches to start fresh again.
```cpp
    if (leftCache && rightCache)
    {
        std::cout << "Received samples from FrontLeft and FrontRight. Sum of " << leftCache->counter << " + "
                  << rightCache->counter << " = " << leftCache->counter + rightCache->counter << std::endl;
        leftCache.reset();
        rightCache.reset();
    }
```
