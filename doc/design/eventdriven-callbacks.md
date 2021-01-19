## Name proposals

 - ReactAl = React And Listen
 - Reactor
 - Eventler


## Usage
The usage should be similar to the _WaitSet_ with a key difference - it should 
be **event driven** and not a mixture of event and state driven, depending on
which event is attached, like in the _WaitSet_.

It should have the following behavior:

 - Whenever an event occurs the corresponding callback should be called **once**
    immediately.
 - If an event occurs multiple times before the callback was called, the callback
    should be called **once**.
 - If an event occurs while the callback is executed the callback should be 
    called again **once**.

The following use cases and behaviors should be implemented.

 - The API and ReactAl should be robust this means that it should be impossible 
    to use the API in the wrong way. Since ReactAl is working concurrently a 
    wrong usage could lead to Race Conditions, extremely hard to debug bug reports 
    (HeisenBugs) and can be frustrating to the user.
    We list here features marked with [robust] which are only supported to 
    increase this kind of the robustness.

 - concurrently: attaching a callback at any time from anywhere. This means in particular
    - Attaching a callback from within a callback.
    - [robust] Updating a callback from within the same callback 
      ```cpp
      void sampleReceived2(iox::popo::UntypedSubscriber & subscriber) {}

      void sampleReceived(iox::popo::UntypedSubscriber & subscriber) {
        myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, sampleReceived2);
      }

      myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, sampleReceived);
      ```
 - concurrently: detach a callback at any time from anywhere. This means in particular
    - Detaching a callback from within a callback.
    - [robust] When the detach call returns we guarantee that the callback is never called
      again. E.g. blocks till the callback is removed, if the callback is concurrently 
      running it blocks until the callback is finished and removed.
      Exception: If a callback detaches itself it blocks until the callback is removed 
                 and not until the callback is finished.
    - Calling detach means that after that call the callback is no longer attached
      even when it was not attached in the first place. Therefore it will always succeed.

 - When the ReactAl goes out of scope it should detach itself from every class 
     to which it was attached via a callback.

 - When the class which is attached to the ReactAl goes out of scope it should 
    detach itself from the ReactAl

```cpp
ReactAl myCallbackReactal;
iox::popo::UntypedSubscriber mySubscriber;

void sampleReceived(iox::popo::UntypedSubscriber & subscriber) {
  subscriber.take().and_then([&](auto & sample){
    std::cout << "received " << std::hex << sample->payload() << std::endl;
  });
}

void waitForSubscribtion(iox::popo::UntypedSubscriber & subscriber) {
  std::cout << "subscribed!\n";
  myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED, sampleReceived);
  myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::UNSUBSCRIBED, waitForSubscribtion);
  myCallbackReactal.detachEvent(subscriber, iox::popo::SubscriberEvent::SUBSCRIBED);
}

void waitForUnsubscribe(iox::popo::UntypedSubscriber & subscriber) {
  std::cout << "unsubscribed from publisher\n";
  myCallbackReactal.attachEvent(subscriber, iox::popo::SubscriberEvent::SUBSCRIBED, waitForSubscribtion);
  myCallbackReactal.detachEvent(subscriber, iox::popo::SubscriberEvent::HAS_SAMPLE_RECEIVED);
  myCallbackReactal.detachEvent(subscriber, iox::popo::SubscriberEvent::UNSUBSCRIBED);
}

int main() {
  myCallbackReactal.attachEvent(mySubscriber, iox::popo::SubscriberEvent::SUBSCRIBED, waitForSubscribtion);
}

```

## Overall changes
### Condition Variable

  - add member `std::atomic_bool m_triggeredBy[MAX_CALLBACKS];`

    **Reason:**
      - The reactal will iterate through this array to know who was triggering it 
      - It is cache local so very fast to iterate.
      - WaitSet approach are callbacks which have some overhead implementation 
        and performance wise.

    **Alternative:**
      - Create a new class or provide a template bool array size argument to the 
        ConditionVariable to be more flexible and memory efficient.

    **Note:**
      - Sadly, we cannot pursue this approach in the WaitSet since it is event 
        and state based at the same time and this approach supports only event 
        driven triggering.

  - add class `ConditionVariableIdSignaler`
```cpp
class ConditionVariableIdSignaler {
  public:
    // id = corresponds to the id in the ConditionVariable atomic_bool array to 
    // identify the trigger origin easily
    void notifyOne(const uint64_t id); 
};
```

### ReactAl 
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

### Class which is attachable to ReactAl 

```cpp
class SomeSubscriber {
  //...
  // index is the index to the bool inside of the condition variable, maybe it 
  // is later hidden inside of some abstraction
  //
  // OutOfScopeCallback_t - callback which should be called when SomeSubscriber
  //                        goes out of scope and has to deregister itself from 
  //                        the reactal
  void attachCallback(SubscriberEvent event, ConditionVariable & cv, ConditionVariable::index_t & index
                      const OutOfScopeCallback_t & callback);

  void detachCallback(SubscriberEvent event);
  //...
};
```

The `ReactAl` will call the methods above to attach or detach a condition 
variable to a class.
