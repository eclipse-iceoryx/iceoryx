// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/popo/user_trigger.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

bool killswitch = false;
iox::popo::UserTrigger shutdownTrigger;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
    shutdownTrigger.trigger(); // unblock waitsets
}

void subscriberHandler(iox::popo::WaitSet<>& waitSet)
{
    // run until interrupted
    while (!killswitch)
    {
        auto triggerVector = waitSet.wait();
        for (auto& trigger : triggerVector)
        {
            if (trigger.doesOriginateFrom(&shutdownTrigger))
            {
                return;
            }
            else
            {
                auto untypedSubscriber = trigger.getOrigin<iox::popo::UntypedSubscriber>();
                untypedSubscriber->take()
                    .and_then([](iox::cxx::optional<iox::popo::Sample<const void>>& allocation) {
                        auto position = reinterpret_cast<const Position*>(allocation->get());
                        std::cout << "Got value: (" << position->x << ", " << position->y << ", " << position->z << ")"
                                  << std::endl;
                    })
                    .if_empty([] { std::cout << "Didn't get a value, but do something anyway." << std::endl; })
                    .or_else([](iox::popo::ChunkReceiveError) { std::cout << "Error receiving chunk." << std::endl; });
            }
        }
    }
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime("iox-ex-subscriber-untyped-modern");

    // initialized subscribers
    iox::popo::UntypedSubscriber untypedSubscriber({"Odometry", "Position", "Vehicle"});
    untypedSubscriber.subscribe();

    // set up waitset
    iox::popo::WaitSet<> waitSet;
    untypedSubscriber.attachTo(waitSet, iox::popo::SubscriberEvent::HAS_NEW_SAMPLES);
    shutdownTrigger.attachTo(waitSet);

    // delegate handling of received data to another thread
    std::thread untypedSubscriberThread(subscriberHandler, std::ref(waitSet));
    untypedSubscriberThread.join();

    return (EXIT_SUCCESS);
}
