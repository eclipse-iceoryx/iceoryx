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

- [Condition Variable](https://en.cppreference.com/w/cpp/thread/condition_variable)
  Used by an attached object to inform the Listener/WaitSet that an event has
  occurred.
- **event** is changing the state of an object.
- **event driven** a one time reaction which is caused directly by an event.
      Example: a new sample has been delivered to a subscriber.
- **state** predefined values to which the members of an object are set.
- **state driven** a repeating reaction which is continued as long as the state
      persists.
      Example: a subscriber has stored samples which were not inspected by the user.

## Design

The Listener is a variation of the
[reactor pattern](https://en.wikipedia.org/wiki/Reactor_pattern) and
the usage should be similar to the _WaitSet_ with a key difference - it should
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
  - Usually defined with an enum by the developer. One example is `SubscriberEvent::DATA_RECEIVED`.
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
                                        | 1                | 1
                                        |                  |
                                        | 1                | n
+------------------------------------------------------+ +--------------------------------------------------+
| ConditionListener                                    | | ConditionNotifier                                |
|   ConditionListener(ConditionVariableData & )        | |   ConditionNotifier(ConditionVariableData &,     |
|                                                      | |                 uint64_t notificationIndex)      |
|   bool                  wasNotified()                | |                                                  |
|   void                  destroy()                    | |   void notify()                                  |
|   NotificationVector_t  wait()                       | |                                                  |
|   NotificationVector_t  timedWait()                  | |   - m_condVarDataPtr    : ConditionVariableData* |
|                                                      | |   - m_notificationIndex                          |
|   - m_condVarDataPtr : ConditionVariableData*        | +--------------------------------------------------+
|   - m_toBeDestroyed  : iox::concurrent::Atomic<bool> |   | 1
+------------------------------------------------------+   |
        | 1                                                | n
        |                                           +--------------------------------+
        | 1                                         | TriggerHandle                  |
+-------------------------------------------------+ |   bool isValid()               |
| Listener                                        | |   bool wasTriggered()          |
|   attachEvent(Triggerable, EventType, Callback) | |   void trigger()               |
|   detachEvent(Triggerable, EventType)           | |   void reset()                 |
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
| |                            |                  | +-------------------------------------------------------+
| |   - m_origin               |                  | | Triggerable (e.g. Subscriber)                         |
| |   - m_callback             |                  | |                                                       |
| |   - m_invalidationCallback |                  | |   void invalidateTrigger(const uint64_t triggerId)    |
| |   - m_eventId              |                  | |   void enableEvent(TriggerHandle&&, const EventEnum ) |
| +----------------------------+                  | |   void enableEvent(TriggerHandle&&)                   |
+-------------------------------------------------+ |   void disableEvent(const EventEnum)                  |
                                                    |   void disableEvent()                                 |
                                                    |                                                       |
                                                    |   - m_triggerHandle : TriggerHandle                   |
                                                    +-------------------------------------------------------+
```

The Triggerable does not need to implement all `enableEvent`,
`disableEvent` variations, only the ones which are required by the use
case. The `enableEvent` and `disableEvent`, without the distincting `EventEnum`,
can be used when there is only a single event which can be triggered.

#### Class Interactions

- **Creating Listener:** a `ConditionVariableData` is created in the shared memory
    The `Listener` uses the `ConditionListener` to wait for incoming events.

```
                                        PoshRuntime
Listener                                    |
  |   getMiddlewareConditionVariable : var  |
  | --------------------------------------> |
  |   ConditionListener(var)                |             ConditionListener
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
User                Listener                                            Triggerable
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
    Trigger inside the Triggerable via the `invalidationCallback`

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
- **Solution:** The dependency inversion principle, create an abstraction which
    is known by both, the TriggerHandle. Created by the Listener/WaitSet and
    attached to the Triggerable so that it can notify the Listener/WaitSet via
    the underlying `ConditionNotifier` with `TriggerHandle::notify()`.
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

##### Concurrency

The Listener must be able to attach, detach events concurrently. Additionally,
it supports that a callback can attach or detach further events or detach its
corresponding event concurrently. Furthermore, the Listener supports that a
callback is called concurrently while events are being attached/detached.

To realize this we created the `Event_t` abstraction which is stored in an
array called `m_events`. If we would like to attach or detach an event we either
initialize `Event_t::init()` or reset `Event_t::reset()` the corresponding entry
in the `m_events` array. The array has the advantage that the data structure
itself never changes during runtime therefore it does not have to be thread-safe.

The thread-safety must then be ensured by the `Event_t` class itself. Since
every concurrent action is contained in `Event_t` we can use
`concurrent::smart_lock` in combination with a `std::recursive_mutex` to
guarantee the thread-safe access.

1. Concurrent attach/detach event and callback execution, ensured by
   thread-safe `Event_t`.
2. Detaching itself from within a callback is ensured via the
   `std::recursive_mutex`.
3. Attaching/detaching arbitrary events from within a callback is ensured by
   securing every `Event_t` object in `m_events` with a `concurrent::smart_lock`.
   This would not be possible if the data structure itself had to be thread-safe.

##### Lifetime

Since the event contains everything which is required to handle events it has
the responsibility to ensure the life-time of the TriggerHandle. This is done
by the `m_invalidationCallback` which is called in `Event_t::reset()` to
invalidate the TriggerHandle in the corresponding Triggerable. This is either
done when an event is detached or the Listener goes out of scope.

#### Triggerable (e.g. Subscriber)

The Triggerable is a set of classes of which events can be attached to the Listener.

It is possible to either attach a specific event of a class to the Listener or
attach the class without providing an event.

The basic idea is that the Listener creates a TriggerHandle whenever an event
is attached and provides that TriggerHandle to the corresponding Triggerable.
The Triggerable uses then the TriggerHandle to notify the Listener about events.

##### Triggerable With Single Event

Every Triggerable requires:

1. The private methods:

```cpp
void enableEvent(iox::popo::TriggerHandle&& triggerHandle) noexcept;
void disableEvent() noexcept;
void invalidateTrigger(const uint64_t uniqueTriggerId) noexcept;
```

##### Triggerable With Multiple Events

Every Triggerable requires:

1. An `enum class` which uses `iox::popo::EventEnumIdentifier` as underlying type.

```cpp
enum class EventEnum : iox::popo::EventEnumIdentifier {
  EVENT_IDENTIFIER,
  ANOTHER_EVENT_IDENTIFIER,
};
```

2. The private methods:

```cpp
void enableEvent(iox::popo::TriggerHandle&& triggerHandle, const EventEnum event) noexcept;
void disableEvent(const EventEnum event) noexcept;
void invalidateTrigger(const uint64_t uniqueTriggerId) noexcept;
```

The methods above are used by the Listener to transfer the ownership of the TriggerHandle
to the Triggerable. The Triggerable should have one TriggerHandle member for every
attachable event/state.
The TriggerHandle is then used to notify the Listener by the Triggerable that
a certain event has occurred.

3. It must be friend with `iox::popo::NotificationAttorney`. It is possible to
   provide public access to the previous methods but then the user has the ability
   to call methods which should only used by the Listener.


