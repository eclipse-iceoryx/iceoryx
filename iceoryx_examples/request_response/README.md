# request_response

## Introduction

This example demonstrates how to use iceoryx in a client-server architecture
using the request-response communication pattern.
The main feature is that the server only response data at the request of a client.

## Expected output

[![asciicast](https://asciinema.org/a/469913.svg)](https://asciinema.org/a/469913)

## Code walkthrough

### Client

At first, the includes for the client port and request-response types and runtime are needed.
<!-- [geoffrey] [geoffrey][iceoryx_examples/request_response/client_cxx_basic.cpp] [iceoryx includes] -->
```cpp
#include "request_and_response_types.hpp"

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/client.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

Next, the iceoryx runtime is initialized. With this call,
the application will be registered at `RouDi`, the routing and discovery daemon.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_basic.cpp] [initialize runtime] -->
```cpp
constexpr char APP_NAME[] = "iox-cpp-request-response-client-basic";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

After creating the runtime, the client port is created based on a ServiceDescription.
<!--[geoffrey][iceoryx_examples/request_response/client_cxx_basic.cpp][create client]-->
```cpp
iox::popo::Client<AddRequest, AddResponse> client({"Example", "Request-Response", "Add"});
```

The main goal of the client is to request from the server the sum of two numbers that the
client sends. When to sum is received from the server, the received sum is re-used to insert
it to the `addend` of the next request to send.
This calculates a Fibonacci sequence.
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_basic.cpp] [[send requests in a loop]] -->
```cpp
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    int64_t requestSequenceId = 0;
    int64_t expectedResponseSequenceId = requestSequenceId;
    while (!iox::posix::hasTerminationRequested())
    {
        // send request to server for sum up two numbers

        // the server polls with an interval of 100ms
        constexpr std::chrono::milliseconds DELAY_TIME{150U};
        std::this_thread::sleep_for(DELAY_TIME);

        // receive sum

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
<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_basic.cpp] [[send request]] -->
```cpp
    client.loan()
        .and_then([&](auto& request) {
            request.getRequestHeader().setSequenceId(requestSequenceId);
            expectedResponseSequenceId = requestSequenceId;
            requestSequenceId += 1;
            request->augend = fibonacciLast;
            request->addend = fibonacciCurrent;
            std::cout << APP_NAME << " Send Request: " << fibonacciLast << " + " << fibonacciCurrent << std::endl;
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

<!-- [geoffrey] [iceoryx_examples/request_response/client_cxx_basic.cpp] [[take response]] -->
```cpp
    while (client.take().and_then([&](const auto& response) {
        auto receivedSequenceId = response.getResponseHeader().getSequenceId();
        if (receivedSequenceId == expectedResponseSequenceId)
        {
            fibonacciLast = fibonacciCurrent;
            fibonacciCurrent = response->sum;
            std::cout << APP_NAME << " Got Response : " << fibonacciCurrent << std::endl;
        }
        else
        {
            std::cout << "Got Response with outdated sequence ID! Expected = " << expectedResponseSequenceId
                      << "; Actual = " << receivedSequenceId << "! -> skip" << std::endl;
        }
    }))
    {
    };
```

### Server

At first, the includes for the server port and request-response types and runtime are needed.
<!-- [geoffrey] [geoffrey][iceoryx_examples/request_response/server_cxx_basic.cpp] [iceoryx includes] -->
```cpp
#include "request_and_response_types.hpp"

#include "iceoryx_hoofs/posix_wrapper/signal_watcher.hpp"
#include "iceoryx_posh/popo/server.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
```

Next, the iceoryx runtime is initialized. With this call,
the application will be registered at `RouDi`, the routing and discovery daemon.
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_basic.cpp] [initialize runtime] -->
```cpp
constexpr char APP_NAME[] = "iox-cpp-user-header-publisher";
iox::runtime::PoshRuntime::initRuntime(APP_NAME);
```

After creating the runtime, the server port is created based on a ServiceDescription.
<!--[geoffrey][iceoryx_examples/request_response/server_cxx_basic.cpp][create server]-->
```cpp
iox::popo::Server<AddRequest, AddResponse> server({"Example", "Request-Response", "Add"});
```

In the main loop, the server receives the requests from the client with
`take()` and prepares with `loan()` a sample for the response.
The received two numbers `augend` and `addend` are added and transmitted
with the `send()` API.
<!-- [geoffrey] [iceoryx_examples/request_response/server_cxx_basic.cpp] [[process requests in a loop] [take request] [send response]] -->
```cpp
    while (!iox::posix::hasTerminationRequested())
    {
        server.take().and_then([&](const auto& request) {
            std::cout << APP_NAME << " Got Request: " << request->augend << " + " << request->addend << std::endl;

            server.loan(request)
                .and_then([&](auto& response) {
                    response->sum = request->augend + request->addend;
                    std::cout << APP_NAME << " Send Response: " << response->sum << std::endl;
                    response.send();
                })
                .or_else([&](auto& error) {
                    std::cout << APP_NAME
                              << "Could not allocate Response! Return value = " << static_cast<uint64_t>(error)
                              << std::endl;
                });
        });

        constexpr std::chrono::milliseconds SLEEP_TIME{100U};
        std::this_thread::sleep_for(SLEEP_TIME);
    }
```

<center>
[Check out request_response on GitHub :fontawesome-brands-github:](https://github.com/eclipse-iceoryx/iceoryx/tree/master/iceoryx_examples/request_response){ .md-button }
</center>
