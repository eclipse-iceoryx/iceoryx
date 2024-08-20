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

iox::concurrent::Atomic<bool> keepRunning{true};

constexpr uint64_t NUMBER_OF_SUBSCRIBERS = 4U;
using WaitSet = iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS>;

volatile WaitSet* waitsetSigHandlerAccess{nullptr};

static void sigHandler(int f_sig [[maybe_unused]])
{
    keepRunning = false;
    if (waitsetSigHandlerAccess)
    {
        waitsetSigHandlerAccess->markForDestruction();
    }
}

int main()
{
    // register sigHandler
    auto signalIntGuard =
        iox::registerSignalHandler(iox::PosixSignal::INT, sigHandler).expect("failed to register SIGINT");
    auto signalTermGuard =
        iox::registerSignalHandler(iox::PosixSignal::TERM, sigHandler).expect("failed to register SIGTERM");

    iox::runtime::PoshRuntime::initRuntime("iox-cpp-waitset-grouping");
    //! [create waitset]
    WaitSet waitset;
    waitsetSigHandlerAccess = &waitset;
    //! [create waitset]

    // create subscriber and subscribe them to our service
    //! [create subscribers]
    iox::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
    for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
    }
    //! [create subscribers]

    constexpr uint64_t FIRST_GROUP_ID = 123U;
    constexpr uint64_t SECOND_GROUP_ID = 456U;

    //! [configure subscribers]
    // attach the first two subscribers to waitset with a id of FIRST_GROUP_ID
    for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS / 2; ++i)
    {
        waitset.attachState(subscriberVector[i], iox::popo::SubscriberState::HAS_DATA, FIRST_GROUP_ID)
            .or_else([&](auto) {
                std::cerr << "failed to attach subscriber" << i << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }

    // attach the remaining subscribers to waitset with a id of SECOND_GROUP_ID
    for (auto i = NUMBER_OF_SUBSCRIBERS / 2; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        waitset.attachState(subscriberVector[i], iox::popo::SubscriberState::HAS_DATA, SECOND_GROUP_ID)
            .or_else([&](auto) {
                std::cerr << "failed to attach subscriber" << i << std::endl;
                std::exit(EXIT_FAILURE);
            });
    }
    //! [configure subscribers]

    //! [event loop]
    while (keepRunning)
    {
        auto notificationVector = waitset.wait();

        for (auto& notification : notificationVector)
        {
            //! [data path]
            // we print the received data for the first group
            if (notification->getNotificationId() == FIRST_GROUP_ID)
            {
                auto subscriber = notification->getOrigin<iox::popo::UntypedSubscriber>();
                subscriber->take().and_then([&](auto& userPayload) {
                    const CounterTopic* data = static_cast<const CounterTopic*>(userPayload);
                    auto flags = std::cout.flags();
                    std::cout << "received: " << std::dec << data->counter << std::endl;
                    std::cout.setf(flags);
                    subscriber->release(userPayload);
                });
            }
            // dismiss the received data for the second group
            else if (notification->getNotificationId() == SECOND_GROUP_ID)
            {
                std::cout << "dismiss data\n";
                auto subscriber = notification->getOrigin<iox::popo::UntypedSubscriber>();
                // We need to release the data to reset the trigger hasData
                // otherwise the WaitSet would notify us in 'waitset.wait()' again
                // instantly.
                subscriber->releaseQueuedData();
            }
            //! [data path]
        }

        std::cout << std::endl;
    }
    //! [event loop]

    waitsetSigHandlerAccess = nullptr; // invalidate for signal handler

    return (EXIT_SUCCESS);
}
