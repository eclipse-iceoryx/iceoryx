# Name proposals

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

The following use cases should be possible.

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

## Design


