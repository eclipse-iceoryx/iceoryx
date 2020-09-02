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

#include "iceoryx_posh/popo/subscriber.hpp"
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

void receiving()
{
//    iox::runtime::PoshRuntime::getInstance("/iox-ex-subscriber-modern");
//    iox::popo::Subscriber mySubscriber({"Odometry", "Position", "Vehicle"});
//    mySubscriber.subscribe(10);
//    while (!killswitch)
//    {
//        if (iox::popo::SubscriptionState::SUBSCRIBED == mySubscriber.getSubscriptionState())
//        {
//            const void* chunk = nullptr;
//            while (mySubscriber.getChunk(&chunk))
//            {
//                std::cout << "Got chunk" << std::endl;
//                auto sample = static_cast<const Position*>(chunk);
//                std::cout << "Received val: (" << sample->x << ", " << sample->y << ", " << sample->z << ")" << std::endl;
//                mySubscriber.releaseChunk(chunk);
//            }
//        }
//        else
//        {
//            std::cout << "Not subscribed" << std::endl;
//        }
//        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//    }
//    mySubscriber.unsubscribe();
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
