# icediscovery

## Introduction

This example demonstrates how to search for specific services using iceoryx's
`ServiceDiscovery`. It provides two applications - one offering different
services and one searching for these making different search queries.
A `service` in iceoryx is defined by a `ServiceDescription` that represents a
topic under which publisher/server and subscriber/client can exchange data.

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
