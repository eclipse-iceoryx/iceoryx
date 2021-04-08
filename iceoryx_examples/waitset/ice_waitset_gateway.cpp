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

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iostream>

iox::popo::UserTrigger shutdownTrigger;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownTrigger.trigger();
}

// The callback of the event. Every callback must have an argument which is
// a pointer to the origin of the Trigger. In our case the event origin is
// the untyped subscriber.
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber)
{
    while (subscriber->hasData())
    {
        subscriber->take().and_then([&](auto& userPayloadOfChunk) {
            auto chunkHeader = iox::mepoo::ChunkHeader::fromUserPayload(userPayloadOfChunk);
            std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                      << chunkHeader->userPayloadSize() << " ptr: " << std::hex << chunkHeader->userPayload()
                      << std::endl;
        });
    }
}

int main()
{
    constexpr uint64_t NUMBER_OF_SUBSCRIBERS = 4U;
    constexpr uint64_t ONE_SHUTDOWN_TRIGGER = 1U;

    // register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime("iox-ex-waitset-gateway");

    iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS + ONE_SHUTDOWN_TRIGGER> waitset;

    // attach shutdownTrigger to handle CTRL+C
    waitset.attachEvent(shutdownTrigger).or_else([](auto) {
        std::cerr << "failed to attach shutdown trigger" << std::endl;
        std::terminate();
    });

    // create subscriber and subscribe them to our service
    iox::cxx::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
    for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
        auto& subscriber = subscriberVector.back();

        waitset.attachEvent(subscriber, iox::popo::SubscriberEvent::DATA_RECEIVED, 0, &subscriberCallback)
            .or_else([&](auto) {
                std::cerr << "failed to attach subscriber" << i << std::endl;
                std::terminate();
            });
    }

    // event loop
    while (true)
    {
        auto eventVector = waitset.wait();

        for (auto& event : eventVector)
        {
            if (event->doesOriginateFrom(&shutdownTrigger))
            {
                // CTRL+c was pressed -> exit
                return (EXIT_SUCCESS);
            }
            else
            {
                // call the callback which was assigned to the event
                (*event)();
            }
        }

        std::cout << std::endl;
    }

    return (EXIT_SUCCESS);
}
