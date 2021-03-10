# Listener 

For an introduction into the terminology please read the Glossary in the
[WaitSet C++ example](../waitset).

The Listener is a completely threadsafe construct which reacts to events by 
executing registered callbacks in a background thread. Events can be emitted by 
_EventOrigins_ like a subscriber or a user trigger. Some of the _EventOrigins_ 
like the subscriber can hereby emit more then one event type.

The interface of a listener consists of two methods `attachEvent` to attach a 
new event with a callback and `detachEvent`. These two methods can be called 
concurrently, even from inside a callback which was triggered by an event!

## Example 

Let's say we have an application which offers us two distinct services:
`Radar.FrontLeft.Counter` and `Rader.FrontRight.Counter`. Everytime we have 
received a sample from left and right we would like to calculate the sum with 
the newest values and print it out. If we have received only one of the samples 
we store it until we received the other side.

The publisher of this example does not contain any new features but if you have 
some questions take a look at the [icedelivery example](../icedelivery).

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
everytime a new sample (`iox::popo::SubscriberEvent::HAS_DATA`) is received our callback 
(`onSampleReceivedCallback`) will be called. We also attach 
our `heartbeat` user trigger to print the hearbeat message to the console via another
callback (`heartbeatCallback`).
```cpp
listener.attachEvent(heartbeat, heartbeatCallback);
listener.attachEvent(subscriberLeft, 
      iox::popo::SubscriberEvent::HAS_DATA, onSampleReceivedCallback);
listener.attachEvent(subscriberRight, 
      iox::popo::SubscriberEvent::HAS_DATA, onSampleReceivedCallback);
```
Since a user trigger has only one event we do not have to specify an event when we attach it to the listener.
