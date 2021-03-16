// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <iostream>

bool killswitch = false;
constexpr char APP_NAME[] = "iox-ex-subscriber-untyped";

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime(APP_NAME);

    // initialized subscriber
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueCapacity = 10U;
    iox::popo::UntypedSubscriber subscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);

    // run until interrupted by Ctrl-C
    while (!killswitch)
    {
        if (subscriber.getSubscriptionState() == iox::SubscribeState::SUBSCRIBED)
        {
            subscriber.take()
                .and_then([&](const void* data) {
                    auto object = static_cast<const RadarObject*>(data);
                    std::cout << APP_NAME << " got value: " << object->x << std::endl;

                    // note that we explicitly have to release the data
                    // and afterwards the pointer access is undefined behavior
                    subscriber.release(data);
                })
                .or_else([](auto& result) {
                    if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                    {
                        std::cout << "Error receiving chunk." << std::endl;
                    }
                });
        }
        else
        {
            std::cout << "Not subscribed!" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    return (EXIT_SUCCESS);
}
