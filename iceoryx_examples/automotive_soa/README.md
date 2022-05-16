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

The applications runs until `Ctrl-C` is pressed.

### Proxy `main()`

Similar to the skeleton application, the `MinimalProxy` header is included

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [include proxy] -->
```cpp
#include "minimal_proxy.hpp"
```

and the runtime is created.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [create runtime] -->
```cpp
Runtime::GetInstance(APP_NAME);
```

Unlike the `MinimalSkeleton` object the `MinimalProxy` one is wrapped in both a `cxx::optional` and
a `concurrent::smart_lock` because it is destroyed concurrently when `MinimalSkeleton` becomes
unavailable.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [wrap proxy] -->
```cpp
iox::concurrent::smart_lock<optional<MinimalProxy>> maybeProxy;
```

When starting the proxy application, the discovery phase is happening. Initially, a synchronous `FindService` call is
performed.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [sychronous discovery] -->
```cpp
kom::InstanceIdentifier exampleInstanceSearchQuery(TruncateToCapacity, "Example");
std::cout << "Searching for instances of '" << MinimalProxy::m_serviceIdentifier << "' called '"
          << exampleInstanceSearchQuery.c_str() << "':" << std::endl;
auto handleContainer = MinimalProxy::FindService(exampleInstanceSearchQuery);
```

If the skeleton application was already started, the `maybeProxy` is initialized from the search result.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [create proxy] -->
```cpp
for (auto& handle : handleContainer)
{
    std::cout << "  Found instance of service: '" << MinimalProxy::m_serviceIdentifier << "', '"
              << handle.GetInstanceId().c_str() << "'" << std::endl;
    maybeProxy->emplace(handle);
}
```

If the skeleton application was not started yet and the search result is empty, an asynchronous
find service call is set up.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [set up asynchronous search] -->
```cpp
auto handle = MinimalProxy::StartFindService(callback, exampleInstanceSearchQuery);
```

Once the `MinimalSkeleton` instance becomes available, the callback will be executed, which will create the
`MinimalProxy` object

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [create proxy asynchronously] -->
```cpp
for (auto& proxyHandle : container)
{
    if (!maybeProxy->has_value())
    {
        std::cout << "  Found instance of service: '" << MinimalProxy::m_serviceIdentifier << "', '"
                  << proxyHandle.GetInstanceId().c_str() << "'" << std::endl;
        maybeProxy->emplace(proxyHandle);
    }
}
```

and respectively destroy the `MinimalProxy` when the once available service becomes unavailable.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [destroy proxy asynchronously] -->
```cpp
if (container.empty())
{
    std::cout << "  Instance '" << maybeProxy->value().m_instanceIdentifier.c_str() << "' of service '"
              << MinimalProxy::m_serviceIdentifier << "' has disappeared." << std::endl;
    maybeProxy->value().m_event.UnsetReceiveHandler();
    maybeProxy->reset();
    return;
}
```

After the discovery phase, the applications continues with the runtime phase receiving data and
performing remote method calls on the `MinimalSkeleton`. For the `computeSum()` method call two
integers are created.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [Field: create ints for computeSum] -->
```cpp
uint64_t addend1{0};
uint64_t addend2{0};
```

As different threads work concurrently on `maybeProxy`, first of all exclusive access is gained.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [gain exclusive access to proxy] -->
```cpp
auto proxyGuard = maybeProxy.getScopeGuard();
if (proxyGuard->has_value())
{
    auto& proxy = proxyGuard->value();
```

For the event communication an `onReceive` callback is set up

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [Event: set receiveHandler] -->
```cpp
if (!proxy.m_event.HasReceiveHandler())
{
    proxy.m_event.Subscribe(10U);
    proxy.m_event.SetReceiveHandler(onReceive);
}
```

which, receives the data and prints out both the `counter` and the latency with which the topic
was received. The call happens instantly after data was sent on the event by `MinimalSkeleton`.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [Event: receiveHandler callback] -->
```cpp
auto onReceive = [&]() -> void {
    proxy.m_event.GetNewSamples([](const auto& topic) {
        auto finish = std::chrono::steady_clock::now();
        std::cout << "Event: value is " << topic->counter << std::endl;
        auto duration =
            std::chrono::duration_cast<std::chrono::nanoseconds>(finish - topic->sendTimestamp);
        std::cout << "Event: latency (ns) is " << duration.count() << std::endl;
    });
};
```

The data of the field is received as a `std::future`, which can throw exceptions. Consequently,
a `try`-`catch` block is used to handle potential errors. Initially, the value of the field
is `4242` and during the first thirty iterations the incremented value is `Set()` from the
`MinimalProxy` side. Afterwards the value is updated from the `MinimalSkeleton` side.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [Field: get data] -->
```cpp
auto fieldFuture = proxy.m_field.Get();
try
{
    auto result = fieldFuture.get();
    std::cout << "Field: value is " << result.counter << std::endl;

    if (result.counter >= 4242)
    {
        result.counter++;
        proxy.m_field.Set(result);
        std::cout << "Field: value set to " << result.counter << std::endl;
    }
}
catch (const std::future_error&)
{
    std::cout << "Empty future from field received, please start the 'iox-cpp-automotive-skeleton'."
              << std::endl;
}
```

Calling the mehtod `computeSum()` on the `MinimalProxy` can throw exceptions, too. Hence again,
a `try`-`catch` block is used for error handling. The `addend`s are provided as parameters and
changed after each iteration.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [Method: call computeSum remotely] -->
```cpp
auto methodFuture = proxy.computeSum(addend1, addend2);
try
{
    auto result = methodFuture.get();
    std::cout << "Method: result of " << std::to_string(addend1) << " + " << std::to_string(addend2)
              << " = " << result.sum << std::endl;
}
catch (const std::future_error&)
{
    std::cout << "Empty future from method received, please start the 'iox-cpp-automotive-skeleton'."
              << std::endl;
}
```

Again, the applications runs until `Ctrl-C` is pressed.

If an asynchronous find service call was set up, the call is stopped with the return value of
`StartFindService` before termination.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_proxy.cpp] [stop find service] -->
```cpp
if (maybeHandle.has_value())
{
    MinimalProxy::StopFindService(maybeHandle.value());
}
```

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

Contains three members which use a different communication pattern:

* `EventPublisher m_event`
* `FieldPublisher m_field`
* `MethodServer computeSum`

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
