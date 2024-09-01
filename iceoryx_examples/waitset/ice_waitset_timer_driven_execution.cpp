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

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/atomic.hpp"
#include "iox/signal_handler.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iostream>

iox::concurrent::Atomic<bool> keepRunning{true};

using WaitSet = iox::popo::WaitSet<>;
volatile iox::popo::WaitSet<>* waitsetSigHandlerAccess{nullptr};

static void sigHandler(int f_sig [[maybe_unused]])
{
    keepRunning = false;
    if (waitsetSigHandlerAccess)
    {
        waitsetSigHandlerAccess->markForDestruction();
    }
}

//! [cyclic run]
class SomeClass
{
  public:
    static void cyclicRun(iox::popo::UserTrigger*)
    {
        std::cout << "activation callback\n";
    }
};
//! [cyclic run]

int main()
{
    // register sigHandler
    auto signalIntGuard =
        iox::registerSignalHandler(iox::PosixSignal::INT, sigHandler).expect("failed to register SIGINT");
    auto signalTermGuard =
        iox::registerSignalHandler(iox::PosixSignal::TERM, sigHandler).expect("failed to register SIGTERM");

    iox::runtime::PoshRuntime::initRuntime("iox-cpp-waitset-sync");

    //! [create waitset]
    iox::popo::WaitSet<> waitset;
    waitsetSigHandlerAccess = &waitset;
    //! [create waitset]

    // create and attach the cyclicTrigger with a callback to
    // SomeClass::cyclicRun
    //! [create trigger]
    iox::popo::UserTrigger cyclicTrigger;
    waitset.attachEvent(cyclicTrigger, 0U, createNotificationCallback(SomeClass::cyclicRun)).or_else([](auto) {
        std::cerr << "failed to attach cyclic trigger" << std::endl;
        std::exit(EXIT_FAILURE);
    });
    //! [create trigger]

    // start a thread which triggers cyclicTrigger every second
    //! [cyclic thread]
    std::thread cyclicTriggerThread([&] {
        while (keepRunning.load())
        {
            cyclicTrigger.trigger();
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });
    //! [cyclic thread]

    //! [event loop]
    while (keepRunning.load())
    {
        auto notificationVector = waitset.wait();

        for (auto& notification : notificationVector)
        {
            //! [data path]
            // call SomeClass::cyclicRun
            (*notification)();
            //! [data path]
        }

        std::cout << std::endl;
    }
    //! [event loop]

    cyclicTriggerThread.join();

    waitsetSigHandlerAccess = nullptr; // invalidate for signal handler

    return (EXIT_SUCCESS);
}
