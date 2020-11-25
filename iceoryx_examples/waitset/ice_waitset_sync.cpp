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

#include "iceoryx_posh/popo/modern_api/typed_subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

iox::popo::UserTrigger shutdownGuard;

static void sigHandler(int f_sig [[gnu::unused]])
{
    shutdownGuard.trigger();
}

class SomeClass
{
  public:
    static void cyclicRun(iox::popo::UserTrigger*)
    {
        std::cout << "activation callback\n";
    }
};

int main()
{
    signal(SIGINT, sigHandler);
    iox::runtime::PoshRuntime::getInstance("/iox-ex-waitset-sync");
    std::atomic_bool keepRunning{true};

    iox::popo::WaitSet waitset;

    // attach shutdownGuard to handle CTRL+C
    shutdownGuard.attachToWaitset(waitset);

    // create and attach the cyclicTrigger with a callback to
    // SomeClass::myCyclicRun
    iox::popo::UserTrigger cyclicTrigger;
    cyclicTrigger.attachToWaitset(waitset, 0, SomeClass::cyclicRun);

    // start a thread which triggers cyclicTrigger every second
    std::thread cyclicTriggerThread([&] {
        while (keepRunning.load())
        {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            cyclicTrigger.trigger();
        }
    });

    // event loop
    while (true)
    {
        auto triggerVector = waitset.wait();

        for (auto& trigger : triggerVector)
        {
            if (trigger.doesOriginateFrom(&shutdownGuard))
            {
                // CTRL+c was pressed -> exit
                keepRunning.store(false);
                cyclicTriggerThread.join();
                return (EXIT_SUCCESS);
            }
            else
            {
                // call SomeClass::myCyclicRun
                trigger();
            }
        }

        std::cout << std::endl;
    }

    cyclicTriggerThread.join();

    return (EXIT_SUCCESS);
}
