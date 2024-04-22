# Request response

## Introduction

This example demonstrates how to use iceoryx in a client-server architecture
using the request-response communication pattern. The client sends a request with
two consecutive fibonacci numbers and the server responds with the next number in
the sequence.

We provide three examples, the very basic typed and untyped examples
and the most natural setup combining a server with a Listener and a client using
a WaitSet. Since you can find the general setup and functionality of the client
and the server also in the Listener/WaitSet example, we will describe only this
one.

## Expected output

[![asciicast](https://asciinema.org/a/476829.svg)](https://asciinema.org/a/476829)

## Code walkthrough

In the following scenario the client (client_cxx_waitset.cpp) uses the WaitSet to wait for a response from the server
(server_cxx_listener.cpp). The server uses the Listener API to take and process the requests from the client.

The client is inspired by the `iox-cpp-waitset-basic` example from the [WaitSet](../waitset)
example and the server from the `iox-cpp-callbacks-subscriber` in the [Listener](../callbacks)
example.

This is the most recommended way to create an efficient client-server combination with iceoryx.

### Client using WaitSet

At first, the includes for the client port, request-response types, WaitSet, and runtime are needed.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [iceoryx includes] -->
```cpp
#include "request_and_response_types.hpp"

#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_handler.hpp"
#include "iox/signal_watcher.hpp"
```

Afterwards we prepare some `ContextData` where we can store the Fibonacci numbers
and the sequence ids that we use to ensure the correct ordering of the responses.

<!--[geoffrey][iceoryx_examples/request_response/client_cxx_waitset.cpp][context data to store Fibonacci numbers and sequence ids]-->
```cpp
struct ContextData
{
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    int64_t requestSequenceId = 0;
    int64_t expectedResponseSequenceId = requestSequenceId;
};
```

Next, the iceoryx runtime is initialized. With this call,
the application will be registered at `RouDi`, the routing and discovery daemon.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [initialize runtime] -->
```cpp
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

After creating the runtime, the client is created and attached to the WaitSet.

The [options](../iceoptions) can be used to alter
the behavior of the client, like setting the response queue capacity or blocking behavior when the response queue is
full or the server is too slow. The `ClientOptions` are similar to `PublisherOptions`/`SubscriberOptions`.

<!--[geoffrey][iceoryx_examples/request_response/client_cxx_waitset.cpp][create waitset]-->
```cpp
WaitSet waitset;
waitsetSigHandlerAccess = &waitset;

iox::popo::ClientOptions options;
options.responseQueueCapacity = 2U;
iox::popo::Client<AddRequest, AddResponse> client({"Example", "Request-Response", "Add"}, options);

// attach client to waitset
waitset.attachState(client, iox::popo::ClientState::HAS_RESPONSE).or_else([](auto) {
    std::cerr << "failed to attach client" << std::endl;
    std::exit(EXIT_FAILURE);
});
```

Since the client requests the sum of two numbers from the server, we provide the
structs `AddRequest` and `AddResponse` as template parameters. When the sum is
received, it is re-used as the `addend` of the next request to send. This
calculates a Fibonacci sequence.

<!--[geoffrey][iceoryx_examples/request_response/request_and_response_types.hpp][request]-->
```cpp
struct AddRequest
{
    uint64_t augend{0};
    uint64_t addend{0};
};
```

<!--[geoffrey][iceoryx_examples/request_response/request_and_response_types.hpp][response]-->
```cpp
struct AddResponse
{
    uint64_t sum{0};
};
```

In the main loop, the client prepares first a request using the `loan()` API.
The request is a sample consisting of the two numbers `augend` and `addend` that
the server shall sum up. Additionally, the sample is marked with a sequence id
that is incremented before every send cycle to ensure a correct ordering of the
messages. The request is transmitted to the server via the `send()` API.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [[send request]] -->
```cpp
client.loan()
    .and_then([&](auto& request) {
        request.getRequestHeader().setSequenceId(ctx.requestSequenceId);
        ctx.expectedResponseSequenceId = ctx.requestSequenceId;
        ctx.requestSequenceId += 1;
        request->augend = ctx.fibonacciLast;
        request->addend = ctx.fibonacciCurrent;
        std::cout << APP_NAME << " Send Request: " << ctx.fibonacciLast << " + " << ctx.fibonacciCurrent
                  << std::endl;
        request.send().or_else(
            [&](auto& error) { std::cout << "Could not send Request! Error: " << error << std::endl; });
    })
    .or_else([](auto& error) { std::cout << "Could not allocate Request! Error: " << error << std::endl; });
```

Once the request has been sent, we block and wait for samples to arrive. Then we
iterate over the notification vector to check if we were triggered from our client:
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [[wait and check if the client triggered]] -->
```cpp
auto notificationVector = waitset.timedWait(iox::units::Duration::fromSeconds(5));

for (auto& notification : notificationVector)
{
    if (notification->doesOriginateFrom(&client))
    {
        // ...
    }
}
```

The client receives the responses from the server using `take()`
and extracts the sequence id with `response.getResponseHeader().getSequenceId()`.
When the server response comes in the correct order, the received sum is stored
in the `ContextData` struct `ctx` for usage in the next request.

<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [[take response]] -->
```cpp
while (client.take().and_then([&](const auto& response) {
    auto receivedSequenceId = response.getResponseHeader().getSequenceId();
    if (receivedSequenceId == ctx.expectedResponseSequenceId)
    {
        ctx.fibonacciLast = ctx.fibonacciCurrent;
        ctx.fibonacciCurrent = response->sum;
        std::cout << APP_NAME << " Got Response : " << ctx.fibonacciCurrent << std::endl;
    }
    else
    {
        std::cout << "Got Response with outdated sequence ID! Expected = "
                  << ctx.expectedResponseSequenceId << "; Actual = " << receivedSequenceId
                  << "! -> skip" << std::endl;
    }
}))
{
}
```

### Server using Listener

At first, the includes for the server port, Listener, request-response types and runtime are needed.
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp] [iceoryx includes] -->
```cpp
#include "request_and_response_types.hpp"

#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/signal_watcher.hpp"
```

Then a callback is created that shall be called when the server receives a request.
In this case the calculation and the sending of the response is done in the Listener callback.
If there are more resource-consuming tasks,
this could also be outsourced with a thread pool to handle the requests.

<!--[geoffrey][iceoryx_examples/request_response/server_cxx_listener.cpp][request callback]-->
```cpp
void onRequestReceived(iox::popo::Server<AddRequest, AddResponse>* server)
{
    while (server->take().and_then([&](const auto& request) {
        std::cout << APP_NAME << " Got Request: " << request->augend << " + " << request->addend << std::endl;

        server->loan(request)
            .and_then([&](auto& response) {
                response->sum = request->augend + request->addend;
                std::cout << APP_NAME << " Send Response: " << response->sum << std::endl;
                response.send().or_else(
                    [&](auto& error) { std::cout << "Could not send Response! Error: " << error << std::endl; });
            })
            .or_else([](auto& error) { std::cout << "Could not allocate Response! Error: " << error << std::endl; });
    }))
    {
    }
}
```

The server provides the `take()` method for receiving requests and the `loan()` and `send()` methods
for sending the responses with the sum of the two numbers.

Next, the iceoryx runtime is initialized.
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp] [initialize runtime] -->
```cpp
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

After creating the runtime, the server port is created based on a `ServiceDescription`. Similar to the client,
the `options` are used to alter the behavior of the server, like setting the request
queue capacity or blocking behavior when the request queue is full or the client is too slow.
<!--[geoffrey][iceoryx_examples/request_response/server_cxx_listener.cpp][create server]-->
```cpp
iox::popo::ServerOptions options;
options.requestQueueCapacity = 10U;
iox::popo::Server<AddRequest, AddResponse> server({"Example", "Request-Response", "Add"}, options);
```

Now we want to listen to an incoming server event and call the previously created callback
whenever a request has been received.
This is done with the following call:
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp][attach listener] -->
```cpp
listener
    .attachEvent(
        server, iox::popo::ServerEvent::REQUEST_RECEIVED, iox::popo::createNotificationCallback(onRequestReceived))
    .or_else([](auto) {
        std::cerr << "unable to attach server" << std::endl;
        std::exit(EXIT_FAILURE);
    });
```

With that the preparation is done and the main thread can just sleep or do other things:

<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp][wait for termination] -->
```cpp
iox::waitForTerminationRequest();
```

Once the user wants to shutdown the server, the server event is detached from the listener:
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp][cleanup] -->
```cpp
listener.detachEvent(server, iox::popo::ServerEvent::REQUEST_RECEIVED);
```

<center>
[Check out request_response on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/main/iceoryx_examples/request_response){ .md-button } <!--NOLINT github url required for website-->
</center>
