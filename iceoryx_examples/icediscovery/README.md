# icediscovery

## Introduction

This example demonstrates how to search for specific services using iceoryx's
`ServiceDiscovery`. It provides two applications - one offering different
services and one searching for these making different search queries.
A `service` in iceoryx is defined by a `ServiceDescription` that represents a
topic under which publisher/server and subscriber/client can exchange data.

In addition the applications `iox-wait-for-service` and `iox-discovery-monitor` demonstrate how
to write custom discovery functionality to wait for specific services or monitor
the availability of services, respectively.

<!--## Expected Output-->
<!-- @todo Add expected output with asciinema recording before v2.0-->

## Code walkthrough

### Offer services

We create several publishers which offer their services on construction by
default. For more dynamism the `cameraPublishers` offer/stop their services
periodically. If you want more information on how to create a publisher, have a
look at the [icehello](https://github.com/eclipse-iceoryx/iceoryx/tree/v2.0.0/iceoryx_examples/icehello),
[icedelivery](https://github.com/eclipse-iceoryx/iceoryx/tree/v2.0.0/iceoryx_examples/icedelivery),
and [iceoptions](https://github.com/eclipse-iceoryx/iceoryx/tree/v2.0.0/iceoryx_examples/icedelivery)
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
service [string identifiers](https://github.com/eclipse-iceoryx/iceoryx/blob/v2.0.0/doc/website/getting-started/overview.md#creating-service-descriptions-for-topics)
and a callable which will be applied to all matching services. In this example
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
                             iox::capro::IdString_t{"Image"},
                             printSearchResult);
```

or add wildcards to our search query:

<!--[geoffrey][iceoryx_examples/icediscovery/iox_find_service.cpp][search for all Camera services]-->
```cpp
serviceDiscovery.findService(
    iox::capro::IdString_t{"Camera"}, iox::capro::Wildcard, iox::capro::Wildcard, printSearchResult);
```

With the above `findService` call we look for every `Camera` service with any
instance and any event. Since the `cameraPublishers` periodically offer/stop
their services, you should see sometimes 5 `Camera` services and sometimes none.

<center>
[Check out icediscovery on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/v2.0.0/iceoryx_examples/icediscovery){ .md-button }
</center>

### Wait for services

Start the applications `iox-wait-for-service` and `iox-offer-service` (in any order, but for demonstration purposes `iox-offer-service` should be started last).

`iox-wait-for-service` uses a customized service discovery `Discovery` which supports to wait for services by including

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][include custom discovery]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][include custom discovery]-->

We then can use our custom discovery class

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][create custom discovery]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][create custom discovery]-->

which provides a function `waitUntil` to wait for some discovery-related search query condition.

We define the search query

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][define search query]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][define search query]-->

This is essentially any callable with `bool(void)` signature, but it should depend on the discovery somehow (by capture),
as it is only checked when the service availability changes in some way. Here we require some specific service to be found
before we proceed.

Now we can wait until the service discovery changes and the services becomes available.
<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][wait until service was available]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][wait until service was available]-->
This wait is blocking until the service was available. If it already is available we do not block and proceed.
It is important that due to the nature of concurrent systems we cannot know that the service is still available
once we return from `waitUntil`, as the application offering the service may have stopped doing so in the meantime.

Usually we will assume that the service is available and may continue, e.g. by creating subscribers and running
application specific code.

We can also block until any unspecified change in the service availability occurs
<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][wait until discovery changes]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][wait until discovery changes]-->

This change is relative to the last `findService` call we issued, i.e. if something changed compared to
the available services at this point, we wake up and continue.

We then can check any condition we like, but usually it will be most useful to again check discovery-related conditions.
Here we check whether a particular service becomes unavailable (essentially the negation of our query before)
<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][check service availability]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][check service availability]-->

Note that we use a customized `findService` version which returns a result container which can easily be build
using the version which takes a function to be applied to all services in the search result.

Once the service becomes unavailable, the application exits.

Should the service we wait for never become available we can unblock any of the wait calls with

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][unblock wait]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_wait_for_service.cpp][unblock wait]-->

### Monitor service availability

If we want to continously monitor the availability of some service or check some discovery condition we can do so by using e.g. a listener.
To do so, we start the applications `iox-discovery-monitor` and `iox-offer-service` (again in any order, but for demonstration purposes `iox-offer-service` should be started last).

Again we can use a service discovery `Discovery` customized for this purpose by including

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][include custom discovery]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][include custom discovery]-->

and creating it like so

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][create custom discovery]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][create custom discovery]-->

Afterwards we create a callback to be called whenever the service availability changes.

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][create monitoring callback]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][create monitoring callback]-->

This callback essentially checks whether a specific service is available or unavailable and generates output accordingly.
Other reactions are possible as well, such as changing the processing logic of an pplication.

To start the monitoring, we register the callback

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][register callback]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][register callback]-->

Monitoring happens in a background thread implicitly created by the `Discovery`, i.e. the callback is executed in this thread.

When we want to stop monitoring we have to deregister the callback

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][deregister callback]-->

<!--[geoffrey][iceoryx_examples/icediscovery/iox_discovery_monitor.cpp][deregister callback]-->

Here this is done at the very end where it is technically not required, but in a more complex example it could be done
while the application is processing data. The main processing loop of the application is deliberately left empty for simplicty.
Usually it would interact with the callback by e.g. changing application behavior whenever the availability of some service changes.

While we only can attach one callback to the general event that the service availability changes in some way, we can generalize the mechanism
here to check for multiple conditions and react to each of them by e.g. calling a specific function.
These conditions would still need to be checked in the callback we defined though.

### Implementation of Discovery with blocking wait

### Implemtation of Discovery monitoring
