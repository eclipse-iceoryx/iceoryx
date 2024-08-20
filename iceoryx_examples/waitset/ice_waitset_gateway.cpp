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
#include "iox/atomic.hpp"
#include "iox/signal_handler.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iostream>

constexpr uint64_t NUMBER_OF_SUBSCRIBERS = 2U;

iox::concurrent::Atomic<bool> keepRunning{true};

//! [waitset type alias]
using WaitSet = iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS>;
//! [waitset type alias]

volatile WaitSet* waitsetSigHandlerAccess{nullptr};

static void sigHandler(int f_sig [[maybe_unused]])
{
    keepRunning = false;
    if (waitsetSigHandlerAccess)
    {
        waitsetSigHandlerAccess->markForDestruction();
    }
}

// The callback of the event. Every callback must have an argument which is
// a pointer to the origin of the Trigger. In our case the event origin is
// the untyped subscriber.
//! [subscriber callback]
void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber, uint64_t* const sumOfAllSamples)
{
    while (subscriber->hasData())
    {
        subscriber->take().and_then([&](auto& userPayload) {
            auto chunkHeader = iox::mepoo::ChunkHeader::fromUserPayload(userPayload);
            auto flags = std::cout.flags();
            std::cout << "subscriber: " << std::hex << subscriber << " length: " << std::dec
                      << chunkHeader->userPayloadSize() << " ptr: " << std::hex << chunkHeader->userPayload()
                      << std::dec << std::endl;
            std::cout.setf(flags);
        });
        // no nullptr check required since it is guaranteed != nullptr
        ++(*sumOfAllSamples);
    }
}
//! [subscriber callback]

int main()
{
    // register sigHandler
    auto signalIntGuard =
        iox::registerSignalHandler(iox::PosixSignal::INT, sigHandler).expect("failed to register SIGINT");
    auto signalTermGuard =
        iox::registerSignalHandler(iox::PosixSignal::TERM, sigHandler).expect("failed to register SIGTERM");

    iox::runtime::PoshRuntime::initRuntime("iox-cpp-waitset-gateway");

    //! [create waitset]
    WaitSet waitset;
    waitsetSigHandlerAccess = &waitset;
    //! [create waitset]

    //! [configure]
    uint64_t sumOfAllSamples = 0U;

    // create subscribers and subscribe them to our service
    iox::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
    for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
        auto& subscriber = subscriberVector.back();

        /// important: the user has to ensure that the 'contextData' (here 'sumOfAllSamples') lives as long as
        ///            the subscriber with its callback when attached to the 'waitset'
        waitset
            .attachEvent(subscriber,
                         iox::popo::SubscriberEvent::DATA_RECEIVED,
                         0,
                         createNotificationCallback(subscriberCallback, sumOfAllSamples))
            .or_else([&](auto) {
                std::cerr << "failed to attach subscriber" << i << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }
    //! [configure]

    //! [event loop]
    while (keepRunning)
    {
        auto notificationVector = waitset.wait();

        for (auto& notification : notificationVector)
        {
            // call the callback which was assigned to the notification
            (*notification)();
        }

        auto flags = std::cout.flags();
        std::cout << "sum of all samples: " << std::dec << sumOfAllSamples << std::endl;
        std::cout.setf(flags);
    }
    //! [event loop]

    waitsetSigHandlerAccess = nullptr; // invalidate for signal handler

    return (EXIT_SUCCESS);
}
