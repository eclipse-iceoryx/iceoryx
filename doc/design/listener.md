# Listener

## Summary

The Listener is besides the WaitSet one of the building blocks to realize a
push approach to detect and react to certain events. The Listener
offers the user the ability to attach objects with a corresponding event and
callback. Whenever the object receives the specified event the corresponding
callback is called in the Listener background thread as a reaction.

The two key differences to the WaitSet are that the Listener is event-driven
and not event- and state-driven as the WaitSet and that the Listener creates
a separate background thread in which the event-callbacks are executed in contrast
to the WaitSet where the user has to call the event-callbacks explicitly.

## Terminology

- [Condition Variable](https://en.wikipedia.org/wiki/Monitor_(synchronization)#Condition_variables_2)
  Used by an attached object to inform the Listener/WaitSet that an event has
  occurred.
- **event** is changing the state of an object.
- **event driven** a one time reaction which is caused directly by an event.
      Example: a new sample has been delivered to a subscriber.
- [Heisenbug](https://en.wikipedia.org/wiki/Heisenbug)
- [Reactor pattern](https://en.wikipedia.org/wiki/Reactor_pattern)
  The Listener and WaitSet are both variations of the that pattern.
- **state** predefined values to which the members of an object are set.
- **state driven** a repeating reaction which is continued as long as the state
      persists.
      Example: a subscriber has stored samples which were not inspected by the user.

## Design

The usage should be similar to the _WaitSet_ with a key difference - it should
be **event driven** and not a mixture of event and state driven, depending on
which event is attached, like in the _WaitSet_.

### Requirements

- Whenever an event occurs the corresponding callback should be called **once**
    as soon as possible.
- If an event occurs multiple times before the callback was called, the callback
    should be called **once**.
- If an event occurs while the callback is being executed the callback should be
    called again **once**.
- Thread-safety: attaching an event at any time from any thread.
- Thread-safety: detaching an event at any time from any thread.
  - If the callback is currently running `detachEvent` blocks until the callback
      is finished.
  - After the `detachEvent` call the event-callback is not called anymore, even
      when the event was signalled while `detachEvent` was running and the
      callback was not yet executed.
  - If the callback is detached from within the event-callback then `detachEvent`
     is non blocking. The event is detached right after the `detachEvent` call.
- One can attach at most one callback to a specific event of a specific object.
  - Usually defined with an enum by the developer. One example is `SubscriberEvent::HAS_SAMPLE_RECEIVED`.
  - Attaching a callback to an event where a callback has been already attached
      results in an error.
- One can attach the same event to different objects at the same time.
- One can attach multiple different events to a single object
- When the Listener goes out of scope it detaches itself from every attached
    object via a callback provided by the attached object (like in the WaitSet).
- When the class which is attached to the Listener goes out of scope it
    detaches itself from the Listener via a callback provided by the Listener
    (like in the WaitSet).

### Solution

#### Class diagram

```
                                   +---------------------------+
                                   | ConditionVariableData     |
                                   |   - m_semaphore           |
                                   |   - m_runtimeName         |
                                   |   - m_toBeDestroyed       |
                                   |   - m_activeNotifications |
                                   +---------------------------+
                                        | 1               | 1
                                        |                 |
                                        | n               | n
+-----------------------------------------------+ +--------------------------------------------------+
| ConditionListener                             | | ConditionNotifier                                |
|   ConditionListener(ConditionVariableData & ) | |   ConditionNotifier(ConditionVariableData &,     |
|                                               | |                 uint64_t notificationIndex)      |
|   bool                  wasNotified()         | |                                                  |
|   void                  destroy()             | |   void notify()                                  |
|   NotificationVector_t  wait()                | |                                                  |
|   NotificationVector_t  timedWait()           | |   - m_condVarDataPtr    : ConditionVariableData* |
|                                               | |   - m_notificationIndex                          |
|   - m_condVarDataPtr : ConditionVariableData* | +--------------------------------------------------+
|   - m_toBeDestroyed  : std::atomic_bool       |        | 1
+-----------------------------------------------+        |
        | 1                                              | n
        |                                           +--------------------------------+
        | 1                                         | TriggerHandle                  |
+-------------------------------------------------+ |   bool isValid()               |
| Listener                                        | |   bool wasTriggered()          |
|   attachEvent(EventOrigin, EventType, Callback) | |   void trigger()               |
|   detachEvent(EventOrigin, EventType)           | |   void reset()                 |
|                                                 | |   void invalidate()            |
|   - m_events : Event_t[]                        | |   void getUniqueId()           |
|   - m_thread : std::thread                      | |                                |
|   - m_conditionListener : ConditionListener     | |   - m_conditionVariableDataPtr |
|                                                 | |   - m_resetCallback            |
| +----------------------------+                  | |   - m_uniqueTriggerId          |
| | Event_t                    |                  | +--------------------------------+
| |   void executeCallback()   |                  |      | 1
| |   bool reset()             |                  |      |
| |   bool init(...)           |                  |      | n
| |                            |                  | +----------------------------------------------------+
| |   - m_origin               |                  | | Triggerable (e.g. Subscriber)                      |
| |   - m_callback             |                  | |                                                    |
| |   - m_invalidationCallback |                  | |   void invalidateTrigger(const uint64_t triggerId) |
| |   - m_eventId              |                  | |   void enableEvent(TriggerHandle&&)                |
| +----------------------------+                  | |   void disableEvent(const EventEnum event)         |
+-------------------------------------------------+ |                                                    |
                                                    |   - m_triggerHandle : TriggerHandle                |
                                                    +----------------------------------------------------+
```

#### Class Interactions

  - **Creating Listener:** an `ConditionVariableData` is created in the shared memory. 
      The `Listener` uses the `ConditionListener` to wait for incoming events.
```
                                          PoshRuntime
  Listener                                    |
    |   getMiddlewareConditionVariable : var  |
    | --------------------------------------> |
    |   ConditionListener(var)                |             EventListener
    | ----------------------------------------+-----------------> |
    |   wait() : vector<uint64_t>             |                   |
    | ----------------------------------------+-----------------> |
```
  - **Attaching Triggerable Event (SubscriberEvent::DATA_RECEIVED) to Listener:**
      The Listener creates a TriggerHandle and provides it to the Triggerable (Subscriber)
      via `enableEvent` so that the Triggerable owns the handle. Whenever the event occurs
      the Triggerable can use the `trigger()` method of the TriggerHandle to notify
      the Listener.
```
  User                ReactAL                                             Triggerable 
   |   attachEvent()     |                                                     |
   | ------------------> |      TriggerHandle                                  |
                         |   create   |                                        |
                         | ---------> |                                        |
                         |        enableEvent(std::move(TriggerHandle))        |
                         | -----------+--------------------------------------> |
```
  - **Signal an event from Triggerable:** `TriggerHandle::trigger()` is called 
      and the Listener calls the corresponding callback in the background thread

  - **Triggerable goes out of scope:**

  - **Listener goes out of scope:**
  
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


