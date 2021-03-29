# icedelivery

## Introduction

This example showcases a data transmission setup with zero-copy inter-process communication (IPC) on iceoryx.
It provides publisher and subscriber applications. They come in two C++ API flavours (untyped and typed).

## Expected output

Create three terminals and start RouDi, a publisher and a subscriber. You can also mix the typed and untyped versions.

[![asciicast](https://asciinema.org/a/382036.svg)](https://asciinema.org/a/382036)

## Code walkthrough

This example makes use of two kind of API flavours. With the untyped API you have the most flexibility. It enables you
to put higher level APIs with different look and feel on top of iceoryx. E.g. the ara::com API of AUTOSAR Adaptive or
the ROS2 API. It is not meant to be used by developers in daily life, the assumption is that there will always be a higher
abstraction. A simple example how such an abstraction could look like is given in the second step with the typed
example. The typed API provides type safety combined with [RAII](https://en.cppreference.com/w/cpp/language/raii).

### Publisher application (untyped)

First off, let's include the publisher, the runtime and the signal handler:

```cpp
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
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
constexpr char APP_NAME[] = "iox-ex-publisher-untyped";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

Now that RouDi knows our publisher application is existing, let's create a publisher instance for our charming struct
to everyone:

```cpp
iox::popo::UntypedPublisher publisher({"Radar", "FrontLeft", "Object"});
```

Now comes the work mode. Data needs to be created. But hang on.. we need memory first! Let's reserve a memory chunk
which fits our RadarObject struct

```cpp
auto result = publisher.loan(sizeof(RadarObject));
```

#### #1 Loan and emplace

Two different ways of handling the returned `cxx::expected` are possible. Either you save the result in a variable and
do the error check with an if-condition (#1):

```cpp
auto result = publisher.loan(sizeof(RadarObject));
if (!result.has_error())
{
    // ...
}
else
{
    // ...
}
```

#### #2 Functional approach with loaning

Or try the functional way (#2) by concatenating `and_then` and `or_else`. Well, that's a bit of a
[lambda](https://en.wikipedia.org/wiki/Anonymous_function#C++_(since_C++11)) jungle. Read it like a story in a book:
"Loan memory and then if it succeeds, fill it with some data or else if it fails, handle the error"

```cpp
publisher.loan(sizeof(RadarObject))
    .and_then([&](auto& chunk)
    {
        // ...
    })
    .or_else([&](iox::popo::AllocationError error)
    {
        // ...
    });
```

If choosing #1, please mind the additional step to unwrap the `cxx::expected` with `value()`

```cpp
if (!result.has_error())
{
    void* chunk = result.value();
    // ...
}
```

Whichever way you choose, the untyped API will be bare-metal!

Hence, the `RadarObject` needs to be constructed with a placement new:

```cpp
RadarObject* data = new (chunk) RadarObject(ct, ct, ct);
```

Then, we can write some values:

```cpp
data->x = 1.0;
data->y = 2.0;
data->z = 3.0;
```

And finanlly, in both ways, the value is made available to other subscribers with

```cpp
publisher.publish(chunk);
```

The incrementation and sending of the data is done in a loop every second till the user pressed `Ctrl-C`. It is
captured with the signal handler and stops the loop.

### Subscriber application (untyped)

How can the subscriber application receive the data the publisher application just transmitted?

Similar to the publisher we need to include the runtime and the subscriber as well as the topic data header:

```cpp
#include "topic_data.hpp"

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
```

To make RouDi aware of the subscriber an runtime object is created, once again with a unique identifier string:

```cpp
constexpr char APP_NAME[] = "iox-ex-subscriber-untyped";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

In the next step a subscriber object is created, matching exactly the `capro::ServiceDescription` that the publisher
offered:

```cpp
iox::popo::UntypedSubscriber untypedSubscriber({"Radar", "FrontLeft", "Object"});
```

When using the default n:m communication philosophy, the `SubscriptionState` is immediately `SUBSCRIBED`.
However, when restricting iceoryx to the 1:n communication philosophy before being in the state `SUBSCRIBED`, the state is change to `SUBSCRIBE_REQUESTED`.

Again in a while-loop we do the following:

```cpp
while (!killswitch)
{
    untypedSubscriber.take()
        .and_then([](const void* chunk)
        {
            // ...
        })
        .or_else([](auto& result)
        {
            if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
            {
                std::cout << "Error receiving chunk." << std::endl;
            }
        });
```

The `killswitch` will be used to stop the programm execution.

Let's translate it into a story again: "Take the data and then if this succeeds, work with the sample, or else if an
error occured, print the string 'Error receiving chunk.'" Of course you don't need to take care about all cases, but
it is advised to do so.

In the `and_then` case the content of the sample is printed to the command line:

```cpp
auto object = static_cast<const RadarObject*>(chunk);
std::cout << APP_NAME << " got value: " << object->x << std::endl;
```

Please note the `static_cast` before reading out the data. It is necessary, because the untyped subscriber is unaware
of the type of the transmitted data.

After accessing the value, the chunk of memory needs to be explicitly released by calling:

```cpp
subscriber.release(chunk);
```

The subscriber runs 10x times faster than the publisher, to make sure that all data samples are received.

### Publisher application (typed)

The typed publisher application is an example for a high-level user API and does the same thing as the publisher
described before. In this summary, just the differences to the prior publisher application are described.

Starting again with the includes, there is now a different one:

```cpp
#include "iceoryx_posh/popo/publisher.hpp"
```

When it comes to the runtime, things are the same as in the untyped publisher. However, a typed publisher object is
created and the transmitted data type is provided as template parameter:

```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

The topic type must be default- and copy-constructible when the typed API is used.

A similar while-loop is used to send the data to the subscriber. In contrast to the untyped publisher the typed one
offers three additional possibilities.

#### #2 Functional approach with loaning

Usage #2 constructs the data type with the values provided in loan:

```cpp
auto result = publisher.loan(ct, ct, ct);
if (!result.has_error())
{
    auto& sample = result.value();
    sample->x = ct;
    sample->y = ct;
    sample->z = ct;
    sample.publish();
}
else
{
    // ..
}
```

One might wonder what the type of the variable `sample` is? It is `iox::popo::Sample<RadarObject>`. This class behaves
similar to a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr) and makes sure that the ownership
handling is done automatically and memory is freed when going out of scope on subscriber side.

#### #4 Publish by copy

Usage #4 does a copy-and-publish in one call. This should only be used for small data types, as otherwise copies can
lead to a larger runtime.

```cpp
auto object = RadarObject(ct, ct, ct);
publisher.publishCopyOf(object).or_else([](iox::popo::AllocationError) {
    // Do something with error.
});
```

#### #5 Publish the result of a computation

Usage #5 can be useful if you have a callable e.g. a function or [functor](https://en.wikipedia.org/wiki/Function_object#In_C_and_C++) should be always called

```cpp
publisher.publishResultOf(getRadarObject, ct).or_else([](iox::popo::AllocationError) {
    // Do something with error.
});
publisher.publishResultOf([&ct](RadarObject* object) { *object = RadarObject(ct, ct, ct); })
    .or_else([](iox::popo::AllocationError) {
        // Do something with error.
    });
```

### Subscriber application (typed)

As with the typed publisher application there is an different include compared to the untyped subscriber:

```cpp
#include "iceoryx_posh/popo/subscriber.hpp"
```

An instance of `Subscriber` is created:

```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
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
.and_then([](const void* sample)
{
    // ...
})
```

In case of the typed `Subscriber` it is a `const RadarObject` instead of `const void*`.
