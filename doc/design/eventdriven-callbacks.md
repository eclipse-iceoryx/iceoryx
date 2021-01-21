## Problem Description

We require a public API so that a developer can register callbacks for certain
events like the arrival of a sample.

## Name of the design element

 - ReactAl = React And Listen
 - Reactor
 - Eventler
 - LiAR - Listen And React


## Terminology

 - **event driven** a one time reaction which is caused directly by an event.
      Example: a new sample has been delivered to a subscriber.
 - **state driven** a repeating reaction which is caused by a specific structural 
      state until the state changes.
      Example: a subscriber has stored samples which were not inspected by the user.
 - **robust API** we use this term to describe an API which is hard 
      to misuse. Especially important in concurrent code since a misuse can lead to race 
      conditions, Heisenbugs and in general hard to debug problems.
 - **robust API feature** a feature we only support to increase the robustness 
      of the API and bugs caused by misuse.
 - (Reactor pattern)[https://en.wikipedia.org/wiki/Reactor_pattern]
 - (Condition Variable)[https://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables_2]
 
## Design

### General
The usage should be similar to the _WaitSet_ with a key difference - it should 
be **event driven** and not a mixture of event and state driven, depending on
which event is attached, like in the _WaitSet_.

It should have the following behavior:

 - Whenever an event occurs the corresponding callback should be called **once**
    immediately.
 - If an event occurs multiple times before the callback was called, the callback
    should be called **once**.
 - If an event occurs while the callback is being executed the callback should be 
    called again **once**.

The following use cases and behaviors should be implemented.

 - The API and ReactAl should be robust this means that it should be very hard 
    to use the API in the wrong way. Since ReactAl is working concurrently a 
    wrong usage could lead to race conditions, extremely hard to debug bug reports 
    (HeisenBugs) and can be frustrating to the user.
    We list here features marked with [robust] which are only supported to 
    increase this kind of the robustness.

 - concurrently: attaching a callback at any time from anywhere. This means in particular
    - Attaching a callback from within a callback.
    - [robust] Updating a callback from within the same callback 
      ```cpp
      void onSampleReceived2(iox::popo::UntypedSubscriber & subscriber) {}

      void onSampleReceived(iox::popo::UntypedSubscriber & subscriber) {
        myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceived2);
      }

      myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceived);
      ```
 - One can attach at most one callback to a specific event of an object.
    - Attaching a callback to an event where a callback has been already attached overrides
      the existing callback with the new one.
 - concurrently: detach a callback at any time from anywhere. This means in particular
    - Detaching a callback from within a callback.
    - [robust] When the detach call returns we guarantee that the callback is never called
      again. E.g. blocks till the callback is removed, if the callback is concurrently 
      running it will block until the callback is finished and removed.
      Exception: If a callback detaches itself it blocks until the callback is removed 
                 and not until the callback is finished.
    - Calling detach means that after that call the callback is no longer attached
      even when it was not attached in the first place. Therefore it will always succeed.

 - When the ReactAl goes out of scope it should detach itself from every class 
     to which it was attached via a provided callback (like in the WaitSet).

 - When the class which is attached to the ReactAl goes out of scope it should 
    detach itself from the ReactAl via a provided callback (like in the WaitSet).

### Usage Code Example
```cpp
ReactAl myCallbackReactal;
iox::popo::UntypedSubscriber mySubscriber;

void onSampleReceived(iox::popo::UntypedSubscriber & subscriber) {
  subscriber.take().and_then([&](auto & sample){
    std::cout << "received " << std::hex << sample->payload() << std::endl;
  });
}

void onWaitForSubscription(iox::popo::UntypedSubscriber & subscriber) {
  std::cout << "subscribed!\n";
  myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceived);
  myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::UNSUBSCRIBED, onWaitForSubscription);
  myCallbackReactal.detachEvent(subscriber, iox::popo::SubscriberEvent::SUBSCRIBED);
}

void onWaitForUnsubscribe(iox::popo::UntypedSubscriber & subscriber) {
  std::cout << "unsubscribed from publisher\n";
  myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::SUBSCRIBED, onWaitForSubscription);
  myCallbackReactal.detachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED);
  myCallbackReactal.detachEvent(subscriber, iox::popo::SubscriberEvent::UNSUBSCRIBED);
}

int main() {
  myCallbackReactal.attachEvent(mySubscriber, iox::popo::SubscriberEvent::SUBSCRIBED, onWaitForSubscription);
}
```

### Solution
#### Condition Variable

  - Alternative names: `EventVariable`, `EventSignaler`, `EventWaiter`
  
  - **Problem:** The ReactAl would like to know by whom it was triggered. The WaitSet has a 
                  big list of callbacks where it can ask the object if it triggered the WaitSet. This has 
                  certain disadvantages.
              - Cache misses and low performance 
              - The object could lie due to a bug.
              - Implementation overhead since we have to manage callbacks.

  - **Solution:** The condition variable tracks by whom it was triggered. To realize this every signaler
      gets provided with an id. When the condition variable is notified a corresponding bool array entry is set to true 
      to trace the trigger origin.

  - Create from `ConditionVariableWithOriginTracing` (yeah we need a better name) has `ConditionVariableData` 
    as a member and adds: `std::atomic_bool m_triggeredBy[MAX_CALLBACKS];`

    **Reason:**
      - The reactal will iterate through this array to find out by which event it was triggered 
      - It is cache local, since it is one piece of contiguous memory which can be put directly 
        into the CPU cache when it is being iterated. Therefore it is very performant.
      - WaitSet approach are callbacks which have an overhead implementation 
        and performance wise.
        * implementation, since we have to manage set and reset all the callbacks 
        * performance, when we call a callback with every iteration, we have to first load the 
            callback into the cpu cache (generates cache misses for all the other callback 
            elements in the vector which is being iterated). In the callback a method in an 
            object is called (the object has to be loaded again into the cpu cache, cache miss for 
            every member which is not used by the method).

    **Potential optimization:**
      - Create an additional bool member `m_wasTriggered` which is true when at least 
        one bool in `m_triggerdBy` is true. This would spare us from iterating over the array 
        when it was not triggered at all.

    **Adjustments:**
      - Create the class with a template bool array size argument to be more flexible and memory efficient.

    **Note:**
      - Sadly, we cannot pursue this approach in the WaitSet since it is event 
        and state based at the same time and this approach supports only event 
        driven triggering.

  - add class `ConditionVariableWithOriginTracingSignaler` (I know, better name)
  
    **Reason:** it behaves quiete differently then the `ConditionVariable`. The `ConditionVariable` tells you:
    someone has signalled you and you have to find out who it was. The `ConditionVariableWithOriginTracing`
    would tell you: A, B and C have signalled you.

    ```cpp
    class ConditionVariableWithOriginTracingSignaler {
      public:
        // originId = corresponds to the id in the ConditionVariable atomic_bool array to 
        // identify the trigger origin easily
        void notify(const uint64_t originId); 

        // alternative: origin is set in the ctor and we just call notify
        ConditionVariableWithOriginTracingSignaler(const uint64_t originId);
        void notify();
    };
    ```

  - add class `ConditionVariableWithOriginTracingWaiter`
  ```cpp
  class ConditionVariableWithOriginTracingWaiter {
    public:
      // returns a list of ids which have notified the conditionVariable
      cxx::vector<uint64_t> wait(); 
      // sets the atomic bool to false in m_triggeredBy[id]
      void resetWaitState(const uint64_t id) noexcept; 
  };
  ```

#### ReactAl 
```cpp
enum class ReactAlErrors {
  CALLBACK_CAPACITY_EXCEEDED
};

template<uint64_t CallbackCapacity>
class ReactAl {
  public:
    template<typename EventOrigin, typename EventType>
    cxx::expected<ReactAlErrors> attachEvent(
      EventOrigin & origin, const EventType & eventType,
      const cxx::function_ref<void(EventOrigin&)> & callback
    ) noexcept;

    template<typename EventOrigin, typename EventType>
    void detachEvent( EventOrigin & origin, const EventType & eventType );
};

```
 
  - Contains a thread which will wake up and iterate through ConditionVariable
    bool array. If an entry is true, set it to false and then call the 
    corresponding callback.

#### Class which is attachable to ReactAl 

```cpp
class SomeSubscriber {
  //...
  // index is the index to the bool inside of the condition variable, maybe it 
  // is later hidden inside of some abstraction
  //
  // OutOfScopeCallback_t - callback which should be called when SomeSubscriber
  //                        goes out of scope and has to deregister itself from 
  //                        the reactal
  friend class ReactAl;
  private:
    void attachCallback(SubscriberEvent event, ConditionVariable & cv, ConditionVariable::index_t & index
                        const OutOfScopeCallback_t & callback);

    void detachCallback(SubscriberEvent event);
  //...
};
```

The `ReactAl` will call the methods above to attach or detach a condition 
variable to a class. The methods should be private and the `ReactAl` must then 
be declared as a friend.

## Open Points

 - Maybe introduce some abstraction Condition Variable tracing handling. 
    One thought is to introduce a `SignalVector` where you can acquire a 
    `Notifyier` which then signals the `ConditionVariable` and with the 
    correct id.


