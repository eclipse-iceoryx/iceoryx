// Copyright (c) 2020, 2021 by Apex.AI Inc. All rights reserved.
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

class SomeClass
{
  public:
    static void cyclicRun(iox::popo::UserTrigger* trigger)
    {
        std::cout << "activation callback\n";

        // after every call we have to reset the trigger otherwise the waitset
        // would immediately call us again since we still signal to the waitset that
        // we have been triggered (waitset is state based)
        trigger->resetTrigger();
    }
};

int main()
{
    signal(SIGINT, sigHandler);
    iox::runtime::PoshRuntime::initRuntime("iox-ex-waitset-sync");
    std::atomic_bool keepRunning{true};

    iox::popo::WaitSet<> waitset;

    // attach shutdownTrigger to handle CTRL+C
    waitset.attachEvent(shutdownTrigger);

    // create and attach the cyclicTrigger with a callback to
    // SomeClass::myCyclicRun
    iox::popo::UserTrigger cyclicTrigger;
    waitset.attachEvent(cyclicTrigger, 0U, &SomeClass::cyclicRun);

    // start a thread which triggers cyclicTrigger every second
    std::thread cyclicTriggerThread([&] {
        while (keepRunning.load())
        {
            cyclicTrigger.trigger();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    // event loop
    while (keepRunning.load())
    {
        auto eventVector = waitset.wait();

        for (auto& event : eventVector)
        {
            if (event->doesOriginateFrom(&shutdownTrigger))
            {
                // CTRL+c was pressed -> exit
                keepRunning.store(false);
            }
            else
            {
                // call SomeClass::myCyclicRun
                (*event)();
            }
        }

        std::cout << std::endl;
    }

    cyclicTriggerThread.join();
    return (EXIT_SUCCESS);
}
