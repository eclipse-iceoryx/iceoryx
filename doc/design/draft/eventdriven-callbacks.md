## Problem Description

We require a public API so that a developer can register callbacks for certain
events like the arrival of a sample. These callbacks should be executed in a 
background thread.

## Name of the design element

 - ReactAL = React And Listen
 - Eventler
 - LiAR - Listen And React
 - Wait And React -> WAR
 - Listen and Execute -> LEx
 - Robust Event Awaiting Concurrent Taken On Run -> REACTOR
 - event listening for execution -> ELFE


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
 - [Reactor pattern](https://en.wikipedia.org/wiki/Reactor_pattern)
 - [Condition Variable](https://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables_2)
 - [Heisenbug](https://en.wikipedia.org/wiki/Heisenbug)
 
## Design

### General
The usage should be similar to the _WaitSet_ with a key difference - it should 
be **event driven** and not a mixture of event and state driven, depending on
which event is attached, like in the _WaitSet_.

It should have the following behavior:

 - Whenever an event occurs the corresponding callback should be called **once**
    as soon as possible.
 - If an event occurs multiple times before the callback was called, the callback
    should be called **once**.
 - If an event occurs while the callback is being executed the callback should be 
    called again **once**.

The following use cases and behaviors should be implemented.

 - The API and ReactAL should be robust. Since ReactAL is working concurrently a 
    wrong usage could lead to race conditions, extremely hard to debug bug reports
    (Heisenbugs) and can be frustrating to the user.
    We list here features marked with [robust] which are only supported to 
    increase this kind of the robustness.

 - concurrently: attaching a callback at any time from anywhere. This means in particular
    - Attaching a callback from within a callback.
    - [robust] Updating a callback from within the same callback 
      ```cpp
      void onSampleReceived2(iox::popo::UntypedSubscriber & subscriber) {}

      void onSampleReceived(iox::popo::UntypedSubscriber & subscriber) {
        myCallbackReactAL.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceived2);
      }

      myCallbackReactAL.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceived);
      ```
 - One can attach at most one callback to a specific event of a specific object. The event is 
    - Usually defined with an enum by the developer. One example is `SubscriberEvent::HAS_SAMPLE_RECEIVED`.
    - Attaching a callback to an event where a callback has been already attached overrides
      the existing callback with the new one.
    - One can attach the same event to different objects at the same time.
      ```cpp
      myReactAL.attachEvent(subscriber1, SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceive);

      // overrides first callback
      myReactAL.attachEvent(subscriber1, SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceive2); 

      // callback is being added to the ReactAL since it belonging to a different object
      myReactAL.attachEvent(subscriber2, SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceive); 
      ```
 - concurrently: detach a callback at any time from anywhere. This means in particular
    - Detaching a callback from within a callback.
    - [robust] When the detach call returns we guarantee that the callback is never called
      again. E.g. blocks till the callback is removed, if the callback is concurrently 
      running it will block until the callback is finished and removed.
      Exception: If a callback detaches itself it blocks until the callback is removed 
                 and not until the callback is finished. After the successful removal the 
                 callback will continue and will not be called again.
    - The callback is no longer attached to the event after calling `detach`.
      The method will succeed even if it was not attached in the first place.

 - When the ReactAL goes out of scope it detaches itself from every class 
     to which it was attached via a callback provided by the attached class (like in the WaitSet).

 - When the class which is attached to the ReactAL goes out of scope it 
    detaches itself from the ReactAL via a callback provided by the ReactAL (like in the WaitSet).

### Usage Code Example
```cpp
ReactAL myCallbackReactAL;
iox::popo::UntypedSubscriber mySubscriber;

void onSampleReceived(iox::popo::UntypedSubscriber & subscriber) {
  subscriber.take().and_then([&](auto & sample){
    std::cout << "received " << std::hex << sample->payload() << std::endl;
  });
}

void onSubscribe(iox::popo::UntypedSubscriber & subscriber) {
  std::cout << "subscribed!\n";
  myCallbackReactAL.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, onSampleReceived);
  myCallbackReactAL.attachEvent(subscriber, iox::popo::SubscriberEvent::UNSUBSCRIBED, onUnsubscribe);
  myCallbackReactAL.detachEvent(subscriber, iox::popo::SubscriberEvent::SUBSCRIBED);
}

void onUnsubscribe(iox::popo::UntypedSubscriber & subscriber) {
  std::cout << "unsubscribed from publisher\n";
  myCallbackReactAL.attachEvent(subscriber, iox::popo::SubscriberEvent::SUBSCRIBED, onSubscribe);
  myCallbackReactAL.detachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED);
  myCallbackReactAL.detachEvent(subscriber, iox::popo::SubscriberEvent::UNSUBSCRIBED);
}

int main() {
  iox::popo::SubscriberOptions subscriberOptions;
  subscriberOptions.queueCapacity = 10U;
  iox::popo::TypedSubscriber<RadarObject> mySubscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);

  myCallbackReactAL.attachEvent(mySubscriber, iox::popo::SubscriberEvent::SUBSCRIBED, onSubscribe);
  mySubscriber.subscribe();

  App::mainloop();
}
```

### Solution
#### Class diagram

```
+-----------------------+                          +----------------------------------+
| ConditionVariableData |                          | EventVariableData                |
|   - m_semaphore       | <----------------------- |   - m_activeNotifications : bool |
|   - m_process         |                          +----------------------------------+
|   - m_toBeDestroyed   |                        / 1            | 1
+-----------------------+                       /               |
                                               / 1              | 1
+-----------------------------------------------+ +----------------------------------------------+
| EventListener                                 | | EventNotifier                                |
|   EventListener(EventVariableData & dataPtr)  | |   EventNotifier(EventVariableData & dataPtr, |
|                                               | |                 uint64_t notificationIndex)  |
|   vector<bool> wait()                         | |   void notify()                              |
|   vector<bool> timedWait()                    | |                                              |
|                                               | |   - m_notificationIndex                      |
|   - m_pointerToEventVariableData              | |   - m_pointerToEventVariableData             |
+-----------------------------------------------+ +----------------------------------------------+
        | 1                                                     | m
        |                                                       |
        | 1                                                     | 1  [one EventNotifier per Event]
+--------------------------------------------------+  +-----------------------------------+
| ReactAL                                          |  | EventAssignable (e.g. Subscriber) |
|   attachEvent(EventOrigin, EventType, Callback)  |  +-----------------------------------+
|   detachEvent(EventOrigin, EventType)            |
|                                                  |
|   - m_callbacks                                  |
+--------------------------------------------------+
```

#### Class Interactions

  - **Creating ReactAL:** an `EventVariableData` is created in the shared memory. 
      The `ReactAL` uses the `EventListener` to wait for incoming events.
```
                                      PoshRuntime
  ReactAL                                 |
    |   getMiddlewareEventVariable : var  |
    | ----------------------------------> |
    |   EventListener(var)                |             EventListener
    | ------------------------------------+-----------------> |
    |   wait() : vector<uint64_t>         |                   |
    | ------------------------------------+-----------------> |
```
  - **Attaching Subscriber Event (sampleReceived) to ReactAL:** The subscriber attaches the `EventNotifier`
      appropriately and signals the `ReactAL` via the `EventNotifier` about the occurrence 
      of the event (sampleReceived).
```
  User                ReactAL                                             Subscriber 
   |   attachEvent()     |                                                     |
   | ------------------> |   EventNotifier(EventVarDatPtr)    EventNotifier    |
                         | --------------------------------------> |           |
                         |                                         |           |
                         |   enableEvent(EventNotifier, ...)       |           |
                         | ----------------------------------------+---------> |
```
  - **Signal an event from subscriber:** `EventNotifier.notify()` is called which sets an, to the subscriber 
      attached flag, to true and `EventListener` will wake up.
```
  Subscriber          EventNotifier                EventVariableData.m_activeNotifications
    |   notify()           |                                            |
    | -------------------> |   operator[](m_notificationIndex) = true   |
    |                      | -----------------------------------------> |
```
  - **ReactAL reacting to events:** `EventListener.wait()` will provide a complete list of all `EventNotifier`
      which were notifying the `EventListener` till the last `wait()` call. The ReactAL uses this list 
      to call the corresponding callbacks.
```cpp
  auto activeCallbacks = m_eventListener.wait();
  for(auto index : activeCallbacks)
     m_callbacks[index]();
```

#### Event Variable

  In this section we discuss the details of the three classes `EventVariableData`, `EventListener` and 
  `EventNotifier` from the class diagram above. The `EventListener` and `EventNotifier` are sharing their 
  data `EventVariableData` and the whole construct should be named `EventVariable`.

  - **Problem:** The ReactAL would like to know by whom it was triggered. The WaitSet has a 
                  list of callbacks where it can ask the object if it triggered the WaitSet. This has 
                  certain disadvantages.
              - Cache misses and low performance 
              - The object could lie due to a bug.
              - Implementation overhead since we have to manage callbacks.

  - **Solution:** The condition variable tracks by whom it was triggered. To realize this every signaler
      gets provided with an id. When the condition variable is notified a corresponding bool array entry is set to true 
      to trace the trigger origin.

  - `EventVariableData` inherits from `ConditionVariableData` and adds the member `std::atomic_bool m_triggeredBy[MAX_CALLBACKS];`

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
        - Reserve the last/first element in the bool array for `m_wasTriggered`
          to may gain even more cache efficiency.

    **Adjustments:**
      - Create the class with a template bool array size argument to be more flexible and memory efficient.

    **Note:**
      - Sadly, we cannot pursue this approach in the WaitSet since it is event 
        and state based at the same time and this approach supports only event 
        driven triggering.

  - add class `EventNotifier`

    **Reason:** it behaves quite differently then the `ConditionVariable`. The `ConditionVariable` notifies the user
    that it was signalled and the user has to find the origin manually. The `EventVariable`
    would notify the user by which origin it was signalled.

    ```cpp
    class EventNotifier {
      public:
        // identify the trigger origin easily
        void notify(); 

        // alternative: origin is set in the ctor and we just call notify
        ConditionVariableWithOriginTracingSignaler(const uint64_t originId);
        void notify();

      private:
        // originId = corresponds to the id in the ConditionVariable atomic_bool array to 
        m_originId;
    };
    ```

  - add class `EventListener`
    ```cpp
    class EventListener {
      public:
        // returns a list of ids which have notified the conditionVariable
        cxx::vector<uint64_t> wait(); 
        // sets the atomic bool to false in m_triggeredBy[id]
        void resetWaitState(const uint64_t id) noexcept; 
    };
    ```

#### ReactAL 
```cpp
enum class ReactALErrors {
  CALLBACK_CAPACITY_EXCEEDED
};

template<uint64_t CallbackCapacity>
class ReactAL {
  public:
    template<typename EventOrigin, typename EventType>
    cxx::expected<ReactALErrors> attachEvent(
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

#### Class which is attachable to ReactAL 

```cpp
class SomeSubscriber {
  //...
  // index is the index to the bool inside of the condition variable, maybe it 
  // is later hidden inside of some abstraction
  //
  // OutOfScopeCallback_t - callback which should be called when SomeSubscriber
  //                        goes out of scope and has to deregister itself from 
  //                        the reactal
  friend class ReactAL;
  private:
    void enableEvent(SubscriberEvent event, ConditionVariable & cv, ConditionVariable::index_t & index
                        const OutOfScopeCallback_t & callback);

    void disableEvent(SubscriberEvent event);
  //...
};
```

The `ReactAL` will call the methods above to enable or disable a specific event by 
providing a condition variable. The methods should be private and the `ReactAL` must then 
be declared as a friend.

## Open Points

 - Maybe introduce some abstraction Condition Variable tracing handling. 
    One thought is to introduce a `SignalVector` where you can acquire a 
    `Notifier` which then signals the `ConditionVariable` and with the 
    correct id.


