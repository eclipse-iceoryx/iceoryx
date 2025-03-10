// Copyright (c) 2025 by Valour Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

//! [iceoryx includes]
#include "request_and_response_types.hpp"

#include "topic_data.hpp"

#include "iox/optional.hpp"
#include "iox/posh/experimental/node.hpp"
#include "iox/signal_watcher.hpp"

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-node-client";
volatile bool keepRunning = {true};

using WaitSet = iox::popo::WaitSet<>;
volatile WaitSet* waitsetSigHandlerAccess{nullptr};

//! [context data to store Fibonacci numbers and sequence ids]
struct ContextData
{
    uint64_t fibonacciLast = 0;
    uint64_t fibonacciCurrent = 1;
    int64_t requestSequenceId = 0;
    int64_t expectedResponseSequenceId = requestSequenceId;
};
//! [context data to store Fibonacci numbers and sequence ids]

static void signalHandler(int sig [[maybe_unused]])
{
    keepRunning = false;
    if (waitsetSigHandlerAccess)
    {
        waitsetSigHandlerAccess->markForDestruction();
    }
}

int main()
{
    auto sigTermGuard =
        iox::registerSignalHandler(iox::PosixSignal::TERM, signalHandler).expect("failed to register SIGTERM");
    auto sigIntGuard =
        iox::registerSignalHandler(iox::PosixSignal::INT, signalHandler).expect("failed to register SIGINT");

    //! [create the node]
    auto node_result = iox::posh::experimental::NodeBuilder(APP_NAME).domain_id_from_env_or_default().create();

    while (keepRunning && node_result.has_error())
    {
        std::cout << "Could not create the node!" << std::endl;

        node_result = iox::posh::experimental::NodeBuilder(APP_NAME)
                          .domain_id_from_env_or_default()
                          .roudi_registration_timeout(iox::units::Duration::fromSeconds(1))
                          .create();
    }

    auto node = std::move(node_result.value());
    //! [create the node]


    //! [create waitset]
    auto waitset = node.wait_set().create().expect("Getting a wait set");
    waitsetSigHandlerAccess = waitset.get();

    //! [create client]
    auto client = node.client({"Example", "Request-Response", "Add"})
                      .response_queue_capacity(2)
                      .create<AddRequest, AddResponse>()
                      .expect("Getting a client");
    //! [create client]

    // attach client to waitset
    waitset->attachState(*client.get(), iox::popo::ClientState::HAS_RESPONSE).or_else([](auto) {
        std::cerr << "failed to attach client" << std::endl;
        std::exit(EXIT_FAILURE);
    });
    //! [create waitset]

    ContextData ctx;

    //! [mainloop]
    while (keepRunning)
    {
        //! [send request]
        client->loan()
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
        //! [send request]


        // We block and wait for samples to arrive, when the time is up we send the request again
        //! [wait and check if the client triggered]
        auto notificationVector = waitset->timedWait(iox::units::Duration::fromSeconds(5));

        for (auto& notification : notificationVector)
        {
            if (notification->doesOriginateFrom(client.get()))
            {
                //! [take response]
                while (client->take().and_then([&](const auto& response) {
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
                //! [take response]
            }
        }
        //! [wait and check if the client triggered]
        constexpr std::chrono::milliseconds SLEEP_TIME{950U};
        std::this_thread::sleep_for(SLEEP_TIME);
    }
    //! [mainloop]

    std::cout << "shutting down" << std::endl;

    waitsetSigHandlerAccess = nullptr; // invalidate for signal handler

    return (EXIT_SUCCESS);
}
