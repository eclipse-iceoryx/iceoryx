# icedelivery

## Introduction

This example showcases a data transmission setup with zero-copy inter-process communication (IPC) on iceoryx.
It provides publisher and subscriber applications. They come in two C++ API flavors (untyped and typed).

## Expected Output

[![asciicast](https://asciinema.org/a/476740.svg)](https://asciinema.org/a/476740)

## Code walkthrough

This example makes use of two kinds of API flavors. With the untyped API, you have the most flexibility. It enables you
to define higher level APIs with a different look and feel on top of iceoryx, e.g. the ara::com API of AUTOSAR Adaptive or
the ROS 2 API. It is not meant to be used by developers in daily life, the assumption is that there will always be a higher
abstraction. A simple example how such an abstraction could look like is given in the second step with the typed
example. The typed API provides type safety combined with [RAII](https://en.cppreference.com/w/cpp/language/raii).

### Publisher application (untyped)

First off, let's include the publisher, the runtime and the signal handler:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][includes]-->
```cpp
#include "iceoryx_posh/popo/untyped_publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
```

You might be wondering what the publisher application is sending? It's this struct.

<!--[geoffrey][iceoryx_examples/icedelivery/topic_data.hpp][topic data]-->
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

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][include topic data]-->
```cpp
#include "topic_data.hpp"
```

For the communication with RouDi a runtime object is created. The parameter of the method `initRuntime()` contains a
unique string identifier for this publisher.

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][runtime initialization]-->
```cpp
constexpr char APP_NAME[] = "iox-cpp-publisher-untyped";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

Now that RouDi knows our publisher application is existing, let's create a publisher instance for sending our charming
struct to everyone:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][create untyped publisher]-->
```cpp
iox::popo::UntypedPublisher publisher({"Radar", "FrontLeft", "Object"});
```

Now comes the work mode. Data needs to be created. But hang on.. we need memory first! Let's reserve a memory chunk
which fits our RadarObject struct.

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][[Loan chunk and provide logic to populate it via a lambda]]-->
```cpp
publisher.loan(sizeof(RadarObject))
    .and_then([&](auto& userPayload) {
        // ...
    })
    .or_else([&](auto& error) {
        // ...
    });
```

The call to `loan()` returns a `expected`. By concatenating `and_then` and `or_else` branches are implicitly taken
and your code becomes less error-prone compared to using `if() { .. } else { .. }`. Well, it's a bit of a
[lambda](https://en.wikipedia.org/wiki/Anonymous_function#C++_(since_C++11)) jungle. Read it like a story in a book:
"Loan memory and then if it succeeds, fill it with some data or else if it fails, handle the error".

Remember, the untyped API will always be bare-metal!

Hence, the `RadarObject` needs to be constructed with a placement new:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][construct RadarObject]-->
```cpp
RadarObject* data = new (userPayload) RadarObject(ct, ct, ct);
```

Then, we can write some values:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][write data]-->
```cpp
data->x = ct;
data->y = ct;
data->z = ct;
```

Finally, the value is made available to any subscriber with

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher_untyped.cpp][publish]-->
```cpp
publisher.publish(userPayload);
```

Incrementing the counter and sending the data happens in a loop every second until the user presses `Ctrl-C`. It is
captured with the signal handler and stops the loop.

### Subscriber application (untyped)

How can the subscriber application receive the data the publisher application just transmitted?

Similar to the publisher, we include the topic data, the subscriber, the runtime as well as the signal handler header:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][includes]-->
```cpp
#include "topic_data.hpp"

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
```

To make RouDi aware of the subscriber a runtime object is created, once again with a unique identifier string:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][initialize runtime]-->
```cpp
constexpr char APP_NAME[] = "iox-cpp-subscriber-untyped";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

In the next step a subscriber object is created, matching exactly the `capro::ServiceDescription` that the publisher
offered:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][create untyped subscriber]-->
```cpp
iox::popo::UntypedSubscriber subscriber({"Radar", "FrontLeft", "Object"});
```

Again in a while-loop we do the following:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][[loop] [chunk happy path]]-->
```cpp
while (!iox::hasTerminationRequested())
{
    subscriber
        .take()
        .and_then([&](const void* userPayload) {
            // ...
        })
        .or_else([](auto& result) {
            if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
            {
                std::cout << "Error receiving chunk." << std::endl;
            }
        });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}
```

The program execution is stopped when the user presses `Ctrl-C`.

Let's translate it into a story again: "Take the data and then if this succeeds, work with the sample, or else if an
error other than `NO_CHUNK_AVAILABLE` occurred, print the string 'Error receiving chunk.'". Of course, you don't need to
take care of all cases, but we advise doing so.

In the `and_then` case the content of the sample is printed to the command line:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][chunk received]-->
```cpp
auto object = static_cast<const RadarObject*>(userPayload);
std::cout << APP_NAME << " got value: " << object->x << std::endl;
```

Please note the `static_cast` before reading out the data. It is necessary because the untyped subscriber is unaware
of the type of the transmitted data.

After accessing the value, the chunk of memory needs to be explicitly released by calling:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][release]-->
```cpp
// note that we explicitly have to release the sample
// and afterwards the pointer access is undefined behavior
subscriber.release(userPayload);
```

The subscriber runs 10x times faster than the publisher, to make sure that all data samples are received.

### Publisher application (typed)

The typed publisher application is an example for a high-level user API and does the same thing as the untyped
publisher described before. In this summary, just the differences to the prior publisher application are described.

Starting again with the includes, there is now a different one:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher.cpp][include publisher]-->
```cpp
#include "iceoryx_posh/popo/publisher.hpp"
```

When it comes to the runtime, things are the same as in the untyped publisher. However, a typed publisher object is
created and the transmitted data type is provided as template parameter:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher.cpp][create publisher]-->
```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

A similar while-loop is used to send the data to the subscriber. In contrast to the untyped publisher the typed one
offers three additional possibilities.

#### #1 Loan and publish

Usage #1 default constructs the data type in-place:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher.cpp][API Usage #1]-->
```cpp
//  * Retrieve a typed sample from shared memory.
//  * Sample can be held until ready to publish.
//  * Data is default constructed during loan
publisher.loan()
    .and_then([&](auto& sample) {
        sample->x = sampleValue1;
        sample->y = sampleValue1;
        sample->z = sampleValue1;
        sample.publish();
    })
    .or_else([](auto& error) {
        // Do something with error
        std::cerr << "Unable to loan sample, error: " << error << std::endl;
    });
```

#### #2 Loan, construct in-place and publish

Usage #2 constructs the data type with the values provided in loan:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher.cpp][API Usage #2]-->
```cpp
//  * Retrieve a typed sample from shared memory and construct data in-place
//  * Sample can be held until ready to publish.
//  * Data is constructed with the arguments provided.
publisher.loan(sampleValue2, sampleValue2, sampleValue2)
    .and_then([](auto& sample) { sample.publish(); })
    .or_else([](auto& error) {
        // Do something with error
        std::cerr << "Unable to loan sample, error: " << error << std::endl;
    });
```

One might wonder what the type of the variable `sample` is? It is `iox::popo::Sample<RadarObject>`. This class behaves
similar to a [`std::unique_ptr`](https://en.cppreference.com/w/cpp/memory/unique_ptr) and makes sure that the ownership
handling is done automatically and memory is freed when going out of scope on subscriber side.

#### #3 Publish by copy

Usage #3 does a copy-and-publish in one call. This should only be used for small data types, as otherwise copies can
lead to a larger runtime.

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher.cpp][API Usage #3]-->
```cpp
//  * Basic copy-and-publish. Useful for smaller data types.
auto object = RadarObject(sampleValue3, sampleValue3, sampleValue3);
publisher.publishCopyOf(object).or_else([](auto& error) {
    // Do something with error.
    std::cerr << "Unable to publishCopyOf, error: " << error << std::endl;
});
```

#### #4 Publish the result of a computation

Usage #4 can be useful if you have a callable, e.g. a function or
[functor](https://en.wikipedia.org/wiki/Function_object#In_C_and_C++) that should always be called. The callable needs
to have the signature `void(SampleType*, ...)`.  The semantics are as follows: The publisher loans a sample from
shared memory and if loaning was successful the callable is called with a pointer to the `SampleType` as first
argument. If loaning was unsuccessful, the callable is not called, but instead the `or_else` branch is taken.

<!--[geoffrey][iceoryx_examples/icedelivery/iox_publisher.cpp][API Usage #4]-->
```cpp
//  * Provide a callable that will be used to populate the loaned sample.
//  * The first argument of the callable must be T* and is the location that the callable should
//      write its result to.
publisher.publishResultOf(getRadarObject, ct).or_else([](auto& error) {
    // Do something with error.
    std::cerr << "Unable to publishResultOf, error: " << error << std::endl;
});
publisher
    .publishResultOf([&sampleValue4](RadarObject* object) {
        *object = RadarObject(sampleValue4, sampleValue4, sampleValue4);
    })
    .or_else([](auto& error) {
        // Do something with error.
        std::cerr << "Unable to publishResultOf, error: " << error << std::endl;
    });
```

### Subscriber application (typed)

As with the typed publisher application there is a different include compared to the untyped subscriber:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber.cpp][include subscriber]-->
```cpp
#include "iceoryx_posh/popo/subscriber.hpp"
```

An instance of `Subscriber` is created:

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber.cpp][create subscriber]-->
```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
```

Everything else is nearly the same. However, there is one crucial difference which makes the `Subscriber` typed.

Compare this line from the `UntypedSubscriber`

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber_untyped.cpp][[chunk happy path]]-->
```cpp
.and_then([&](const void* userPayload) {
    // ...
})
```

with

<!--[geoffrey][iceoryx_examples/icedelivery/iox_subscriber.cpp][[sample happy path]]-->
```cpp
.and_then([](auto& sample) {
    // ...
})
```

In case of the typed `Subscriber`, `auto` is deduced to `iox::popo::Sample<const RadarObject>`. With the
`UntypedSubscriber` the parameter is `const void*` as no type information is available.

<center>
[Check out icedelivery on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/icedelivery){ .md-button } <!--NOLINT github url for website-->
</center>
