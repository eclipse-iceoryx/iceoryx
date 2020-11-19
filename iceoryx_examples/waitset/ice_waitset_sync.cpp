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

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownGuard.trigger();
}

void waitForNextActivation(iox::popo::GuardCondition*)
{
    std::cout << "activation callback\n";
}

void receiving()
{
    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-waitset");

    iox::popo::WaitSet waitset;

    iox::popo::GuardCondition waitForNextActivationTrigger;
    waitForNextActivationTrigger.attachToWaitset(waitset, 0, waitForNextActivation);

    shutdownGuard.attachToWaitset(waitset);

    std::thread t1([&] {
        while (true)
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            waitForNextActivationTrigger.trigger();
        }
    });

    while (true)
    {
        auto triggeredConditions = waitset.wait();

        for (auto& condition : triggeredConditions)
        {
            if (condition.doesOriginateFrom(&shutdownGuard))
            {
                return;
            }
            else
            {
                condition();
            }
        }

        std::cout << std::endl;
    }

    t1.join();
}

int main()
{
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
