// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/popo/modern_api/untyped_subscriber.hpp"
#include "iceoryx_posh/popo/guard_condition.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

bool killswitch = false;
iox::popo::GuardCondition shutdownGuard;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
    shutdownGuard.trigger(); // unblock waitsets
}

void subscriberHandler(iox::popo::WaitSet& waitSet)
{
    // run until interrupted
    while(!killswitch)
    {
        auto triggeredConditions = waitSet.wait();
        for(auto& condition : triggeredConditions)
        {
            auto subscriber = dynamic_cast<iox::popo::TypedSubscriber<Position>*>(condition);
            if(subscriber)
            {
                subscriber->take().and_then([](iox::cxx::optional<iox::popo::Sample<const Position>>& maybePosition){
                    if(maybePosition.has_value())
                    {
                        auto& position = maybePosition.value();
                        std::cout << "Got value: (" << position->x << ", " << position->y << ", " << position->z << ")" << std::endl;
                    }
                });
            }
        }
    }
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-typed-modern");

    // initialized subscribers
    iox::popo::TypedSubscriber<Position> typedSubscriber({"Odometry", "Position", "Vehicle"});
    typedSubscriber.subscribe();

    // set up waitset
    iox::popo::WaitSet waitSet{};
    waitSet.attachCondition(typedSubscriber);
    waitSet.attachCondition(shutdownGuard);

    // delegate handling of received data to another thread
    std::thread subscriberHandlerThread(subscriberHandler, std::ref(waitSet));
    subscriberHandlerThread.join();

    return (EXIT_SUCCESS);
}
