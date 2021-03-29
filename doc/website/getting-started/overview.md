# Overview

This document covers the core functionality of Eclipse iceoryx and is intended to quickly get started to
set up iceoryx applications. It is no in-depth API documentation and while the API is still subject to changes, the
basic concepts presented here will still apply.

## General

To set up a collection of applications using iceoryx (an _iceoryx system_), the applications need to initialize a
runtime and create _publishers_ and _subscribers_. The publishers send data of a specific _topic_ which can be
received by subscribers of the same topic. To enable publishers to offer their topic and subscribers to subscribe to
these offered topics, the middleware daemon, called ``RouDi``, must be running.

For further information how iceoryx can be used, see the 
[examples](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/README.md). The
[conceptual guide](https://github.com/eclipse-iceoryx/iceoryx/blob/master/doc/conceptual-guide.md) provides additional 
information about the _Shared Memory communication_ that lies at the heart of iceoryx.

### icehello example

Before we get into more details let's start with a simple example:

We need to create a runtime with a unique name along all applications for each application.
```cpp
iox::runtime::PoshRuntime::initRuntime("some_unique_name");
```
Now this application is ready to communicate with RouDi and we can define the data type we want to send.
```cpp
struct CounterTopic
{
    uint32_t counter;
};
```
Then we create a publisher that offers our CounterTopic.
```cpp
iox::popo::Publisher<CounterTopic> publisher({"Group", "Instance", "CounterTopic"});
```
Now we can use the publisher to send the data.
```cpp
auto result = publisher.loan();
if(!result.has_error())
{
    auto& sample = result.value();
    sample->counter = 30;
    sample.publish();
}
else
{
    // handle the error
}
```
Here ``result`` is an ``expected`` and hence we may get an error which we have to handle. This can happen if we try 
to loan too many samples and exhaust memory. If you want to know more about ``expected``, take a look at 
[How optional and error values are returned in iceoryx](https://github.com/eclipse-iceoryx/iceoryx/blob/master/doc/website/advanced/how-optional-and-error-values-are-returned-in-iceoryx.md).

Let's create a corresponding subscriber.
```cpp
iox::popo::Subscriber<CounterTopic> subscriber({"Group", "Instance", "CounterTopic"});
```
Now we can use the subscriber to receive data. For simplicity, we assume that we periodically check for new data. It 
is also possible to explicitly wait for data using the 
[Waitset](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/waitset/README.md) or the 
[Listener](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/callbacks/README.md). The code to 
receive the data is the same, the only difference is the way we wake up before checking for data.
```cpp
while (keepRunning)
{
    // wait for new data (either sleep and wake up periodically or by notification from the waitset)

    auto result = subscriber.take();

    if(!result.has_error())
    {
        auto& sample = result.value();
        uint32_t counter = sample->counter;
        //process the data
    }
    else
    {
        //handle the error
    }
}
```
By calling ``take`` we get an ``expected`` and hence we may have to handle an error.

And that's it. We have created our first simple iceoryx example.
[Here](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/README.md) you can find further examples 
which demonstrate how iceoryx can be used and describe our API in more detail.

Now that we have applications capable of sending and receiving data, we can run the complete iceoryx system.

First we need to start RouDi.

```
# If installed and available in PATH environment variable
iox-roudi
# If build from scratch with script in tools
$ICEORYX_ROOT/build/iox-roudi
```

Afterwards, we can start the applications which immediately connect to the RouDi via their runtime.

When the application terminates, the runtime cleans up all resources needed for communication with RouDi. This
includes all memory chunks used for the data transmission which may still be held by the application.


We now briefly define the main entities of an iceoryx system which were already used in the example above.

## RouDi

RouDi is an abbreviation for **Rou**ting and **Di**scovery. RouDi takes care of the
communication setup but does not actually participate in the communication between the publisher and the subscriber.
RouDi can be thought of as the switchboard operator of iceoryx. One of his other major tasks is the setup of the
shared memory, which the applications use for exchanging payload data. Sometimes referred to as daemon, RouDi manages
the shared memory and is responsible for the service discovery, i.e. enabling subscribers to find topics offered by 
publishers. It also keeps track of all applications which have initialized a runtime and are hence able to use
publishers or subscribers. To view the available command line options call `$ICEORYX_ROOT/build/iox-roudi --help`.

## Runtime

Each application that wants to use iceoryx has to instantiate its runtime, which essentially enables communication
with RouDi. Only one runtime object per user process is allowed.

To do so, the following lines of code are required
```cpp
iox::runtime::PoshRuntime::initRuntime("some_unique_application_name");
```
## Creating service descriptions for topics

A ``ServiceDescription`` in iceoryx represents the data to be transmitted and is uniquely identified by three string
identifiers.

1. ``Group`` name
2. ``Instance`` name
3. ``Topic`` name

A triple consisting of such strings is called a ``ServiceDescription``. The service model of iceoryx is derived
from AUTOSAR and is still used in the API with these names. The so called canonical protocol is implemented in the
namespace ``capro``.

The following table gives an overview of the different terminologies and the current mapping:

|         | Group   | Instance         | Topic                  |
|---------|---------|------------------|------------------------|
| ROS2    | Type    | Namespace::Topic | -                      |
| AUTOSAR | Service | Instance         | Event                  |
| DDS     | -       | -                | /Group/Instance/Topic  |

Service and instance are like classes and objects in C++. So you always have a specific instance of a service during
runtime. The mapping will be reworked with release v2.0.

Two ``ServiceDescription``s are considered matching if all these three strings are element-wise equal, i.e. group,
instance and topic names are the same for both of them.

This means the group and instance identifier can be ignored to create different ``ServiceDescription``s. They will be
needed for advanced filtering functionality in the future.

The data type of the transmitted data can be any C++ class, struct or plain old data type as long as it satisfies the 
following conditions:
- no heap is used
- the data structure is entirely contained in shared memory - no pointers to process local memory, no references to 
process local constructs, no dynamic allocators
- the data structure must not internally use pointers/references
- no virtual members

## Publisher

A publisher is tied to a topic and needs a service description to be constructed. If it is typed one needs to
additionally specify the data type as a template parameter. Otherwise, the publisher is only aware of raw memory and 
the user has to take care that it is interpreted correctly.

Once it has offered its topic, it is able to publish (send) data of the specific type. Note that the default is to
have multiple publishers for the same topic (n:m communication). A compile-time option to restrict iceoryx to
1:n communication is available. Should 1:n communication be used RouDi checks for multiple publishers on the same 
topics and raises an error if there is more than one publisher for a topic.

## Subscriber

Symmetrically a subscriber also corresponds to a topic and thus needs a service description to be constructed. As for
publishers we distinguish between typed and untyped subscribers.

Once a subscriber is subscribed to some topic, it is able to receive data of the type tied to this topic. In the
untyped case this is raw memory and the user must take care that it is interpreted in a way that is compatible to the
data that was actually sent.

When multiple publishers have offered the same topic the subscriber will receive the data of all of them (but in
indeterminate order between different publishers).

## Waitset

The easiest way to receive data is to periodically poll whether data is available. This is sufficient for simple use
cases but inefficient in general, as it often leads to unnecessary latency and wake-ups without data.

The ``Waitset`` can be used to relinquish control (putting the thread to sleep) and wait for user-defined ``events``
to occur. Here an event is associated with a condition and occurs when this condition becomes true.
Usually, these events correspond to the availability of data at specific subscribers. This way we
can immediately wake up when data is available and will avoid unnecessary wake-ups if no data is available.

It manages a set of triggers which can be activated to indicate that a corresponding event occurred which wakes
up a potentially waiting thread. Upon waking up it can be determined which conditions became true and caused the
wake-up. In the case that the wake up event was the availability of new data, this data can now be collected at
the subscriber.

For more information on how to use the Waitset see 
[Waitset](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/waitset/README.md).

## Listener
part of #350

## API

The API is offered in two languages, C++ and C. Detailed information can be found in the 
[C++ example](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/icedelivery) and 
[C example](https://github.com/eclipse-iceoryx/iceoryx/blob/master/iceoryx_examples/icedelivery_in_c/README.md).

Many parts of the C++ API follow a functional programming approach which is less error-prone. This requires using 
the monadic types ``cxx::expected`` and ``cxx::optional`` which are introduced 
[here](http://iceoryx.io/advanced/usage-guide/how-optional-and-error-values-are-returned-in-iceoryx/).

We distinguish between the ``typed API`` and the ``untyped API``. In the typed API, the underlying data type is made
apparent by typed pointers or references to some data type T (often a template parameter). This allows working with
the data in a C++ idiomatic and type-safe way and should be preferred whenever possible. The typed API is mainly used 
when iceroryx is used stand-alone, i.e. not integrated into a third party framework.

The untyped API provides [opaque](https://en.wikipedia.org/wiki/Opaque_pointer) (i.e. void) pointers to data, which
is flexible and efficient but also requires that the user takes care to interpret received data correctly, i.e. as 
a type compatible to what was actually sent. This is required for interaction with other lower level APIs and 
integration into third party frameworks such as [ROS](https://www.ros.org/). For further information see the 
respective header files.

