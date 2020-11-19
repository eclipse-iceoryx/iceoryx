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

#include "iceoryx_posh/popo/guard_condition.hpp"
#include "iceoryx_posh/popo/modern_api/typed_subscriber.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::GuardCondition shutdownGuard;
using Subscriber = iox::popo::TypedSubscriber<CounterTopic>;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownGuard.trigger();
}

void receiving()
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-waitset");

    iox::popo::TypedSubscriber<CounterTopic> subscriber1({"Radar", "FrontLeft", "Counter"});
    iox::popo::TypedSubscriber<CounterTopic> subscriber2({"Radar", "FrontLeft", "Counter"});
    iox::popo::WaitSet waitset;

    subscriber1.subscribe();
    subscriber2.subscribe();

    subscriber1.attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    subscriber2.attachToWaitset(waitset, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);


    shutdownGuard.attachToWaitset(waitset);

    while (true)
    {
        auto triggeredConditions = waitset.wait();

        for (auto& condition : triggeredConditions)
        {
            if (condition.doesOriginateFrom(&subscriber1))
            {
                subscriber1.take().and_then([&](iox::popo::Sample<const CounterTopic>& sample) {
                    std::cout << " subscriber 1 received: " << sample->counter << std::endl;
                });
            }
            if (condition.doesOriginateFrom(&subscriber2))
            {
                std::cout << "subscriber 2 received something - dont care\n";
            }
            else if (condition.doesOriginateFrom(&shutdownGuard))
            {
                return;
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
