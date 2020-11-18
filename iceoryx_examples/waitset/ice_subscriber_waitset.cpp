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

#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/popo/guard_condition.hpp"
#include "iceoryx_posh/popo/modern_api/typed_subscriber.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

/// TODO trigger type?

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::GuardCondition shutdownGuard;
using Subscriber = iox::popo::TypedSubscriber<CounterTopic>;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownGuard.trigger();
}

void subscriberCallback(iox::popo::Condition* const subscriber)
{
    reinterpret_cast<Subscriber*>(subscriber)->take().and_then([](iox::popo::Sample<const CounterTopic>& sample) {
        std::cout << "Received: " << sample->counter << std::endl;
    });
}

void receiving()
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-waitset");

    iox::popo::TypedSubscriber<CounterTopic> mySubscriber({"Radar", "FrontLeft", "Counter"});
    iox::popo::WaitSet waitset;

    mySubscriber.attachTo(&waitset, {&mySubscriber, &Subscriber::hasNewSamples}, 5, subscriberCallback);

    mySubscriber.subscribe();

    iox::popo::GuardCondition guard;
    guard.attachToWaitset(waitset);

    bool keepRunning = true;

    std::thread t1([&] {
        std::this_thread::sleep_for(std::chrono::seconds(4));
        guard.trigger();
    });

    while (keepRunning)
    {
        auto triggeredConditions = waitset.wait();

        for (auto& condition : triggeredConditions)
        {
            // if (condition.getTriggerId() == 5)
            //{
            //    mySubscriber.take().and_then([](iox::popo::Sample<const CounterTopic>& sample) {
            //        std::cout << "Received: " << sample->counter << std::endl;
            //    });
            //}
            if (condition.doesOriginateFrom(&mySubscriber))
            {
                // mySubscriber.take().and_then([](iox::popo::Sample<const CounterTopic>& sample) {
                //    std::cout << "Received: " << sample->counter << std::endl;
                //});
                condition();
            }
            else if (condition.getTriggerId() == 4)
            {
                mySubscriber.unsubscribe();
                keepRunning = false;
            }
            else if (condition.doesOriginateFrom(&guard))
            {
                std::cout << "guard called\n";
                mySubscriber.unsubscribe();
                keepRunning = false;
            }
            else
            {
                std::cout << "UNDEFINED\n";
            }
        }
    }

    printf("waiting for thread\n");
    t1.join();
}

int main()
{
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
