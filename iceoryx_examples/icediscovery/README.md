# icediscovery

## Introduction

This example demonstrates how to search for specific services using iceoryx's
`ServiceDiscovery`. It provides two applications - one offering different
services and one searching for these making different search queries.
A `service` in iceoryx is defined by a `ServiceDescription` that represents a
topic under which publisher/server and subscriber/client can exchange data.

In addition the applications `iox-wait-for-service` and `iox-discovery-monitor` demonstrate how
to write custom discovery functionality to wait for specific services or monitor
the availability of services respectively.

<!--
## Expected Output

/// @todo iox-#1295 re-record asciicast

[![asciicast](https://asciinema.org/a/476673.svg)](https://asciinema.org/a/476673)
-->

## Code walkthrough

### Offer services

We create several publishers which offer their services on construction by
default. For more dynamism the `cameraPublishers` offer/stop their services
periodically. If you want more information on how to create a publisher, have a
look at the [icehello](../icehello),
[icedelivery](../icedelivery),
and [iceoptions](../iceoptions)
examples.

### Find services

To be able to search for services, we have to create a `ServiceDiscovery` object:

<!--[geoffrey][iceoryx_examples/icediscovery/iox_find_service.cpp][create ServiceDiscovery object]-->
```cpp
iox::runtime::ServiceDiscovery serviceDiscovery;
```

It is included via:

<!--[geoffrey][iceoryx_examples/icediscovery/iox_find_service.cpp][include ServiceDiscovery]-->
```cpp
#include "iceoryx_posh/runtime/service_discovery.hpp"
```

On that object we can call the method `findService` which expects the three
service [string identifiers](../../doc/website/getting-started/overview.md#creating-service-descriptions-for-topics)
and a callable which will be applied to all matching services.
In addition, we have to specify whether we want to search for publishers (`MessagingPattern::PUB_SUB`)
used in publish-subscribe communication or servers (`MessagingPattern::REQ_RES`) used in
request-response communication.

In this example
we pass a function that prints the found services on the console:

<!--[geoffrey][iceoryx_examples/icediscovery/iox_find_service.cpp][print function to be applied to search results]-->
```cpp
void printSearchResult(const iox::capro::ServiceDescription& service)
{
    std::cout << "- " << service << std::endl;
}
```

We can search for exactly matching services:

<!--[geoffrey][iceoryx_examples/icediscovery/iox_find_service.cpp][search for unique service]-->
```cpp
serviceDiscovery.findService(iox::capro::IdString_t{"Radar"},
                             iox::capro::IdString_t{"FrontLeft"},
                             iox::capro::IdString_t{"Objects"},
                             printSearchResult,
                             iox::popo::MessagingPattern::PUB_SUB);
```

or add wildcards to our search query:

<!--[geoffrey][iceoryx_examples/icediscovery/iox_find_service.cpp][search for all Camera services]-->
```cpp
serviceDiscovery.findService(iox::capro::IdString_t{"Camera"},
                             iox::capro::Wildcard,
                             iox::capro::Wildcard,
                             printSearchResult,
                             iox::popo::MessagingPattern::PUB_SUB);
```

With the above `findService` call we look for every `Camera` service with any
instance and any event. Since the `cameraPublishers` periodically offer/stop
their services, you should see sometimes 5 `Camera` services and sometimes none.

### Wait for services

Start the applications `iox-wait-for-service` and `iox-offer-service`. This can be done in any order,
but for demonstration purposes `iox-offer-service` should be started last.

`iox-wait-for-service` uses a customized service discovery [Discovery](#implementation-of-discovery-with-blocking-wait)
which supports to wait for services by including

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][include custom discovery]-->
```cpp
#include "discovery_blocking.hpp"
```

We then can use our custom discovery class

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][create custom discovery]-->
```cpp
// requires the runtime to be created first
Discovery discovery;
```

which provides a function `waitUntil` to wait for some discovery-related search query condition.

We define the search query

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][define search query]-->
```cpp
auto query = [&]() {
    auto result = discovery.findService(service, instance, event);
    return !result.empty();
};
```

This is essentially any callable with `bool(void)` signature, but it should depend on the discovery somehow (by capture),
as it is only checked when the service availability changes in some way. Here, we require some specific service to be found
before we proceed.

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][service to wait for]-->
```cpp
iox::capro::IdString_t service{"Camera"};
iox::capro::IdString_t instance{"FrontLeft"};
iox::capro::IdString_t event{"Image"};
```

Now we can wait until the service discovery changes and the service becomes available.
<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][wait until service was available]-->
```cpp
bool serviceWasAvailable = discovery.waitUntil(query);
```

This wait blocks until the service is available. If it already is available we do not block and proceed.
It is important that due to the nature of concurrent systems we cannot know that the service is still available
once we return from `waitUntil`, as the application offering the service may have stopped doing so in the meantime.

Usually, we will assume that the service is available and may continue, e.g. by creating subscribers and running
application specific code.

We can also block until any unspecified change in the service availability occurs
<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][wait until discovery changes]-->
```cpp
discovery.waitUntilChange();
```

This change is relative to the last `findService` call we issued, i.e. if something changed compared to
the available services at this point, we wake up and continue.

We then can check any condition we like, but usually it will be most useful to again check discovery-related conditions.
Here, we check whether a particular service becomes unavailable (essentially the negation of our query before)
<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][check service availability]-->
```cpp
if (discovery.findService(service, instance, event).empty())
{
    break;
}
```

Note that we use a customized `findService` version which returns a result container which can easily be built
using the version which takes a function to be applied to all services in the search result.

Once the service becomes unavailable, the application exits.

Should the service we wait for never become available we can unblock any of the wait calls with

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][unblock wait]-->
```cpp
keepRunning = false;
if (discoverySigHandlerAccess)
{
    discoverySigHandlerAccess->unblockWait();
}
```

### Monitor service availability

If we want to continuously monitor the availability of some service or check some discovery condition we can do so by
using e.g. a listener to conditionally execute [callbacks](../callbacks).

To do so, we start the applications `iox-discovery-monitor` and `iox-offer-service`
(again in any order, but for demonstration purposes `iox-offer-service` should be started last).

Again, we can use a [Discovery](#implementation-of-discovery-monitoring) customized for this purpose by including

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][include custom discovery]-->
```cpp
#include "discovery_monitor.hpp"
```

and creating it like so

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][create custom discovery]-->
```cpp
// requires the runtime to be created first
Discovery discovery;
```

Afterwards, we create a callback to be called whenever the service availability changes.

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][create monitoring callback]-->
```cpp
auto callback = [&](Discovery& discovery) -> void {
    auto result = discovery.findService(service, instance, event);

    if (!result.empty())
    {
        std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> available" << std::endl;
    }
    else
    {
        std::cout << APP_NAME << " <" << service << ", " << instance << ", " << event << "> unavailable"
                  << std::endl;
    }
    printSearchResult(result);
};
```

This callback essentially checks whether a specific service is available or unavailable and generates output accordingly.
Other reactions are possible as well, such as changing the processing logic of an application.

To start the monitoring, we register the callback

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][register callback]-->
```cpp
discovery.registerCallback(callback);
```

Monitoring happens in a background thread implicitly created by the `Discovery`, i.e. the callback is executed in this thread.

When we want to stop monitoring we have to deregister the callback

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][deregister callback]-->
```cpp
discovery.deregisterCallback();
```

Here, this is done at the very end where it is technically not required, but in a more complex application it could be done
while the application is processing data. The main processing loop of the application is deliberately left empty for simplicity.
Usually it would interact with the callback by e.g. changing application behavior whenever the availability of some service changes.

While we only can attach one callback to the general event that the service availability changes in some way, we can
generalize the mechanism here to check for multiple conditions and react to each of them by e.g. calling a specific function.
These conditions would still need to be checked in the callback we defined though.

### Implementation of Discovery with blocking wait

We build our custom discovery on top of the `iox::runtime::ServiceDiscovery` by composition. While inheritance is an option,
composition has the advantage that we can use `ServiceDiscovery` as a singleton (to save memory)
but our custom `Discovery` class can be fairly lightweight and does not need to be a singleton.

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_blocking.cpp][service discovery singleton]-->
```cpp
ServiceDiscovery& serviceDiscovery()
{
    static ServiceDiscovery instance;
    return instance;
}
```

This is useful as the `ServiceDiscovery` may be fairly large and in general there is no point in having multiple
`ServiceDiscovery` objects that all have the same purpose and (if updated) the same view of the available services.

The key idea is to use a waitset and attach to the event that the service availability changes

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_blocking.cpp][attach waitset]-->
```cpp
m_waitset.attachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED)
    .or_else(errorHandler);
```

Waiting for any availability change is now as simple as waiting on the waitset

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_blocking.cpp][wait until change]-->
```cpp
void Discovery::waitUntilChange()
{
    do
    {
        auto notificationVector = m_waitset.wait();
        for (auto& notification : notificationVector)
        {
            if (notification->doesOriginateFrom(m_discovery))
            {
                return;
            }
        }
    } while (m_blocking);
}
```

If we want to wait for a specific condition, we can do so with

<!--[geoffrey][iceoryx_examples/icediscovery/include/discovery_blocking.hpp][wait until condition]-->
```cpp
template <typename Condition>
bool Discovery::waitUntil(const Condition& condition)
{
    do
    {
        // 1) does the condition hold?
        bool result = condition();
        if (result)
        {
            // 2) condition held and we return (without mutex to protect condition changes
            // there is no way to guarantee it still holds)
            return true;
        }
        else
        {
            if (!m_blocking)
            {
                return false;
            }
        }
        // 3) condition did not hold but it may hold if we use the latest discovery data
        //    which may have arrived in the meantime

        // 4) this does not wait if there is new discovery data (and hence we try again immediately)
        waitUntilChange();
        // 5) discovery data changed, check condition again (even if unblocked)
    } while (true);

    return false;
}
```

The  condition needs to be evaluable to `bool` and takes no arguments. While this can be generalized to any variadic arguments,
it is not needed as we can use capturing lambda expressions. The wait simply checks for the condition, and if true returns
immediately. Otherwise it waits until the available services change using `waitUntilChange` before checking the condition again.

It is also possible to unblock any of the waits even if nothing changes or the condition does not hold

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_blocking.cpp][unblock wait]-->
```cpp
void Discovery::unblockWait() volatile noexcept
{
    m_blocking = false;
    // could also unblock with a dedicated condition to unblock the wait but that requires more code
    // (additional trigger) and is not necessary if it is only supposed to happen once
    m_waitset.markForDestruction();
}
```

This can only be called once and makes all future wait calls non-blocking. It is useful to unblock any wait calls to be
able to stop the application.

Finally we provide a custom implementation of `findService` which returns a container of our choice, in this case a `std::vector`.

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_blocking.cpp][findService]-->
```cpp
ServiceContainer Discovery::findService(const iox::optional<iox::capro::IdString_t>& service,
                                        const iox::optional<iox::capro::IdString_t>& instance,
                                        const iox::optional<iox::capro::IdString_t>& event)
{
    ServiceContainer result;
    auto filter = [&](const iox::capro::ServiceDescription& s) { result.emplace_back(s); };
    m_discovery->findService(service, instance, event, filter, iox::popo::MessagingPattern::PUB_SUB);
    return result;
}
```

It is implemented by using the native `findService` call of the `ServiceDiscovery` with an appropriate filter function.
The benefit is that this way we can choose containers which do not necessarily reside on the stack.

### Implementation of Discovery monitoring

To implement a `Discovery` where we actively monitor availability of services we employ a
[listener](../callbacks).
Contrary to the blocking solution this does not block the user threads and executes any callback
in a background thread created by the listener.
The callback will be executed on any change of the available services.

To register the callback we call
<!--[geoffrey][iceoryx_examples/icediscovery/include/discovery_monitor.hpp][registerCallback]-->
```cpp
template <typename Callback>
void Discovery::registerCallback(const Callback& callback)
```

which attaches the callback to the listener.

<!--[geoffrey][iceoryx_examples/icediscovery/include/discovery_monitor.hpp][attach listener]-->
```cpp
auto invoker = iox::popo::createNotificationCallback(invokeCallback, *this);
m_listener.attachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED, invoker)
    .or_else(errorHandler);
```

The callback is stored as a `iox::function` which does not require dynamic memory (but limits the size of the stored function,
which is relevant e.g. for capturing lambdas). If dynamic memory is no concern we can also use a `std::function`.
The callback can be any callable with a `(void)(discovery::Discovery&)` signature.
Again the callback signature can be generalized somewhat but there are constraints to use it with the listener.
Since the listener can only call static or free functions, we use an additional indirection to call the actual callback

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_monitor.cpp][invokeCallback]-->
```cpp
void Discovery::invokeCallback(ServiceDiscovery*, Discovery* self)
{
    // discarded discovery argument is required by the listener
    (*self->m_callback)(*self);
}
```

As soon as the callback is registered, the listener thread will invoke it on any service availability change.
There is a small caveat though that while a callback is called on any change, we can only access
the latest discovery information by e.g. calling `findService`.
This means all intermediate changes cannot be detected, in particular we may encounter an ABA problem of service availability:
the service is available, becomes unavailable and available again in quick succession.
If the callback issues a `findService`, it will not observe any change in this case.
As one is usually mainly interested in the available services this can be considered a minor limitation.

To stop monitoring changes in the availability of services we simply call

<!--[geoffrey][iceoryx_examples/icediscovery/src/discovery_monitor.cpp][deregisterCallback]-->
```cpp
void Discovery::deregisterCallback()
{
    if (m_callback)
    {
        m_listener.detachEvent(*m_discovery, iox::runtime::ServiceDiscoveryEvent::SERVICE_REGISTRY_CHANGED);
    }
    m_callback.reset();
}
```

which detaches the callback from the listener.

As before we built on an `iox::runtime::ServiceDiscovery` by composition and defined a custom`findService` function
which returns a `std::vector`.

<center>
[Check out icediscovery on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/icediscovery){ .md-button } <!--NOLINT github url required for website-->
</center>
