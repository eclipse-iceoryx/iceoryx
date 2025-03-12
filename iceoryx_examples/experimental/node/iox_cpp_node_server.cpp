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

#include "iox/optional.hpp"
#include "iox/posh/experimental/node.hpp"
#include "iox/signal_watcher.hpp"
//! [iceoryx includes]

#include <iostream>

constexpr char APP_NAME[] = "iox-cpp-node-server";
volatile bool keepRunning = {true};

static void signalHandler(int sig [[maybe_unused]])
{
    keepRunning = false;
}

//! [request callback]
void onRequestReceived(iox::popo::Server<AddRequest, AddResponse>* server)
{
    //! [take request]
    while (server->take().and_then([&](const auto& request) {
        std::cout << APP_NAME << " Got Request: " << request->augend << " + " << request->addend << std::endl;

        //! [send response]
        server->loan(request)
            .and_then([&](auto& response) {
                response->sum = request->augend + request->addend;
                std::cout << APP_NAME << " Send Response: " << response->sum << std::endl;
                response.send().or_else(
                    [&](auto& error) { std::cout << "Could not send Response! Error: " << error << std::endl; });
            })
            .or_else([](auto& error) { std::cout << "Could not allocate Response! Error: " << error << std::endl; });
        //! [send response]
    }))
    {
    }
    //! [take request]
}
//! [request callback]

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

    //! [create server]
    auto server = node.server({"Example", "Request-Response", "Add"})
                      .request_queue_capacity(10u)
                      .create<AddRequest, AddResponse>()
                      .expect("Getting a listener");
    //! [create server]

    //! [create listener]
    auto listener = node.listener().create().expect("Getting a listener");
    //! [create listener]

    //! [attach listener]
    listener
        ->attachEvent(*server.get(),
                      iox::popo::ServerEvent::REQUEST_RECEIVED,
                      iox::popo::createNotificationCallback(onRequestReceived))
        .or_else([](auto) {
            std::cerr << "unable to attach server" << std::endl;
            std::exit(EXIT_FAILURE);
        });
    //! [attach listener]

    //! [wait for termination]
    iox::waitForTerminationRequest();
    //! [wait for termination]

    //! [cleanup]
    listener->detachEvent(*server.get(), iox::popo::ServerEvent::REQUEST_RECEIVED);
    //! [cleanup]

    return EXIT_SUCCESS;
}
