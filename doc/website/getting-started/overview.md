# Getting started with iceoryx

This document covers the core functionality of the ``iceoryx`` middleware and is intended to quickly get started to
set up iceoryx applications. It is no in-depth API documentation and while the API is still subject to changes, the
basic concepts presented here will still apply.

## General

To set up a collection of applications using iceoryx (an ``iceoryx system``), the applications need to initialize a
runtime and create ``publishers`` and ``subscribers``. The publishers send data of a specific ``topic`` which can be
received by subscribers of the same topic. To enable publishers to offer their topic and subscribers to subscribe to
these offered topics, the middleware daemon, called ``RouDi``, must be running.

For further information how iceoryx can be used see the [examples](../../../iceoryx_examples/README.md). The
[conceptual-guide](../../conceptual-guide.md) provides additional information about the ``Shared Memory
communication`` that lies at the heart of iceoryx.

We now briefly define the main entities of an iceoryx system before showing how they are created and used by the
iceoryx API.

### RouDi

RouDi is an abbreviation for **Rou**ting and **Di**scovery. RouDi takes care of the
communication setup but does not actually participate in the communication between the publisher and the subscriber.
RouDi can be thought of as the switchboard operator of iceoryx. One of his other major tasks is the setup of the
shared memory, which the applications use for exchanging payload data. Sometimes referred to as daemon, RouDi manages
the shared memory and is responsible for the service discovery, i.e. enabling subscribers to find topics offered by publishers. It also keeps track of all applications which have initialized a runtime and are hence able to use
publishers or subscribers. To view the available command line options call `iox-roudi --help`.

### Runtime

Each application which wants to use iceoryx has to instantiate its runtime, which essentially enables communication
with RouDi. Only one runtime object per user process is allowed.

To do so, the following lines of code are required

    #include "iceoryx_posh/runtime/posh_runtime.hpp"

    iox::runtime::PoshRuntime::initRuntime("some_unique_application_name");

### Service description

A ``ServiceDescription`` in iceoryx represents the data to be transmitted and is uniquely identified by three string identifiers.

1. ``Group`` name
2. ``Instance`` name
3. ``Topic`` name

A triple consisting of such strings is called a ``ServiceDescription``. In Autosar terminology these three
identifiers are called ``Service``, ``Instance`` and ``Event`` respectively.

Two ``ServiceDescription`` are considered matching if all these three strings are element-wise equal, i.e. group,
instance and topic names are the same for both of them.

This means the group and instance identifier can be ignored to create different ``ServiceDescription``s. They will be
needed for advanced filtering functionality in the future.

The data type of the transmitted data can be an arbitrary C++ class, struct or plain old data type.

### Publisher

A publisher is tied to a topic and needs a service description to be constructed. If it is typed one needs to
additionally specify the data type as a template parameter. Otherwise publisher is only aware of raw memory and the
user has to take care that it is interpreted correctly.

Once it has offered its topic, it is able to publish (send) data of the specific type. Note that the default is to
have multiple publishers for the same topic (n:m communication). A compile-time option to restrict iceoryx to
1:n communication is available.

### Subscriber

Symmetrically a subscriber also corresponds to a topic and thus needs a Service Description to be constructed. As for
publishers we distinguish between typed and untyped subscribers.

Once a subscriber is subscribed to some topic, it is able to receive data of the type tied to this topic. In the
untyped case this is raw memory and the user must take care that it is interpreted in a way that is compatible to the
data that was actually send.

When multiple publishers have offered the same topic the subscriber will receive the data of all of them (but in
indeterminate order between different publishers).

### Waitset

The easiest way to receive data is to periodically poll whether data is available. This is sufficient for simple use
cases but inefficient in general, as it often leads to unnecessary latency and wake-ups without data.

The ``Waitset`` can be used to relinquish control (putting the thread to sleep) and wait for user defined conditions
to become true. Usually these conditions correspond to the availability of data at specific subscribers. This way we
can immediately wake up when data is available and will avoid unnecessary wake-ups if no data is available.

It manages a set of triggers which can be activated to indicate that a corresponding condition became true which wakes
up a potentially waiting thread. Upon waking up it can be determined which conditions became true and caused the
wake-up. In the case that the wake up event was the availability of new data, this data can now be collected at
the subscriber.

For more information on how to use the Waitset see [Waitset](../../../iceoryx_examples/waitset/README.md).

## API

Now, we show how the API can be used to establish a publish-subscribe communication in an iceoryx system. Many parts
of the API follow a functional programming approach and allow the user to specify functions which handle the possible
cases, e.g. what should happen when data is received.

This is very flexible but requires using the monadic types ``cxx::expected`` and ``cxx::optional``, which we
introduce in the following sections.

We distinguish between the ``typed API`` and the ``untyped API``. In the Typed API the underlying data type is made
apparent by typed pointers or references to some data type T (often a template parameter). This allows working with
the data in an C++ idiomatic and type-safe way and should be preferred whenever possible.

The Untyped API provides [opaque](https://en.wikipedia.org/wiki/Opaque_pointer) (i.e. void) pointers to data, which
is flexible and efficient but also requires that the user takes care to interpret received data correctly, i.e. as a
type compatible to what was actually sent. This is required for interaction with other lower level APIs and should be
used sparingly. For further information see the respective header files.

There also is a plain [C API](../../../iceoryx_examples/icedelivery_on_c/README.md), which can be used if C++ is not
an option.

In the following sections we describe how to use the API in iceoryx applications. We will omit namespaces in several places to keep
the code concise. In most cases it can be assumed that we are using namespace ``iox::cxx``. We also will use ``auto``
sparingly to clearly show which types are involved, but in many cases automatic type deduction is possible and can
shorten the code.

### Optional

The type ``iox::cxx::optional<T>`` is used to indicate that there may or may not be a value of a specific type ``T``
available. This is essentially the maybe [monad](https://en.wikipedia.org/wiki/Monad_(functional_programming)) in functional programming. Assuming we have some optional (usually the
result of some computation)

```cpp
optional<int> result = someComputation();
```

we can check for its value using
```cpp
if(result.has_value())
{
    auto value = result.value();
    // do something with the value
}
else
{
    // handle the case that there is no value
}
```
A shorthand to get the value is 
```cpp
auto value = *result;
```

Note that accessing the value if there is no value is undefined behavior, so it must be checked beforehand.

We can achieve the same with the functional approach by providing a function for both cases.

```cpp
result.and_then([](int& value) { /*do something with the value*/ })
    .or_else([]() { /*handle the case that there is no value*/ });
```
Notice that we get the value by reference, so if a copy is desired it has to be created explicitly in the [lambda](https://en.wikipedia.org/wiki/Anonymous_function#C++_(since_C++11)) or
function we pass.

The optional can be be initialized from a value directly
```cpp
optional<int> result = 73;
result = 37;
```
If it is default initialized it is automatically set to its null value of type ``iox::cxx::nullopt_t``;
This can be also done directly by using the constant ``iox::cxx::nullopt``

```cpp
result = iox::cxx::nullopt;
```

For a complete list of available functions see [``optional.hpp``](../../../iceoryx_utils/include/iceoryx_utils/cxx/optional.hpp).

### Expected
``iox::cxx::expected<T, E>`` generalizes ``iox::cxx::optional`` by admitting a value of another type ``E`` instead of
no value at all, i.e. it contains either a value of type ``T`` or ``E``. In this way, ``expected`` is a special case of the either monad. It is usually used to pass a value of type ``T`` or an error that may have occurred, i.e. ``E`` is the
error type. For more information on how it is used for error handling see
[error-handling.md](../../design/error-handling.md).

Assume we have ``E`` as an error type, then we can create a value
```cpp
iox::cxx::expected<int, E> result(iox::cxx::success<int>(73));
```

and use the value or handle a potential error
```cpp
if (!result.has_error())
{
    auto value = result.value();
    // do something with the value
}
else
{
    auto error = result.get_error();
    // handle the error
}
```

Should we need an error value we set
```cpp
result = iox::cxx::error<E>(errorCode);
```
which assumes that E can be constructed from an ``errorCode``.

We again can employ a functional approach like this
```cpp
auto handleValue = [](int& value) { /*do something with the value*/ };
auto handleError = [](E& value) { /*handle the error*/ };
result.and_then(handleValue).or_else(handleError);
```

There are more convenience functions such as ``value_or`` which provides the value or an alternative specified by the
user. These can be found in [``expected.hpp``](../../../iceoryx_utils/include/iceoryx_utils/cxx/expected.hpp).


## Using the API

Armed with the terminology and functional concepts, we can start to use the API to send and receive data. This
involves settting up the runtime in each application, creating publishers in applications that need to send data and
subscribers in applications that want to receive said data. An application can have both, publishers and subscribers.
It can even send data to itself, but this usually makes little sense.

### Initialize the runtime

Create a runtime with a unique name among all applications for each application

```cpp
iox::runtime::PoshRuntime::initRuntime("some_unique_name");
```

Now this application is ready to communicate with the middleware daemon RouDi.

### Defining a topic

We need to define a data type we can send, which can be any struct or class or even a plain type, such as an int.

```cpp
struct CounterTopic
{
    CounterTopic(uint32_t counter = 0U)
        : counter(counter)
    {
    }

    uint32_t counter;
};
```

The topic type must be default- and copy-constructible when the typed API is used. Using the untyped API imposes no
such restriction, it just has to be possible to construct the data type at a given memory location.

### Typed API

We now demonstrate how to send and receive data with the typed API. 

#### Creating a publisher

We create a publisher that offers our CounterTopic.

```cpp
iox::popo::TypedPublisher<CounterTopic> publisher({"Group", "Instance", "CounterTopic"});
publisher.offer();
```

Note that it suffices to set the first two identifiers (Group and Instance) to some default values for all topics.
#### Sending data

Now we can use the publisher to send the data in various ways.

1. Loan and assign

    ```cpp
    auto result = publisher.loan();
    if (!result.has_error())
    {
        auto& sample = result.value();
        sample->counter = 73;
        sample.publish();
    }
    else
    {
        // handle the error
    }
    ```

    Here result is an ``expected`` and hence we may get an error which we have to handle. This can happen if we try
    to loan too many samples and exhaust memory.
    
    If we successfully get a sample, we can use ``operator->`` to access the underlying data and set it to the value
    we want to send. It is important to note that in the typed case we get a default constructed topic and such an
    access is legal.
    
    Once we are done constructing and preparing the data we publish it, causing it to be delivered to any subscriber
    which is currently subscribed to the topic.


2. Functional approach with loaning

    ```cpp
    publisher.loan()
        .and_then([&](auto& sample) {
            auto ptr = sample.get();
            *ptr = CounterTopic(73);
            sample.publish();
        })
        .or_else([](iox::popo::AllocationError) {
            /* handle the error */
        });   
    
    ```

    We try to loan a sample from the publisher and if successful get the underlying pointer ``ptr`` to our topic and
    if successful assign it a new value. Note that ``ptr`` points to an already default constructed sample, so we
    cannot treat it as uninitialized memory and therefore must assign the data to send.
    
    If you are only using a simple data type which does not rely on RAII, you can also use the pointer to construct
    the data via placement new instead.

    ```cpp
    new (ptr) CounterTopic(73);
    ```

3. Publish by copy

    ```cpp
    CounterTopic counter(73);
    publisher.publishCopyOf(counter);
    ```
    This copies the constructed ``counter`` object and hence should mostly be used for small data.

4. Publish the result of a computation

    ```cpp
    auto myComputation = [](CounterTopic* data) { *data = CounterTopic(73); };
    auto result = publisher.publishResultOf(myComputation);
    if (result.has_error())
    {
        // handle the error
    }    
    ```

    This can be used if we want to set the data by some callable (i.e. lambda, function or [functor](https://en.wikipedia.org/wiki/Function_object#In_C_and_C++)). As with all the
    other ways, it can fail when there is no memory for the sample availabe and this failure must be handled.

#### Creating a subscriber

We now create a corresponding subscriber, usually in another application. While it will work in the same application as
well, there usually is no need sending it via the middleware in such a case.

```cpp
iox::popo::TypedSubscriber<CounterTopic> subscriber({"Group", "Instance", "CounterTopic"});
subscriber.subscribe();
```
The template data type and the three string identifiers have to match those of the publisher, in other words the
service descriptions have to be the same (otherwise we will not receive data from our publisher).

We immediately subscribe here, but this can be postponed to the point where we actually want to receive data.

#### Receiving data
For simplicity we assume that we periodically check for new data. It is also possible to explicitly wait for data
using the [Waitset](../../../iceoryx_examples/waitset/README.md). The code to receive the data is the same, the only
difference is the way we wake up before checking for data.
```cpp
while (keepRunning)
{
    // wait for new data (either sleep and wake up periodically or by notification from the waitset)

    subscriber->take()
        .and_then([](iox::popo::Sample<const CounterTopic>& sample) {
            CounterTopic* ptr = sample.get();
            /* process the received data using the ptr */

            /* alternatively use operator-> */
            uint32_t counter = sample->counter;
        })
        .if_empty([] { /* no data received but also no error */ })
        .or_else([](iox::popo::ChunkReceiveError) { /* handle the error */ });
}

```

By calling ``take`` we get a ``iox::cxx::expected<iox::optional<iox::popo::Sample<const CounterTopic>>>``. Since this
may fail, we handle a potential error in the ``or_else`` branch. If we wake up periodically, it is also possible that
no data is received and if we want to handle this we can optionally do so in the ``if_empty`` branch.

The usual case is that we actually receive data, and we process it in the ``and_then``. Notice that in the lambda we
do not pass a ``CounterTopic`` directly but a reference to a ``iox::popo::Sample<const CounterTopic>&``. We can
access the underlying ``CounterTopic`` either by getting a pointer to it via ``get`` or by using ``operator->``. In
any case, we now can process or copy the received data and once the ``sample`` goes out of scope, the underlying
``CounterTopic`` object is deleted as well (this happens when the temporary object returned by ``take`` is
destroyed). This means it is only safe to hold references to the data as long as the ``sample`` exists. Should we
need a longer lifetime, we have to copy or move the data from the ``sample``.

This only allows us to get one sample at a time. Should we want to get all currently available samples we can do so
by using an additional loop.

```cpp
while (keepRunning)
{
    while (subscriber.hasNewSamples())
    {
        subscriber->take()
            .and_then([](iox::popo::Sample<const CounterTopic>& sample) {
                CounterTopic* ptr = sample.get();
                /* process the received data using the ptr */

                /* alternatively use operator-> */
                uint32_t counter = sample->counter;
            })
            .or_else([](iox::popo::ChunkReceiveError) { /* handle the error */ });
    }
    // wait for new samples (either sleep or use the waitset)
}
```

Here we do not check whether we actually have data since we already know there is data available by calling
``hasNewSamples``.

### Untyped API

The untyped API offeres similar capabilities and is hence usable in a similar way. The major difference is that
neither publisher nor subscriber have any knowledge about the underlying type they send or receive. This means that
the user is responsible to ensure the data is read correctly, i.e. there is no type safety guaranteed by the API
itself.

#### Creating a publisher

When creating an untyped publisher we do not need to specify a data type as template paraemter.

```cpp
iox::popo::UntypedPublisher publisher({"Group", "Instance", "CounterTopic"});
publisher.offer();
```

#### Sending data

1. Loan and emplace

    Before sending, we have to loan a chunk of memory to emplace our data.

    ```cpp
    auto result = publisher.loan(sizeof(CounterTopic));
    ```

    Since the data type is not known to the publisher, we have to provide the size in bytes of the payload data we
    intend to send.

    If we successfully acquired a chunk, we can construct the data to be send using placement new and publish it.

    ```cpp
    if (!result.has_error())
    {
        auto& sample = result.value();
        new (sample.get()) CounterTopic(73);
        sample.publish();
    }
    else
    {
        // could not acquire chunk, handle the error
    }    
    ```

    Here placement new is required, there is no preconstructed object at ``sample.get()``.

2. Functional approach with loaning

    ```cpp
    publisher.loan(sizeof(CounterTopic))
        .and_then([&](iox::popo::Sample<void>& sample) {
            new (sample.get()) CounterTopic(73);
            sample.publish();
        })
        .or_else([](iox::popo::AllocationError) {
            /* handle the error */
        });   
    ```
    Notice that we get an untyped sample, ``iox::popo::Sample<const void>``. We could also use ``auto& sample`` in the
    lambda arguments to shorten it. Again we access the pointer to the underlying raw memory of the sample and
    construct the data we want to send.



#### Creating a subscriber

While the string identifiers still have to match those in the publisher, as in the untyped publisher there is no
template type argument.

```cpp
iox::popo::UntypedSubscriber subscriber({"Group", "Instance", "CounterTopic"});
subscriber.subscribe();
```
#### Receiving data

Receiving the data works in the same way as in the typed API, the main difference is the ``reinterpret_cast`` needed
before accessing the data (since the subscriber has no knowledge of the underlying type).


```cpp
while (keepRunning)
{
    // wait for new data (either sleep and wake up periodically or by notification from the waitset)

    subscriber->take()
        .and_then([](iox::popo::Sample<const void>& sample) {
            CounterTopic* ptr = reinterpret_cast<CounterTopic*>(sample.get());
            /* process the received data using the ptr */
        })
        .if_empty([] { /* no data received but also no error */ })
        .or_else([](iox::popo::ChunkReceiveError) { /* handle the error */ });
}
```

Note that since the received sample received is untyped (``iox::popo::Sample<const void>``), we cannot use
``operator->`` to access the members of the underlying type but have to cast it to the correct type ``CounterTopic``
manually.

As in the untyped case we also could use a loop to get all samples as long as they are available.

### Shutdown

Once we are done sending, we call ``stopOffer`` at the publisher.

```cpp
publisher.stopOffer();
```

Similarly the subscriber can ``unsubscribe`` to stop receiving any data.

```cpp
subscriber.unsubscribe();
```

Both will also be called in the respective destructor if needed (i.e. if the publisher is still offering or the
subscriber is still subscribed).

### Running an iceoryx system

Now that we have applications capable of sending and receiving data, we can run the complete iceoryx system.

First we need to start RouDi.

    # If installed and available in PATH environment variable
    iox-roudi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/posh/iox-roudi

Afterwards we can start the applications which immediately connect to the RouDi via their runtime.

When the application terminates, the runtime cleans up all resources needed for communication with RouDi. This
includes all memory chunks used for the data transmission which may still be hold by the application.

## Examples

This covers the main use cases and should enable the user to quickly develop iceoryx applications.

Full examples and instructions on how to build and run them can be found in
[examples](../../../iceoryx_examples/README.md). The [icedelivery](../../../iceoryx_examples/icedelivery/README.md) example can be a starting point to further explore how iceoryx is used.

## C API

The C API usage is outlined in [icedelivery_on_c](../../../iceoryx_examples/icedelivery_on_c/README.md).

This API is still under much development and not fully supported yet.
