// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "topic_data.hpp"

#include "iox/atomic.hpp"
#include "iox/optional.hpp"
#include "iox/posh/experimental/node.hpp"
#include "iox/signal_handler.hpp"

#include <iostream>

iox::concurrent::Atomic<bool> keep_running{true};

using WaitSet = iox::popo::WaitSet<>;
volatile WaitSet* waitsetSigHandlerAccess{nullptr};

static void sigHandler(int sig [[maybe_unused]])
{
    keep_running = false;
    if (waitsetSigHandlerAccess)
    {
        waitsetSigHandlerAccess->markForDestruction();
    }
}

int main()
{
    iox::log::Logger::init(iox::log::logLevelFromEnvOr(iox::log::LogLevel::INFO));

    auto signalIntGuard =
        iox::registerSignalHandler(iox::PosixSignal::INT, sigHandler).expect("failed to register SIGINT");
    auto signalTermGuard =
        iox::registerSignalHandler(iox::PosixSignal::TERM, sigHandler).expect("failed to register SIGTERM");

    constexpr char APP_NAME[] = "iox-cpp-node-subscriber";
    auto node_result = iox::posh::experimental::NodeBuilder(APP_NAME).domain_id_from_env_or_default().create();

    while (keep_running && node_result.has_error())
    {
        std::cout << "Could not create the node!" << std::endl;

        node_result = iox::posh::experimental::NodeBuilder(APP_NAME)
                          .domain_id_from_env_or_default()
                          .roudi_registration_timeout(iox::units::Duration::fromSeconds(1))
                          .create();
    }

    if (!keep_running)
    {
        return EXIT_SUCCESS;
    }

    auto node = std::move(node_result.value());

    auto ws = node.wait_set().create().expect("Getting a wait set");
    waitsetSigHandlerAccess = ws.get();

    auto subscriber =
        node.subscriber({"Radar", "FrontLeft", "Object"}).create<RadarObject>().expect("Getting a subscriber");

    ws->attachState(*subscriber.get(), iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
        std::cout << "Failed to attach subscriber" << std::endl;
        std::exit(EXIT_FAILURE);
    });

    while (keep_running)
    {
        auto notification_vector = ws->wait();

        for (auto& notification : notification_vector)
        {
            if (notification->doesOriginateFrom(subscriber.get()))
            {
                subscriber->take()
                    .and_then([](const auto& sample) { std::cout << "Receive value: " << sample->x << std::endl; })
                    .or_else([](auto& result) {
                        if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                        {
                            std::cout << "Error receiving chunk." << std::endl;
                        }
                    });
            }
        }
    }

    waitsetSigHandlerAccess = nullptr; // invalidate for signal handler

    return (EXIT_SUCCESS);
}
