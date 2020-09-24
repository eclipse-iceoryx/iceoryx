// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
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

    // run until interrupted
    while(!killswitch)
    {
        // wake up on new data
        auto triggeredConditions = waitSet.wait();
        for(auto& condition : triggeredConditions)
        {
            auto subscriber = dynamic_cast<iox::popo::TypedSubscriber<Position>*>(condition);
            subscriber->receive().and_then([](iox::cxx::optional<iox::popo::Sample<const Position>>& maybePosition){
                if(maybePosition.has_value())
                {
                    auto& position = maybePosition.value();
                    std::cout << "Got Position: (" << position->x << ", " << position->y << ", " << position->z << ")" << std::endl;
                }
            });

        }
    }

    waitSet.detachAllConditions();

    return (EXIT_SUCCESS);
}
