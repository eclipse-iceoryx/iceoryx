# Getting started with iceoryx

This document covers the core functionality of the ``iceoryx`` middleware and is intended to quickly get started to set up iceoryx applications. It is no in-depth API documentation and while the API is still subject to changes, the basic concepts will still apply.  
## General

To set up a collection of applications using iceoryx (an iceoryx system), the applications need to initialize a runtime and create ``publishers`` and ``subscribers``. The publishers send data of a specific ``topic`` which can be received by subscribers of the same topic.
To enable publishers to offer their topic and subscribers to subscribe to these offered topics, the middleware daemon, called ``Roudi``, must be running. 

For further information see the [examples](todo) and [conceptual-guide.md](todo). We now briefly define the main entities of an iceoryx system before showing how they are created and used by the iceoryx API.



### Roudi

The middleware daemon manages the shared memory and is responsible for the service discovery, i.e. enabling subscribers to find topics offered by publishers. It also keeps track of all applications which have initialized a runtime and are hence able to use publishers or subscribers.

It can be started like this

    # If installed and available in PATH environment variable
    iox-roudi
    # If build from scratch with script in tools
    $ICEORYX_ROOT/build/posh/iox-roudi

### Runtime

Each application which wants to use iceoryx has to instantiate its runtime, which essentially enables communication with Roudi.

To do so, the following lines of code are required
 
    #include "iceoryx_posh/runtime/posh_runtime.hpp"

    iox::runtime::PoshRuntime::initRuntime("/some_unique_application_name");

The application name must be unique among all other applications and have a leading `\`.


### Topics

A topic in iceoryx specifies some kind of data and is uniquely indetified by three string identifiers.

1. ``Group`` name
2. ``Instance`` name
3. ``Topic`` name

A triple consisting of such strings is called a ``Service Description``.

Two topics are considered matching if all these three strings are element-wise equal, i.e. group, instance and topic names are the same for both of them.

This means the group and instance identifier can be ignored to create different topics. They will be needed for advanced filtering functionality in the future.

The data type of the topic can be an arbitrary C++ class or struct (which supports copy behavior).

TODO: Add Autosar terminology mapping.

### Publisher
A publisher is tied to a topic and needs a Service Description to be constructed. If it is typed one needs to additionally specify the data type
as a template parameter. Otherwise publisher is only aware of raw memory and the user has to take care that it is interpreted correctly.

Once it has offered its topic, it is able to publish (send) data of the specific type. Note that it is possible to have multiple publishers for the same topic.

### Subscriber
Symmetrically a subscriber also corresponds to a topic and thus needs a Service Description to be constructed. As for publishers we distinguish between typed and untyped subscribers.

Once a subscriber is subscribed to some topic, it is able to receive data of the type tied to this topic. In the untyped case this is raw memory and the user must take care that it is interpreted in a way that is compatible to the data that was actually send.

When multiple publishers have offered the same topic the subscriber will receive the data of all of them (but in indeterminate order between different publishers).

### Waitset
The easiest way to receie data is to periodically poll whether data is available. This is sufficient for simple use cases but inefficient in general, as it often leads to unnecessary latency and wake-ups without data.

The ``Waitset`` can be used to relinquish control (putting the thread to sleep) and wait for user defined conditions to become true. 
Usually these conditions correspond to the availability of data at specific subscribers. This way we can (almost) immediately wake up when data is available and will avoid unnecessary wake-ups if no data is available.

To do so it manages a set of triggers which can be activated and indicate that a corresponding condition became true which in turn will wake up a potentially waiting thread. In this way it extends a condition variable to a collection of conditions. Upon waking up it can be determined which conditions became true and caused
the wake up. In the case that the wake up event was the availability of new data, this data can now be collected at the subscriber.

For more information on how to use the Waitset see [Waitset](todo_link).


## API

We now show how the API can be used to establish a publish-subscribe communication in an iceoryx system. Many parts of the API follow a functional programming approach and allow the user to specify functions which handle the possible cases, e.g. what should happen when data is received.

This is very flexible but requires using the monadic types ``cxx::expected`` and ``cxx::optional``, which we introduce in the following sections.

We distinguish between the ``Typed API`` and the ``Untyped API``. In the Typed API the underlying data type is made apparent by typed pointers or references to some data type T (often a template parameter). this allows working with the data in an C++ idiomatic and type-safe way and should be preferred whenever possible.

The Untyped API provides opaque (i.e. void) pointers to data, which is flexible and efficient but also requires that the user takes care to interpret received data correctly, i.e. as a type compatible to what was actually sent. This is required for interaction with other lower level APIs and should be used sparingly.
For further information see the respective header files.

There also is a plain [C API](todo), which can be used if C++ is not an option.

We now describe the how to use the API in iceoryx applications. We will ommit namespaces in several places to keep the code concise. In most cases it can be assumed that we are using namespace ``iox::cxx``. We also will use ``auto`` sparingly to clearly show which types are involved, but in many cases automatic type deduction is possible and can shorten the code.

### Optional

The type ``cxx::optional<T>`` is used to indicate that there may or may not be a value of a specific type ``T`` available. This is essentially the maybe monad in functional programming. Assuming we have some optional (usually the result of some computation)
```
optional<int> result = someComputation();
```
we can check for its value using
```
if(result.has_value()) {
    auto value = result.value();
    //do something with the value
} else {
    //handle the case that there is no value
}
```
A shorthand to get the value is 
```
auto value = *result;
```

Note that getting the value if there is no value is undefined behavior, so it must be checked beforehand.

We can achieve the same with the functional approach by providing a function for both cases.

```
result.and_then([](int& value) { /*do something with the value*/ })
      .or_else([]() { /*handle the case that there is no value*/ });
```
Notice that we get the value by reference, so if a copy is desired it has to be created explicitly in th e lambda or function we pass.

The optional can be be initialized from a value directly
```
optional<int> result = 73;
result = 37;
```
If it is default initialized it is automatically set to its null value of type ``iox::cxx::nullopt_t``;
This can be also done directly by using the constant ``iox::cxx::nullopt``

```
result = nullopt;
```

### Expected
``cxx::expected<T, E>`` generalizes ``cxx::optional`` by admitting a value of another type ``T`` instead of no value at all, i.e. it contains either a value of type ``T`` or ``E`` (roughly the either monad). This is usually used to pass a value of type or an error that may have occurred, i.e. ``E`` is the error type. For more information on how it is used for error handling see [error-handling.md](todo).

Assume we have ``E`` as an error type, then we can create a value
```
cxx::expected<int, E> result(iox::cxx::success<int>(73));
```

and use the value or handle a potential error
```
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
```
result = iox::cxx::error<E>(errorCode);
```
which assumes that E can be constructed from an ``errorCode``.

We again can employ a functional approach like this
```
auto handleValue = [](int& value) { /*do something with the value*/ };
auto handleError = [](E& value) { /*handle the error*/ };
result.and_then(handleValue).or_else(handleError);
```

There are more convenience functions such as ``value_or`` which provides the value or an alternative specified by the user. These can be found in ``expected.hpp``.


## Using the API

### Initializing the runtime


### Creating a publisher


### Sending data

### Creating a subscriber


### receiving data


## Examples

Examples of the main use cases and instructions on how to build and run them can be found in [examples](../iceoryx_examples/README.md).
