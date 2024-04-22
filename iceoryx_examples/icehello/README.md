# icehello

## Introduction

This example demonstrates a basic data transmission with zero-copy inter-process communication (IPC).
It provides a publisher and a subscriber application.

## Expected Output

[![asciicast](https://asciinema.org/a/407357.svg)](https://asciinema.org/a/407357)

## Code walkthrough

### Publisher

At first, we need to define what kind of data type the publisher and subscriber application will exchange:

<!--[geoffrey][iceoryx_examples/icehello/topic_data.hpp][radar object]-->
```cpp
struct RadarObject
{
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};
```

It is included via:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][include topic]-->
```cpp
#include "topic_data.hpp"
```

Next, we include the publisher and the runtime:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][include]-->
```cpp
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

We create a runtime object to communicate with the RouDi daemon. We use a unique string for identifying our application:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][initialize runtime]-->
```cpp
constexpr char APP_NAME[] = "iox-cpp-publisher-helloworld";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

Now we create a publisher instance for our charming struct. Notice that the topic type is passed as a template
parameter:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][create publisher]-->
```cpp
iox::popo::Publisher<RadarObject> publisher({"Radar", "FrontLeft", "Object"});
```

The three strings which are passed as parameter to the constructor of `iox::popo::Publisher` define our
`capro::ServiceDescription`. `capro` stands for **ca**nionical **pro**tocol and is used to abstract different
[SoA](https://en.wikipedia.org/wiki/Service-oriented_architecture) protocols. `Radar` is the service name, `FrontLeft`
an instance of the service `Radar` and the third string the specific event `Object` of the instance.
In iceoryx, a publisher and a subscriber are connected only if all three IDs match.

For exiting on Ctrl+C, we use the `SignalWatcher`
<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][include sig watcher]-->
```cpp
#include "iox/signal_watcher.hpp"
```

and loop in our `while` loop until it states that `SIGINT` or `SIGTERM` was sent via
the function `hasTerminationRequested`.
<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][wait for term]-->
```cpp
while (!iox::hasTerminationRequested())
```

In order to send our sample, we loan some shared memory inside the `while` loop:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][loan]-->
```cpp
auto loanResult = publisher.loan();
```

If loaning was successful, we assign the incremented counter to all three values in `RadarObject` and `publish()` to the subscriber application:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][publish]-->
```cpp
if (loanResult.has_value())
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

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][error]-->
```cpp
else
{
    auto error = loanResult.error();
    // Do something with error
    std::cerr << "Unable to loan sample, error code: " << error << std::endl;
}
```

Topics are printed and published every second:

<!--[geoffrey][iceoryx_examples/icehello/iox_publisher_helloworld.cpp][msg]-->
```cpp
std::cout << APP_NAME << " sent value: " << ct << std::endl;
std::this_thread::sleep_for(std::chrono::seconds(1));
```

### Subscriber

The subscriber needs to have similar includes, but unlike the publisher `subscriber.hpp` is included:

<!--[geoffrey][iceoryx_examples/icehello/iox_subscriber_helloworld.cpp][include]-->
```cpp
#include "topic_data.hpp"

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
```

As well as the publisher, the subscriber needs to register with the daemon RouDi:

<!--[geoffrey][iceoryx_examples/icehello/iox_subscriber_helloworld.cpp][initialize runtime]-->
```cpp
constexpr char APP_NAME[] = "iox-cpp-subscriber-helloworld";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

Next, the subscriber object is created, again passing the topic type `RadarObject` as template parameter:

<!--[geoffrey][iceoryx_examples/icehello/iox_subscriber_helloworld.cpp][initialize subscriber]-->
```cpp
iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"});
```

Publisher and subscriber will only be connected if they both use exactly these same three strings, our `capro::ServiceDescription`.

Inside the `while` loop, we take the sample from shared memory and print it if we acquired it successfully.

<!--[geoffrey][iceoryx_examples/icehello/iox_subscriber_helloworld.cpp][receive]-->
```cpp
auto takeResult = subscriber.take();
if (takeResult.has_value())
{
    std::cout << APP_NAME << " got value: " << takeResult.value()->x << std::endl;
}
```

In case an error occurred during taking, we need to handle it:

<!--[geoffrey][iceoryx_examples/icehello/iox_subscriber_helloworld.cpp][error]-->
```cpp
if (takeResult.error() == iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
{
    std::cout << "No chunk available." << std::endl;
}
else
{
    std::cout << "Error receiving chunk." << std::endl;
}
```

The subscriber application polls for the sample ten times faster than the publisher is sending it.
Therefore no samples should be missed, but not every time the subscriber tries
to take a sample, it will get some. In this case, we print "No chunk available.".

<!--[geoffrey][iceoryx_examples/icehello/iox_subscriber_helloworld.cpp][wait]-->
```cpp
std::this_thread::sleep_for(std::chrono::milliseconds(100));
```

Increasing the polling rate is just one approach for reliable communication.
[iceoptions](../iceoptions) explains how to
configure the history size of a subscriber. In the
[WaitSet](../waitset) example you learn how to
avoid polling altogether.

<center>
[Check out icehello on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/icehello){ .md-button } <!--NOLINT github url required for website-->
</center>
