// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/untyped_subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::UserTrigger shutdownTrigger;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownTrigger.trigger();
}

int main()
{
    constexpr uint64_t NUMBER_OF_SUBSCRIBERS = 4U;
    constexpr uint64_t ONE_SHUTDOWN_TRIGGER = 1U;

    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::initRuntime("iox-ex-waitset-grouping");
    iox::popo::WaitSet<NUMBER_OF_SUBSCRIBERS + ONE_SHUTDOWN_TRIGGER> waitset;

    // attach shutdownTrigger to handle CTRL+C
    waitset.attachEvent(shutdownTrigger);

    // create subscriber and subscribe them to our service
    iox::cxx::vector<iox::popo::UntypedSubscriber, NUMBER_OF_SUBSCRIBERS> subscriberVector;
    for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
        auto& subscriber = subscriberVector.back();

        subscriber.subscribe();
    }

    constexpr uint64_t FIRST_GROUP_ID = 123U;
    constexpr uint64_t SECOND_GROUP_ID = 456U;

    // attach the first two subscribers to waitset with a eventid of FIRST_GROUP_ID
    for (auto i = 0U; i < NUMBER_OF_SUBSCRIBERS / 2; ++i)
    {
        waitset.attachEvent(subscriberVector[i], iox::popo::SubscriberEvent::HAS_SAMPLES, FIRST_GROUP_ID);
    }

    // attach the remaining subscribers to waitset with a eventid of SECOND_GROUP_ID
    for (auto i = NUMBER_OF_SUBSCRIBERS / 2; i < NUMBER_OF_SUBSCRIBERS; ++i)
    {
        waitset.attachEvent(subscriberVector[i], iox::popo::SubscriberEvent::HAS_SAMPLES, SECOND_GROUP_ID);
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
            // we print the received data for the first group
            else if (event->getEventId() == FIRST_GROUP_ID)
            {
                auto subscriber = event->getOrigin<iox::popo::UntypedSubscriber>();
                subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
                    const CounterTopic* data = static_cast<const CounterTopic*>(sample.get());
                    std::cout << "received: " << std::dec << data->counter << std::endl;
                });
            }
            // dismiss the received data for the second group
            else if (event->getEventId() == SECOND_GROUP_ID)
            {
                std::cout << "dismiss data\n";
                auto subscriber = event->getOrigin<iox::popo::UntypedSubscriber>();
                // We need to release the samples to reset the trigger hasSamples
                // otherwise the WaitSet would notify us in `waitset.wait()` again
                // instantly.
                subscriber->releaseQueuedSamples();
            }
        }

        std::cout << std::endl;
    }

    return (EXIT_SUCCESS);
}
