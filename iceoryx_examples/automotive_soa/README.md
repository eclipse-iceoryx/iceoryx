# automotive_soa

## Introduction

!!! attention
    The example has similarities with `ara::com` but is not a production-ready binding.

This example gives a brief overview on how to use iceoryx in automotive [SOA](https://en.wikipedia.org/wiki/Service-oriented_architecture)
frameworks like the [AUTOSAR Adaptive](https://www.autosar.org/standards/adaptive-platform/)
[`ara::com`](https://www.autosar.org//fileadmin/user_upload/standards/adaptive/20-11/AUTOSAR_EXP_ARAComAPI.pdf) binding.
If a complete `ara::com` binding is needed, please [contact the AUTOSAR foundation](https://www.autosar.org/how-to-join/) and become a member.

The example shows three different ways of communication between a skeleton and a proxy application:

1. Publish/ subscribe communication with events
1. Accessing a value with fields
1. Calling a remote method

## Expected Output

<!-- [![asciicast](https://asciinema.org/a/000000.svg)](https://asciinema.org/a/000000) -->

## Code walkthrough

The following sections discuss the different classes in detail:

* `MinimalSkeleton` and `MinimalProxy`, typically generated
* `Runtime`
* `EventPublisher` and `EventSubscriber`, transfering arbitrary types
* `FieldPublisher` and `FieldSubscriber`, transfering arbitrary types, which always have a value
and can be changed from subscriber side
* `MethodServer` and `MethodClient`, calling methods from the client on the server

### Skeleton `main()`

The skeleton application uses the `MinimalSkeleton`. Consequently, the header is included

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_skeleton.cpp] [include skeleton] -->
```cpp
#include "minimal_skeleton.hpp"
```

After both `Runtime`

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_skeleton.cpp] [create runtime] -->
```cpp
Runtime::GetInstance(APP_NAME);
```

and `MinimalSkeleton` are created on the stack. The skeleton is offered

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_skeleton.cpp] [create skeleton] -->
```cpp
kom::InstanceIdentifier instanceIdentifier{iox::cxx::TruncateToCapacity, "Example"};
MinimalSkeleton skeleton{instanceIdentifier};

skeleton.OfferService();
```

Every second an event with a counter and timestamp is send to the proxy application

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_skeleton.cpp] [send event] -->
```cpp
auto sample = skeleton.m_event.Allocate();
(*sample).counter = counter;
(*sample).sendTimestamp = std::chrono::steady_clock::now();
skeleton.m_event.Send(std::move(sample));
```

The counter is incremented with every iteration of the loop.
After 30 iterations the skeleton starts to `Update` the value of the field

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_skeleton.cpp] [send event] -->
```cpp
auto sample = skeleton.m_event.Allocate();
(*sample).counter = counter;
(*sample).sendTimestamp = std::chrono::steady_clock::now();
skeleton.m_event.Send(std::move(sample));
```

### Proxy `main()`

* Discovery phase, two kinds
  * Synchronous `FindService` call
  * If Skeleton app was not startet yet, asynchronous find service call is set up
  * Once `MinimalSkeleton` instance becomes available a callback will be executed, which will create the `MinimalProxy` object
* The main() is then able to run through its routine
  * Field is received every second
  * Remote-procedure call `computeSum()` is executed every second
  * Receiving the data on the event is done in an asynchronous manner by setting up a receiver handler, which is callted instantly once data has arrived

### Runtime

The `Runtime` is implemented as singleton, meaning each applications holds only one instance.
It is responsible for:

* Initalizing the `runtime::PoshRuntime` which registers with RouDi and sets up shared memory
(see [overview article](overview.md))
* Searching for instances of services

`popo::Listener` and `runtime::ServiceDiscovery` are used to provide service discovery
functionality. Two kind of ways to search are available, sychronous and asynchronous.

#### Sychronous search for instances

The `Runtime::FindService` call searches for all iceoryx service with the given service and
instance identifier both for publish/subscribe and request/response. The `MessagingPattern`
is supplied as the fifth parameter. All results get pushed into a shared container.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/runtime.cpp] [searching for iceoryx services] -->
```cpp
kom::ServiceHandleContainer<kom::ProxyHandleType> iceoryxServiceContainer;

m_discovery.findService(
    serviceIdentifier,
    instanceIdentifier,
    iox::cxx::nullopt,
    [&](auto& service) {
        iceoryxServiceContainer.push_back({service.getEventIDString(), service.getInstanceIDString()});
    },
    iox::popo::MessagingPattern::PUB_SUB);

m_discovery.findService(
    serviceIdentifier,
    instanceIdentifier,
    iox::cxx::nullopt,
    [&](auto& service) {
        iceoryxServiceContainer.push_back({service.getEventIDString(), service.getInstanceIDString()});
    },
    iox::popo::MessagingPattern::REQ_RES);
```

The AUTOSAR Adaptive service model is different than the iceoryx one. Hence, as the next step it
needs to be determined whether a service can be considered as complete. Typically, someone writing
a binding would query a database with e.g. the AUTOSAR meta model here. If the service is complete
the result in form of a container is passed to the caller.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/runtime.cpp] [verify iceoryx mapping] -->
```cpp
kom::ServiceHandleContainer<kom::ProxyHandleType> autosarServiceContainer;
if (verifyThatServiceIsComplete(iceoryxServiceContainer))
{
    autosarServiceContainer.push_back({serviceIdentifier, instanceIdentifier});
}

return autosarServiceContainer;
```

#### Asynchronous search for instances

The `Runtime::StartFindService` works asynchronous using `popo::Listener` to wakeup if the
`ServiceRegistry` changed and execute a user-defined callback if the availability of one of the
registered services has changed.

When calling `Runtime::StartFindService` for the first time the `ServiceDiscovery` object is
attached to the `popo::Listener` and will notified on any change of the `ServiceRegistry`.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/runtime.cpp] [attach discovery to listener] -->
```cpp
if (m_callbacks.size() == 1)
{
    auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
    m_listener.attachEvent(m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
        .or_else([](auto) {
            std::cerr << "unable to attach discovery" << std::endl;
            std::exit(EXIT_FAILURE);
        });
}
```

When any service is offered or not offered anymore `Runtime::invokeCallback` is called.

First, it needs to be assessed whether the availability of one of the registered services has changed.
For this a `FindService` call is executed and the size of the containers whilst the last and the
current call are acquired.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/runtime.cpp] [perform FindService] -->
```cpp
for (auto& callback : self->m_callbacks)
{
    auto container =
        self->FindService(std::get<1>(callback).m_serviceIdentifier, std::get<1>(callback).m_instanceIdentifier);

    auto numberOfAvailableServicesOnCurrentSearch = container.size();
    auto& numberOfAvailableServicesOnLastSearch = std::get<2>(callback);
```

Two cases have to be handled, the special one on the first apperance of the service

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/runtime.cpp] [first execution conditions] -->
```cpp
if (!numberOfAvailableServicesOnLastSearch.has_value() && numberOfAvailableServicesOnCurrentSearch != 0)
{
    executeCallback();
}
```

and the one when the service disappears after having been available

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/runtime.cpp] [second execution conditions] -->
```cpp
if (numberOfAvailableServicesOnLastSearch.has_value()
    && numberOfAvailableServicesOnLastSearch.value() != numberOfAvailableServicesOnCurrentSearch)
{
    executeCallback();
}
```

### Minimal skeleton

* Contains three communication patterns
  * Event
  * Field
  * Method

* Unlike in AUTOSAR, in iceoryx a service is represented by the individual `Publisher` or `Server`
* AUTOSAR service is considered as available as soon as all members are available

#### `EventPublisher`

* Topics with different sizes are available
* Also available as UNIX domain socket implementation to compare to iceoryx's shared memory
implementation

#### `FieldPublisher`

* Field
  * Sends inital value on creation
  * After 30 loop iteration the value is updated

#### `MethodServer`

* Method
  * Works in the background, in its own thread
  * Receives requests, computes answer and sends back response

### Minimal proxy

* Contains the respective counterparts to be able to consume the
`MinimalSkeleton` service with all its three elements aka iceoryx services

#### `EventSubscriber`

#### `FieldSubscriber`

#### `MethodClient`

<center>
[Check out automotive_soa on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/automotive_soa){ .md-button } <!--NOLINT github url for website-->
</center>
