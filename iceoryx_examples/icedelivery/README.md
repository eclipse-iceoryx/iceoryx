# icedelivery

## Introduction

This example showcases a data transmission setup with zero-copy inter-process communication (IPC) on iceoryx.
It provides publisher and subscriber applications. They come in two C++ API flavours (untyped and typed).

## Expected output

Create different terminals and run one command in each of them. Choose at least one publisher and one subscriber for having a data communication. You can also mix the typed and untyped versions. And if you feel like crazy today you start several publishers and subscribers from icedelivery and icedelivery_in_c (needs the default n:m communication, not possible if you build with the ONE_TO_MANY option)

```sh
# If installed and available in PATH environment variable
iox-roudi
# If build from scratch with script in tools
$ICEORYX_ROOT/build/iox-roudi


build/iceoryx_examples/icedelivery/iox-ex-publisher
# The untyped publisher is an alternative
build/iceoryx_examples/icedelivery/iox-ex-publisher-untyped


build/iceoryx_examples/icedelivery/iox-ex-subscriber
# The untyped subscriber is an alternative
build/iceoryx_examples/icedelivery/iox-ex-subscriber-untyped
```

[![asciicast](https://asciinema.org/a/382036.svg)](https://asciinema.org/a/382036)

## Code walkthrough

This example makes use of two kind of API flavours. With the untyped API you have the most flexibility. It enables you
to put higher level APIs with different look and feel on top of iceoryx. E.g. the ara::com API of AUTOSAR Adaptive or
the ROS2 API. It is not meant to be used by developers in daily life, the assumption is that there will always be a higher
abstraction. A simple example how such an abstraction could look like is given in the second step with the typed example.

### Publisher application (untyped)

First off, let's include the publisher and the runtime:
```cpp
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

You might be wondering what the publisher application is sending? It's this struct.
```cpp
struct RadarObject
{
    RadarObject() noexcept
    {
    }
    RadarObject(double x, double y, double z) noexcept
        : x(x)
        , y(y)
        , z(z)
    {
    }
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};
```

It is included by:
```cpp
#include "topic_data.hpp"
```

For the communication with RouDi a runtime object is created. The parameter of the method `initRuntime()` contains a
unique string identifier for this publisher.
```cpp
iox::runtime::PoshRuntime::initRuntime("iox-ex-publisher");
```

Now that RouDi knows our publisher application is existing, let's create a publisher instance for our charming struct
to everyone:
```cpp
iox::popo::UntypedPublisher untypedPublisher({"Radar", "FrontLeft", "Object"});
```

The strings inside the first parameter of the constructor of `iox::popo::Publisher` are of the type
`capro::ServiceDescription`. `capro` stands for **ca**nionical **pro**tocol and is used to abstract different
[SoA](https://en.wikipedia.org/wiki/Service-oriented_architecture) protocols. `Radar` is the service name, `FrontLeft`
an instance of the service `Radar` and the third string the specific event `Object` of the instance.
In iceoryx a publisher and a subscriber only match if all the three IDs match.

Now comes the work mode. Data needs to be created. But hang on.. we need memory first! Let's reserve a memory chunk
which fits our RadarObject struct
```cpp
auto result = untypedPublisher.loan(sizeof(RadarObject));
```

Two different ways of handling the returned `cxx::expected` are possible. Either you save the result in a variable and
do the error check with an if-condition (#1):
```cpp
auto result = untypedPublisher.loan(sizeof(RadarObject));
if (!result.has_error())
{
    // ...
}
else
{
    // ...
}
```

Or try the functional way (#2) by concatenating `and_then` and `or_else`. Read it like a story in a book: "Loan memory
and then if it succeeds, fill it with some data or else if it fails, handle the error"
```cpp
untypedPublisher.loan(sizeof(RadarObject))
    .and_then([&](auto& sample)
    {
        // ...
    }
    .or_else([&](iox::popo::AllocationError error)
    {
        // ...
    });
```

If choosing #1, please mind the additional step to unwrap the `cxx::expected` with `value()`
```cpp
if (!result.has_error())
{
    auto& sample = result.value();
    // ...
}
```

One might wonder what the type of the variable `sample` is? It is `iox::popo::Sample<void>`. This class behaves
similar to a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr) and makes sure that the ownership
handling is done automatically and memory is freed when going out of scope on subscriber side. One slight difference
is, if you want to take the ownership of the pointer, `Sample::release()` does not return the pointer.

Whichever way you choose, the untyped API will be bare-metal! A `void*` is contained inside the `iox::popo::Sample`.
Hence, the pointer needs to be casted to `RadarObject*`
```cpp
auto object = static_cast<RadarObject*>(sample.get());
```

Then we can write a new RadarObject value with an incremented counter in the shared memory
```cpp
*object = RadarObject(ct, ct, ct);
```

Finanlly, in both ways, the value is made available to other subscribers with
```cpp
sample.publish();
```

The incrementation and sending of the data is done in a loop every second till the user pressed `Ctrl-C`. It is
captured with the signal handler and stops the loop.

### Subscriber application (untyped)

How can the subscriber application receive the data the publisher application just transmitted?

Similar to the publisher we need to include the runtime and the subscriber as well as the topic data header:
```cpp
#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"
```

To make RouDi aware of the subscriber an runtime object is created, once again with a unique identifier string:
```cpp
iox::runtime::PoshRuntime::initRuntime("iox-ex-subscriber-untyped");
```

For quality of service a `popo::SubscriberOptions` object is created and the `queueCapacity` is set. This parameter
specifies how many samples the queue of the subscriber object can hold. If the queue would encounter an overflow,
the oldest sample is released to create space for the newest one, which is then stored.
```cpp
iox::popo::SubscriberOptions subscriberOptions;
subscriberOptions.queueCapacity = 10U;
```

In the next step a subscriber object is created, matching exactly the `capro::ServiceDescription` that the publisher
offered. Additionally, the previously created subscriber options are passed to the constructor. If no subscriber options
are created, a default value will be used which sets the queueCapacity to the maximum value:
```cpp
iox::popo::UntypedSubscriber untypedSubscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);
```

When using the default n:m communication philosophy, the `SubscriptionState` is immediately `SUBSCRIBED`.
However, when restricting iceoryx to the 1:n communication philosophy before being in the state `SUBSCRIBED`, the state is change to `SUBSCRIBE_REQUESTED`.

Again in a while-loop we do the following: First check for the `SubscriptionState`
```cpp
while (!killswitch)
{
    if (untypedSubscriber.getSubscriptionState() == iox::SubscribeState::SUBSCRIBED)
    {

```

The `killswitch` will be used to stop the programm execution.

Once the publisher has sent data, we can receive the data
```cpp
untypedSubscriber.take()
    .and_then([](iox::cxx::optional<iox::popo::Sample<const void>>& sample)
    {
        // ...
    })
    .if_empty([]
    {
        // ...
    })
    .or_else([](iox::popo::ChunkReceiveResult error)
    {
        std::cout << "Error receiving chunk." << std::endl;
    });
```

Well, that's a bit of a [lambda](https://en.wikipedia.org/wiki/Anonymous_function#C++_(since_C++11)) jungle. Let's
translate it into a story again: "Take the data and then if this succeeds, work with the sample, if the sample is empty
do something different, or else if an error occured, print the string 'Error receiving chunk.'" Of course you don't
need to take care about all cases, but it is advised to do so.

In the `and_then` case the content of the sample is printed to the command line:
```cpp
auto object = static_cast<const RadarObject*>(sample->get());
std::cout << "Got value: " << object->x << std::endl;
```

Please note the `static_cast` before reading out the data. It is necessary, because the untyped subscriber is unaware
of the type of the transmitted data.

If the `untypedSubscriber` was not yet subscribed
```cpp
std::cout << "Not subscribed!" << std::endl;
```
is printed.

The subscriber runs 10x times faster than the publisher, to make sure that all data samples are received.

### Publisher application (typed)

The typed publisher application is an example for a high-level user API and does the same thing as the publisher
described before. In this summary, just the differences to the prior publisher application are described.

Starting again with the includes, there is now a different one:
```cpp
#include "iceoryx_posh/popo/publisher.hpp"
```

When it comes to the runtime, things are the same as in the untyped publisher. However, a typed publisher object is
created
```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

A similar while-loop is used to send the data to the subscriber. In contrast to the untyped publisher the typed one
offers two additional possibilities
```cpp
auto object = RadarObject(ct, ct, ct);
publisher.publishCopyOf(object).or_else([](iox::popo::AllocationError) {
    // Do something with error.
});
```
This should only be used for small data types, as otherwise copies can lead to a larger runtime.

```cpp
publisher.publishResultOf(getRadarObject, ct).or_else([](iox::popo::AllocationError) {
    // Do something with error.
});
publisher.publishResultOf([&ct](RadarObject* object) { *object = RadarObject(ct, ct, ct); })
    .or_else([](iox::popo::AllocationError) {
        // Do something with error.
    });
```

If you have a callable e.g. a function should be always called, this approach could be a good solution for you.

Another difference compared to the untyped publisher, is the easier handling of `iox::popo::Sample`. There is no need
for any casts with the typed publisher, as the type of the stored data is know. One can directly access the data with
the `operator->()`.

### Subscriber application (typed)

As with the typed publisher application there is an different include compared to the untyped subscriber:
```cpp
#include "iceoryx_posh/popo/subscriber.hpp"
```

An instance of `Subscriber` is created:
```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);
```

Everything else is nearly the same. However, there is one crucial difference which makes the `Subscriber` typed.

Compare this line from the `UntypedSubscriber`
```cpp
.and_then([](iox::popo::Sample<const RadarObject>& object)
{
    // ...
})
```

with
```cpp
.and_then([](iox::popo::Sample<const void>& sample)
{
    // ...
})
```

The difference is the type that is contained in `iox::popo::Sample`. In case of the `Subscriber` it is a
`const RadarObject` instead of `const void`.
