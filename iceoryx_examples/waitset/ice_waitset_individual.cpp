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

#include "iceoryx_posh/popo/typed_subscriber.hpp"
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
    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::initRuntime("iox-ex-waitset-individual");

    iox::popo::WaitSet<> waitset;

    // attach shutdownTrigger to handle CTRL+C
    waitset.attachEvent(shutdownTrigger);

    // create two subscribers, subscribe to the service and attach them to the waitset
    iox::popo::TypedSubscriber<CounterTopic> subscriber1({"Radar", "FrontLeft", "Counter"});
    iox::popo::TypedSubscriber<CounterTopic> subscriber2({"Radar", "FrontLeft", "Counter"});

    subscriber1.subscribe();
    subscriber2.subscribe();

    waitset.attachEvent(subscriber1, iox::popo::SubscriberEvent::HAS_SAMPLES);
    waitset.attachEvent(subscriber2, iox::popo::SubscriberEvent::HAS_SAMPLES);

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
            // process sample received by subscriber1
            else if (event->doesOriginateFrom(&subscriber1))
            {
                subscriber1.take().and_then([&](iox::popo::Sample<const CounterTopic>& sample) {
                    std::cout << " subscriber 1 received: " << sample->counter << std::endl;
                });
            }
            // dismiss sample received by subscriber2
            if (event->doesOriginateFrom(&subscriber2))
            {
                // We need to release the samples to reset the trigger hasSamples
                // otherwise the WaitSet would notify us in `waitset.wait()` again
                // instantly.
                subscriber2.releaseQueuedSamples();
                std::cout << "subscriber 2 received something - dont care\n";
            }
        }

        std::cout << std::endl;
    }

    return (EXIT_SUCCESS);
}
