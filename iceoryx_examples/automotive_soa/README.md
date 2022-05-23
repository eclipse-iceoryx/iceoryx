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

!!! note
    The example should be built with the 1:n communication option (`ONE_TO_MANY_ONLY`).

The following sections discuss the different classes in detail:

* `MinimalSkeleton` and `MinimalProxy`
    * Typically generated from a meta model
* `Runtime`
* `EventPublisher` and `EventSubscriber`
    * Transfering arbitrary types
* `FieldPublisher` and `FieldSubscriber`
    * Transfering arbitrary types, which always have a value and can be changed from subscriber side
* `MethodServer` and `MethodClient`
    * Calling methods from the client on the server

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
if (!sample)
{
    std::exit(EXIT_FAILURE);
}
sample->counter = counter;
sample->sendTimestamp = std::chrono::steady_clock::now();
skeleton.m_event.Send(std::move(sample));
```

The counter is incremented with every iteration of the loop.
After 30 iterations the skeleton starts to `Update` the value of the field

<!-- [geoffrey] [iceoryx_examples/automotive_soa/iox_automotive_skeleton.cpp] [send event] -->
```cpp
auto sample = skeleton.m_event.Allocate();
if (!sample)
{
    std::exit(EXIT_FAILURE);
}
sample->counter = counter;
sample->sendTimestamp = std::chrono::steady_clock::now();
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
needs to be determined whether a service can be considered as complete. Unlike in AUTOSAR, in
iceoryx a service is represented by the individual `Publisher` or `Server`. The `MinimalSkeleton`
service is considered as available as soon as all members are available. Typically, someone
writing a binding would query a database with e.g. the AUTOSAR meta model here. If the service is
complete the result in form of a container is passed to the caller.

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
        .expect("Unable to attach discovery!");
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

!!! info
    `EventPublisher` and `EventSubscriber` are also available as UNIX domain socket implementation
    to compare iceoryx's shared memory implementation. Available topic sizes are:
    1 Byte, 4 kB, 16 kB, 64 kB, 256 kB, 1 MB, 4 MB

#### `EventPublisher`

A `EventPublisher` contains a single member.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/event_publisher.hpp] [EventPublisher members] -->
```cpp
iox::popo::Publisher<T> m_publisher;
```

Its API provides a `Send()` method which performes a copy and a zero-copy one where the user needs
to call `Allocate()` before and acquire a piece of memory in the shared memory.

The onwership to the piece of memeory is represented by a `SampleAllocateePtr`. It behaves like a
`std::unique_ptr`.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/event_publisher.hpp] [EventPublisher allocate] -->
```cpp
SampleAllocateePtr<SampleType> Allocate() noexcept;
```

Afterwards data can be written directly to shared memory by dereferencing the `SampleAllocateePtr`
object. It is implemented using `cxx::optional` and `popo::Sample` and, in line with the iceoryx
philosophy for defined behaviour, terminates if an empty object is dereferenced.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/sample_allocatee_ptr.inl] [SampleAllocateePtr dereferencing] -->
```cpp
template <typename SampleType>
inline SampleType& SampleAllocateePtr<SampleType>::operator*() noexcept
{
    if (!this->has_value())
    {
        // We don't allow undefined behaviour
        std::cerr << "Trying to access an empty sample, terminating!" << std::endl;
        std::terminate();
    }
    return *(this->value().get());
}
```

Finially, the memory can be send to subscribers by calling

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/event_publisher.hpp] [EventPublisher zero-copy send] -->
```cpp
void Send(SampleAllocateePtr<SampleType> userSamplePtr) noexcept;
```

#### `FieldPublisher`

A `FieldPublisher` contains four members.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_publisher.hpp] [FieldPublisher members] -->
```cpp
iox::popo::Publisher<FieldType> m_publisher;
iox::popo::Server<iox::cxx::optional<FieldType>, FieldType> m_server;
iox::popo::Listener m_listener;
// latestValue is written concurrently by Listener and needs exclusive write access, alternatively a
// concurrent::smart_lock could be used
std::atomic<T> m_latestValue;
```

The `Publisher` is used for event-like communication. The `Server` for enabling the user to set
the field value from subscriber side. The `Listener` is used to instantly react on new requests
being send to the `Server`. Additionally, the latest value is stored if the users queries the
value again.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_publisher.hpp] [FieldPublisher members] -->
```cpp
iox::popo::Publisher<FieldType> m_publisher;
iox::popo::Server<iox::cxx::optional<FieldType>, FieldType> m_server;
iox::popo::Listener m_listener;
// latestValue is written concurrently by Listener and needs exclusive write access, alternatively a
// concurrent::smart_lock could be used
std::atomic<T> m_latestValue;
```

`Update()` is very similar to `Send()` in the `EventPublisher`. The `Server` needs to be attached
in the constructor

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_publisher.inl] [FieldPublisher attach] -->
```cpp
m_listener
    .attachEvent(m_server,
                 iox::popo::ServerEvent::REQUEST_RECEIVED,
                 iox::popo::createNotificationCallback(onRequestReceived, *this))
    .expect("Unable to attach server!");
```

Once a new request is received the following callback will be called. It receives the request,
reserves new memory for the response and writes the current value to shared memory.

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_publisher.inl] [FieldPublisher callback] -->
```cpp
template <typename T>
inline void FieldPublisher<T>::onRequestReceived(iox::popo::Server<iox::cxx::optional<FieldType>, FieldType>* server,
                                                 FieldPublisher<FieldType>* self) noexcept
{
    if (self == nullptr)
    {
        std::cerr << "Callback was invoked with FieldPublisher* being a nullptr!" << std::endl;
        return;
    }

    while (server->take().and_then([&](const auto& request) {
        server->loan(request)
            .and_then([&](auto& response) {
                if (request->has_value())
                {
                    self->m_latestValue = request->value();
                }
                *response = self->m_latestValue;
                response.send().or_else(
                    [&](auto& error) { std::cerr << "Could not send response! Error: " << error << std::endl; });
            })
            .or_else([](auto& error) { std::cerr << "Could not allocate response! Error: " << error << std::endl; });
    }))
    {
    }
}
```

#### `MethodServer`

The `MethodServer` is similar to the request-response part of the `FieldPublisher`.

It is implemented with two members

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/method_server.hpp] [MethodServer members] -->
```cpp
iox::popo::Server<AddRequest, AddResponse> m_server;
iox::popo::Listener m_listener;
```

The attachment of the `Server` to the `Listener` and the callback are very similar to the `FieldPublisher`. However, as `MethodServer` task is to add two numbers the response is calculated by calling

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/owl/kom/method_server.cpp] [MethodServer calc response] -->
```cpp
.and_then([&](auto& response) {
    response->sum = self->computeSumInternal(request->addend1, request->addend2);
```

### Minimal proxy

`MinimalProxy` contains the respective counterparts to be able to consume the `MinimalSkeleton`
service with all its three elements:

* `EventSubscriber m_event`
* `FieldSubscriber m_field`
* `MethodClient computeSum`

#### `EventSubscriber`

The `EventSubscriber` class is implemented with the following members

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/event_subscriber.hpp] [EventSubscriber members] -->
```cpp
iox::popo::Subscriber<T> m_subscriber;
iox::cxx::optional<iox::cxx::function<void()>> m_receiveHandler;
static constexpr bool IS_RECURSIVE{true};
iox::posix::mutex m_mutex{IS_RECURSIVE};
iox::popo::Listener m_listener;
```

Again, a `Listener` is used to instantly react on a new topic send by a `EventPublisher`.
This time, the `Listener` executes the callback which was stored with

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/event_subscriber.inl] [EventSubscriber setReceiveHandler] -->
```cpp
template <typename T>
inline void EventSubscriber<T, EventTransmission::IOX>::SetReceiveHandler(EventReceiveHandler handler) noexcept
{
    if (!handler)
    {
        std::cerr << "Can't attach empty receive handler!" << std::endl;
        return;
    }
    m_listener
        .attachEvent(m_subscriber,
                     iox::popo::SubscriberEvent::DATA_RECEIVED,
                     iox::popo::createNotificationCallback(onSampleReceivedCallback, *this))
        .expect("Unable to attach subscriber!");
    std::lock_guard<iox::posix::mutex> guard(m_mutex);
    m_receiveHandler.emplace(handler);
}
```

and once invoked the callback gets executed

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/event_subscriber.inl] [EventSubscriber invoke callback] -->
```cpp
template <typename T>
inline void EventSubscriber<T, EventTransmission::IOX>::onSampleReceivedCallback(iox::popo::Subscriber<T>*,
                                                                                 EventSubscriber* self) noexcept
{
    if (self == nullptr)
    {
        std::cerr << "Callback was invoked with EventSubscriber* being a nullptr!" << std::endl;
        return;
    }

    std::lock_guard<iox::posix::mutex> guard(self->m_mutex);
    self->m_receiveHandler.and_then([](iox::cxx::function<void()>& userCallable) {
        if (!userCallable)
        {
            std::cerr << "Tried to call an empty receive handler!" << std::endl;
            return;
        }
        userCallable();
    });
}
```

The mutex is used to provide thread-safety and protect `m_receiveHandler` because it is accessed
concurrently.

If a receive handler is not needed `GetNewSamples()` can be used in a polling manner.

#### `FieldSubscriber`

The `FieldSubscriber` class is implemented with help of the following members

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_subscriber.hpp] [FieldSubscriber members] -->
```cpp
iox::popo::Subscriber<FieldType> m_subscriber;
iox::popo::Client<iox::cxx::optional<FieldType>, FieldType> m_client;
std::atomic<int64_t> m_sequenceId{0};
iox::popo::WaitSet<> m_waitset;
static constexpr bool IS_RECURSIVE{true};
iox::posix::mutex m_mutex{IS_RECURSIVE};
std::atomic<uint32_t> m_threadsRunning{0};
```

The `Get()` method allows the receive the value from a `FieldPublisher`
object on demand by sending an empty request

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_subscriber.inl] [FieldSubscriber get] -->
```cpp
bool requestSuccessfullySent{false};
m_client.loan()
    .and_then([&](auto& request) {
        request.getRequestHeader().setSequenceId(m_sequenceId);
        request.send().and_then([&]() { requestSuccessfullySent = true; }).or_else([](auto& error) {
            std::cerr << "Could not send Request! Error: " << error << std::endl;
        });
    })
    .or_else([&](auto& error) {
        std::cerr << "Could not allocate Request! Error: " << error << std::endl;
        requestSuccessfullySent = false;
    });

if (!requestSuccessfullySent)
{
    return Future<FieldType>();
}
return receiveResponse();
```

A sequence number is set to ensure monotonic order.

The `Set()` method is implemented in a similar manner but writes the value to the request

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_subscriber.inl] [FieldSubscriber set] -->
```cpp
.and_then([&](auto& request) {
    request.getRequestHeader().setSequenceId(m_sequenceId);
    request->emplace(value);
```

Both methods call `receiveResponse()` at the very end, which will receive the response `Future` by using the `WaitSet`

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/field_subscriber.inl] [FieldSubscriber receive response] -->
```cpp
std::lock_guard<iox::posix::mutex> guard(m_mutex);

auto notificationVector = m_waitset.timedWait(iox::units::Duration::fromSeconds(5));

if (notificationVector.empty())
{
    std::cerr << "WaitSet ran into timeout when trying to receive response!" << std::endl;
}

for (auto& notification : notificationVector)
{
    if (notification->doesOriginateFrom(&m_client))
    {
        while (m_client.take().and_then([&](const auto& response) {
            auto receivedSequenceId = response.getResponseHeader().getSequenceId();
            if (receivedSequenceId == m_sequenceId)
            {
                FieldType result = *response;
                m_sequenceId++;
                promise.set_value_at_thread_exit(result);
            }
            else
            {
                std::cerr << "Got Response with outdated sequence ID! Expected = " << m_sequenceId
                          << "; Actual = " << receivedSequenceId << "!" << std::endl;
                std::terminate();
            }
        }))
        {
        }
    }
}
m_threadsRunning--;
```

Here, the expected sequence identifier is compared with the received one. In the end, the response
is copied into the `std::promise`.

Alternatively, the value can also be received in a polling manner with `GetNewSamples()`.

#### `MethodClient`

Every `MethodClient` object contains the following members

<!-- [geoffrey] [iceoryx_examples/automotive_soa/include/owl/kom/method_client.hpp] [MethodClient members] -->
```cpp
iox::popo::Client<AddRequest, AddResponse> m_client;
std::atomic<int64_t> m_sequenceId{0};
iox::popo::WaitSet<> m_waitset;
static constexpr bool IS_RECURSIVE{true};
iox::posix::mutex m_mutex{IS_RECURSIVE};
std::atomic<uint32_t> m_threadsRunning{0};
```

Receiving the result of the addition of the two numbers is similar to receiving the value in the
`FieldSubscriber`. However, as the `MethodClient` acts a functor providing an `operator()()`,
unlike the `FieldPublisher`, it is not templated but takes and fixed number of arguments.

The request part of `operator()()` is implemted as follows

<!-- [geoffrey] [iceoryx_examples/automotive_soa/src/owl/kom/method_client.cpp] [MethodClient send request] -->
```cpp
Future<AddResponse> MethodClient::operator()(uint64_t addend1, uint64_t addend2)
{
    bool requestSuccessfullySent{false};
    m_client.loan()
        .and_then([&](auto& request) {
            request.getRequestHeader().setSequenceId(m_sequenceId);
            request->addend1 = addend1;
            request->addend2 = addend2;
            request.send().and_then([&]() { requestSuccessfullySent = true; }).or_else([](auto& error) {
                std::cerr << "Could not send request! Error: " << error << std::endl;
            });
        })
        .or_else([&](auto& error) {
            std::cerr << "Could not allocate request! Error: " << error << std::endl;
            requestSuccessfullySent = false;
        });

    if (!requestSuccessfullySent)
    {
        return Future<AddResponse>();
    }
```

The response is received in a similar manner to receiving the response in the `FieldSubscriber`, again, wrapping the result in a `std::future`.

<center>
[Check out automotive_soa on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/automotive_soa){ .md-button } <!--NOLINT github url for website-->
</center>
