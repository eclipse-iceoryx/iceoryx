# icehello

## Introduction

This example demonstrates a basic data transmission with zero-copy inter-process communication (IPC).
It provides a publisher and a subscriber application.

## Expected output

<!-- Add asciinema link here -->

## Code walkthrough

### Publisher

At first, we need to define what kind of topic the publisher and subscriber application will be exchanging:

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

Next up, we include the publisher and the runtime:

```cpp
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

For the communication with the daemon RouDi, a runtime object is created. We use a unique string for identifying our application:

```cpp
constexpr char APP_NAME[] = "iox-ex-publisher-helloworld";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

Up next, we create a publisher instance for our charming struct, notice the topic type is passed as a template parameter:

```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

The three strings passed as parameter to the constructor of `iox::popo::Publisher` are of the type
`capro::ServiceDescription`. `capro` stands for **ca**nionical **pro**tocol and is used to abstract different
[SoA](https://en.wikipedia.org/wiki/Service-oriented_architecture) protocols. `Radar` is the service name, `FrontLeft`
an instance of the service `Radar` and the third string the specific event `Object` of the instance.
In iceoryx a publisher and a subscriber are connected only if all the three IDs match.

For exiting on Ctrl-C, we register a `signalHandler`, that flips `bool killswitch`:

```cpp
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
// snip
auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);
```

In order to send our topic we loan some some shared memory, inside the `while` loop:

```cpp
auto loanResult = publisher.loan();
```

If loaning was successful, we assign the incremented counter to all three values in `RadarObject` and `publish()` to the subscriber application:

```cpp
if (!loanResult.has_error())
{
    auto& sample = loanResult.value();
    // Sample can be held until ready to publish
    sample->x = ct;
    sample->y = ct;
    sample->z = ct;
    sample.publish();
}
```

In case an error occurred during loaning, we need to handle it:

```cpp
else
{
    auto error = loanResult.get_error();
    // Do something with error
    std::cerr << "Unable to loan sample, error code: " << static_cast<uint64_t>(error) << std::endl;
}
```

Topics are printed and published every second:

```cpp
std::cout << APP_NAME << " sent value: " << ct << std::endl;
std::this_thread::sleep_for(std::chrono::seconds(1));
```

### Subscriber

The subscriber needs to have similar includes, but unlike the publisher `subscriber.hpp` is included:

```cpp
#include "topic_data.hpp"

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
```

As well as the publisher, also the subscriber needs to register with the daemon RouDi:

```cpp
constexpr char APP_NAME[] = "iox-ex-subscriber-helloworld";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

Next, the subscriber object is created, again passing the topic type `RadarObject` as template parameter:

```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
```

Publisher and subscriber will only be connected if they both use exactly the same three strings.

Inside the `while` loop, we take the topic from shared memory and print it if taking was successfull:

```cpp
auto takeResult = subscriber.take();
if (!takeResult.has_error())
{
    std::cout << APP_NAME << " got value: " << takeResult.value()->x << std::endl;
}
```

In case an error occurred during taking, we need to handle it:

```cpp
if (takeResult.get_error() == iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
{
    std::cout << "No chunk available." << std::endl;
}
else
{
    std::cout << "Error receiving chunk." << std::endl;
}
```

The subscriber application runs ten times faster than the publisher and should therefore never miss any samples:

```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(100));
```

<center>
[Check out icehello on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/icehello){ .md-button }
</center>