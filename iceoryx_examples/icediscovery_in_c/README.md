# icediscovery in C

## Introduction

This example demonstrates how to search for specific services using iceoryx's
service discovery. It provides two applications - one offering different
services and one searching for those with different search queries. The
behavior and structure is quite similar to the [icediscovery C++ example](../icediscovery).

<!--
## Expected Output

/// @todo iox-#1295 re-record asciicast

[![asciicast](https://asciinema.org/a/476732.svg)](https://asciinema.org/a/476732)
-->

## Code walkthrough

### Offer services

We create several publishers which offer their services on construction by
default. For more dynamism the `cameraPublishers` offer/stop their services
periodically. If you want more information on how to create publishers,
have a look at the [icedelivery C example](../icedelivery_in_c).

### Find services

To be able to search for services, we need to include:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][include service discovery]-->
```c
#include "iceoryx_binding_c/service_discovery.h"
```

We create some storage for the service discovery and initialize it.

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][create service discovery handle]-->
```c
iox_service_discovery_storage_t storage;
iox_service_discovery_t serviceDiscovery = iox_service_discovery_init(&storage);
```

We can now call three different find service functions. Let's start with

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][find service and apply callable]-->
```c
iox_service_discovery_find_service_apply_callable(
    serviceDiscovery, "Radar", "FrontLeft", "Objects", printSearchResult, MessagingPattern_PUB_SUB);
```

which searches for all `{Radar, FrontLeft, Objects}` services offered by
publishers and applies the provided function `printSearchResult` on each of
them. The function must have the signature
`void(const iox_service_description_t)`. Here we pass a function that prints the
found services on the console:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][print function to be applied to search results]-->
```c
void printSearchResult(const iox_service_description_t service)
{
    printf(
        "- Service: %s, Instance: %s, Event: %s\n", service.serviceString, service.instanceString, service.eventString);
}
```

We cannot only search for exact matching services. In combination with
wildcards (`NULL`) we can also search for all instances and events with the
service `Radar`:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][search for all Radar services]-->
```c
iox_service_discovery_find_service_apply_callable(
    serviceDiscovery, "Radar", NULL, NULL, printSearchResult, MessagingPattern_PUB_SUB);
```

The wildcard can be used for all strings which describe a service, i.e. service,
instance and event.

Let's now search for all `Camera` services and store the results in an array:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][search for all Camera services]-->
```c
numberFoundServices = iox_service_discovery_find_service(serviceDiscovery,
                                                         "Camera",
                                                         NULL,
                                                         NULL,
                                                         searchResult,
                                                         SEARCH_RESULT_CAPACITY,
                                                         &missedServices,
                                                         MessagingPattern_PUB_SUB);
```

`searchResult` is a `iox_service_description_t` array of size
`SEARCH_RESULT_CAPACITY`. The matching services are written into this array, up
to a maximum of `SEARCH_RESULT_CAPACITY`. If the number of found services
exceeds the array's capacity, the number of services that could not be stored is
written to `missedServices`. The number of stored services is returned. Since
the `cameraPublishers` periodically offer/stop their services, you should see
sometimes 5 `Camera` services and sometimes none.

Finally, let's try out the third find service function. We search again for all
`Camera` services but additionally count the front camera services:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][search for all front camera services]-->
```c
iox_service_discovery_find_service_apply_callable_with_context_data(
    serviceDiscovery, "Camera", NULL, NULL, searchFrontDevices, &numberFrontCameras, MessagingPattern_PUB_SUB);
```

This function is quite similar to the first find service function, but we
pass an additional argument `numberFrontCameras`:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][store number of front cameras]-->
```c
uint32_t numberFrontCameras = 0U;
```

We use this variable to store the number of front cameras and provide it as
second argument `count` to the function `searchFrontDevices` which is applied to
all found services:

<!--[geoffrey][iceoryx_examples/icediscovery_in_c/iox_c_find_service.c][search function for all front devices]-->
```c
void searchFrontDevices(const iox_service_description_t service, void* count)
{
    if (strncmp(service.instanceString, "FrontLeft", IOX_CONFIG_SERVICE_STRING_SIZE) == 0
        || strncmp(service.instanceString, "FrontRight", IOX_CONFIG_SERVICE_STRING_SIZE) == 0)
    {
        ++*(uint32_t*)count;
    }
}
```

<center>
[Check out icediscovery on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/icediscovery_in_c){ .md-button } <!--NOLINT github url required for website-->
</center>
