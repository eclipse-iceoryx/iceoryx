# request_response

## Introduction

This example demonstrates how to use iceoryx in a client-server architecture
using the request-response communication pattern.

## Expected output basic server-client example

[![asciicast](https://asciinema.org/a/469913.svg)](https://asciinema.org/a/469913)

## Code walkthrough

In the following scenario the client (client_cxx_waitset.cpp) is using the `Waitset` to wait for a response from the server
(server_cxx_listener.cpp) that uses the `Listener` API for taking and processing the requests.

The client is inspired by the `iox-cpp-waitset-basic` example from the `waitset` folder and the server from the
`iox-cpp-callbacks-subscriber` example in the `callbacks` folder.

This is the most recommended way to create an efficient server-client combination with iceoryx.

### Client using Waitset

At first, the includes for the client port, request-response types, WaitSet, and runtime are needed.
<!-- [geoffrey] [geoffrey][iceoryx_examples/request_response/client_cxx_waitset.cpp] [iceoryx includes] -->
```cpp
#include "request_and_response_types.hpp"

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

Next, the iceoryx runtime is initialized. With this call,
the application will be registered at `RouDi`, the routing and discovery daemon.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [initialize runtime] -->
```cpp
iox::runtime::PoshRuntime::initRuntime("iox-cpp-waitset-basic");
```

After creating the runtime, the client port is created and attached to the Waitset.

<!--[geoffrey][iceoryx_examples/request_response/client_cxx_waitset.cpp][create client]-->
```cpp
iox::popo::Client<AddRequest, AddResponse> client({"Example", "Request-Response", "Add"});
```

<!--[geoffrey][iceoryx_examples/request_response/client_cxx_waitset.cpp][create waitset]-->
```cpp
waitset.emplace();
 waitset->attachState(client, iox::popo::ClientState::HAS_RESPONSE).or_else([](auto) {
     std::cerr << "failed to attach client" << std::endl;
     std::exit(EXIT_FAILURE);
 });
```

The main goal of the client is to request from the server the sum of two numbers that the
client sends. When to sum is received from the server, the received sum is re-used to insert
it to the `addend` of the next request to send.
This calculates a Fibonacci sequence.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_waitset.cpp] [[mainloop]] -->
```cpp
    while (!iox::posix::hasTerminationRequested())
    {
        // send request to server for sum up two numbers

        // We block and wait for samples to arrive, when the ime is up we send the request again
        auto notificationVector = waitset.timedWait(iox::units::Duration::fromSeconds(5));

        // when response received, take the sample and process it

        constexpr std::chrono::milliseconds SLEEP_TIME{950U};
        std::this_thread::sleep_for(SLEEP_TIME);
    }
```

In the main loop, the client prepares a request using the `loan()` API.
The request is a sample consisting of two numbers `augend` and `addend` that the server shall sum up.
Additionally, the sample is marked with a sequence id that is incremented before
every send cycle to ensure a correct ordering of the messages
(`request.getRequestHeader().setSequenceId()`).
The request is transmitted to the server via the `send()` API.
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
            request.send();
        })
        .or_else([](auto& error) {
            std::cout << "Could not allocate Request! Return value = " << static_cast<uint64_t>(error) << std::endl;
        });
```

In the same while loop the client receives the responses from the server using `take()`
and extract the sequence id with `response.getResponseHeader().getSequenceId()`.
When the server response comes in the correct order, the received sum is re-used to
insert it to the `addend` of the next request to send.

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

At first, the includes for the server port and request-response types and runtime are needed.
<!-- [geoffrey] [geoffrey][iceoryx_examples/request_response/server_cxx_listener.cpp] [iceoryx includes] -->
```cpp
#include "request_and_response_types.hpp"

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_posh/popo/notification_callback.hpp"
#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

First, a callback is created that shall be called when the server receives a response.
In this case the calculation and the sending of the response is done in the listener callback.
If there are more resource-consuming tasks,
this could also be outsourced with a thread pool to handle the requests

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
                response.send();
            })
            .or_else([](auto& error) {
                std::cout << "Could not allocate Response! Return value = " << static_cast<uint64_t>(error)
                          << std::endl;
            });
    }))
    {
    }
}
```

Maybe: The server provides the `take()` method for receiving requests and the `loan()` and `send()` methods
for sending the responses with the sum of the two numbers.

Next, the iceoryx runtime is initialized.
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp] [initialize runtime] -->
```cpp
constexpr char APP_NAME[] = "iox-cpp-request-response-server-basic;
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

After creating the runtime, the server port is created based on a ServiceDescription.
<!--[geoffrey][iceoryx_examples/request_response/server_cxx_listener.cpp][create server]-->
```cpp
iox::popo::Server<AddRequest, AddResponse> server({"Example", "Request-Response", "Add"});
```

Now we want to listen to an incoming server event and want to fire the previously created callback.
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
iox::posix::waitForTerminationRequest();
```

Once the user wants to shutdown the server, the server event is detached from the listener:
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_listener.cpp][wait for termination] -->
```cpp
listener.detachEvent(server, iox::popo::ServerEvent::REQUEST_RECEIVED);
```

<center>
[Check out request_response on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/request_response){ .md-button }
</center>
