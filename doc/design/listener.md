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

- **Creating Listener:** an `ConditionVariableData` is created in the shared memory
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
 |                     |   create   |                                        |
 |                     | ---------> |                                        |
 |                     |        enableEvent(std::move(TriggerHandle))        |
 |                     | -----------+--------------------------------------> |
```

- **Signal an event from Triggerable:** `TriggerHandle::trigger()` is called, the
    Listener is returning from the `ConditionListener.wait()` call and retrieves
    a list of all the signal notifications. The corresponding event-callbacks
    are called.

```
Triggerable     TriggerHandle  ConditionNotifier    ConditionListener                Listener                     Event_t
     |    trigger()   |                |                    |   wait() : notificationIds |                           |
     | -------------> |    notify()    |                    | <------------------------- |                           |
     |                | -------------> | .... unblocks .... |            blocks          |  exeuteCallback()         |
     |                |                |                    |                            | ------------------------> |
     |                |                |                    |                            |  m_events[notificationId] |
```

- **Triggerable goes out of scope:** The TriggerHandle is a member of the 
    Triggerable, therefore the d'tor of the TriggerHandle is called which removes 
    the trigger from the Listener via the `resetCallback`

```
Triggerable        TriggerHandle         Listener         Event_t
     |  ~TriggerHandle   |                   |               |
     | ----------------> |  removeTrigger()  |               |
     |                   | ----------------> |    reset()    |
     |                   | via resetCallback | ------------> |
```

- **Listener goes out of scope:** The d'tor of the `Event_t` invalidates the 
    Trigger inside the Triggarable via the `invalidationCallback`

```
Listener       Event_t                  Triggerable
   |  ~Event_t()  |                          |
   | -----------> |    invalidateTrigger()   |
   |              | -----------------------> |
   |              | via invalidationCallback |
```

#### TriggerHandle

- **Problem:** The Triggerable should be able to notify a Listener/WaitSet
    without having any knowledge about those class so that circular
    dependencies can be prevented. Furthermore, the Triggerable must be able
    to remove its attached events when it goes out of scope.
- **Solution:** The TriggerHandle which contains the `ConditionNotifier` to
    notify the Listener/WaitSet.
    The cleanup task is performed by the `m_resetCallback` so that the
    Triggerable has no dependencies to any Notifyable.

#### Condition Variable

The `ConditionListener` and `ConditionNotifier` are two different interfaces to
the same class which state is stored in the `ConditionVariableData` class. The
intention of the separation is to provide one side (e.g. Triggerable) only
an API to notify the Notifyable (e.g. Listener) whereas the Notifyable can only
wait on events. So the contract is reflected in the design.

- **Problem:** Since the Listener reacts on events and not states it requires
    the knowledge by whom it was notified.
- **Solution:**
  - Every TriggerHandle has a unique id which is used as index in `ConditionNotifier`.
  - When `ConditionNotifier::notify` is called the Listener is informed via the
    `NotificationVector_t` return value from `ConditionListener::wait()` which
    index notified him. Hence it is the same as the unique id of the TriggerHandle
    the Listener knows which Triggerable notified him.

#### Event_t
- thread safe - smart_lock 
- cleanup
- callback

#### Listener
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


