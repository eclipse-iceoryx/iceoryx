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

#include "iceoryx_posh/popo/modern_api/untyped_subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::UserTrigger shutdownGuard;
using Subscriber = iox::popo::UntypedSubscriber;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownGuard.trigger();
}

void subscriberCallback(iox::popo::UntypedSubscriber* const subscriber)
{
    subscriber->take().and_then([&](iox::popo::Sample<const void>& sample) {
        const CounterTopic* data = reinterpret_cast<const CounterTopic*>(sample.get());
        std::cout << "subscriber: " << std::hex << subscriber << " received: " << std::dec << data->counter
                  << std::endl;
    });
}

void receiving()
{
    constexpr uint64_t FIRST_GROUP_ID = 123;
    constexpr uint64_t SECOND_GROUP_ID = 456;

    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-waitset");
    iox::popo::WaitSet waitset;

    iox::cxx::vector<iox::popo::UntypedSubscriber, 4> subscriberVector;

    for (auto i = 0; i < 4; ++i)
    {
        subscriberVector.emplace_back(iox::capro::ServiceDescription{"Radar", "FrontLeft", "Counter"});
        auto& subscriber = subscriberVector.back();

        subscriber.subscribe();
    }

    for (auto i = 0; i < 2; ++i)
        subscriberVector[i].attachToWaitset(
            waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, FIRST_GROUP_ID, subscriberCallback);

    for (auto i = 2; i < 4; ++i)
        subscriberVector[i].attachToWaitset(
            waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES, SECOND_GROUP_ID, subscriberCallback);

    shutdownGuard.attachToWaitset(waitset);

    while (true)
    {
        auto triggerVector = waitset.wait();

        for (auto& trigger : triggerVector)
        {
            if (trigger.doesOriginateFrom(&shutdownGuard))
            {
                return;
            }
            else if (trigger.getTriggerId() == FIRST_GROUP_ID)
            {
                std::cout << "First group element\n";
                trigger();
            }
            else if (trigger.getTriggerId() == SECOND_GROUP_ID)
            {
                std::cout << "Second group element\n";
                trigger();
            }
        }

        std::cout << std::endl;
    }
}

int main()
{
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
