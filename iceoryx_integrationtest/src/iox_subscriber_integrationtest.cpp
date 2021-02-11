// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/platform/signal.hpp"

#include <iostream>

std::atomic_bool killSwitch{false};

static void sigHandler(int32_t signal [[gnu::unused]])
{
    killSwitch.store(true);
}

void registerSigHandler()
{
    // register sigHandler for SIGINT, SIGTERM and SIGHUP
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_handler = sigHandler;
    act.sa_flags = 0;

    if (iox::cxx::makeSmartC(sigaction, iox::cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGINT, &act, nullptr)
            .hasErrors())
    {
        std::cerr << "Calling sigaction() for SIGINT failed" << std::endl;
    }

    if (iox::cxx::makeSmartC(sigaction, iox::cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGTERM, &act, nullptr)
            .hasErrors())
    {
        std::cerr << "Calling sigaction() for SIGTERM failed" << std::endl;
    }

    if (iox::cxx::makeSmartC(sigaction, iox::cxx::ReturnMode::PRE_DEFINED_SUCCESS_CODE, {0}, {}, SIGHUP, &act, nullptr)
            .hasErrors())
    {
        std::cerr << "Calling sigaction() for SIGHUP failed" << std::endl;
    }
}

int main()
{
    registerSigHandler();

    std::cout << "Application iox_subscriber_integrationtest started" << std::endl;

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime("iox_subscriber_integrationtest");

    // initialized subscriber
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueCapacity = 10U;
    iox::popo::TypedSubscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);
    subscriber.subscribe();

    // run until interrupted by Ctrl-C
    while (!killSwitch.load())
    {
        if (subscriber.getSubscriptionState() == iox::SubscribeState::SUBSCRIBED)
        {
            std::cout << "iox-ex-subscriber-typed subscribed" << std::endl;
            subscriber.take_1_0()
                .and_then([](auto& sample) { std::cout << "Got value: " << sample->x << std::endl; })
                .or_else([](auto& result) {
                    // only has to be called if the alternative is of interest,
                    // i.e. if nothing has to happen when no data is received and
                    // a possible error alternative is not checked or_else is not needed
                    if (result != iox::popo::ChunkReceiveResult::NO_CHUNK_AVAILABLE)
                    {
                        std::cerr << "Error receiving chunk." << std::endl;
                    }
                });
        }
        else
        {
            std::cout << "iox-ex-subscriber-typed not subscribed!" << std::endl;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    subscriber.unsubscribe();

    return (EXIT_SUCCESS);
}
