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

static void sigHandler(int f_sig[[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

void receiving()
{
    // Create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance("/subscriber-bare-metal");

    // Create a subscriber
    iox::popo::Subscriber mySubscriber({"Radar", "FrontLeft", "Counter"});

    // The subscriber will not do the subscription before we call subscribe(). The queue size of the subscriber is
    // provided as parameter
    mySubscriber.subscribe(10);

    while (!killswitch)
    {
        // check if we are subscribed
        if (iox::popo::SubscriptionState::SUBSCRIBED == mySubscriber.getSubscriptionState())
        {
            const void* chunk = nullptr;

            // polling based access to the subscriber
            // this will return true and the oldest chunk in the queue (FiFo) or false if the queue is empty
            while (mySubscriber.getChunk(&chunk))
            {
                // we know what we expect for the CaPro ID we provided with the subscriber c'tor. So we do a cast here
                auto sample = static_cast<const CounterTopic*>(chunk);

                std::cout << "Receiving: " << sample->counter << std::endl;

                // signal the middleware that this chunk was processed and in no more accesssed by the user side
                mySubscriber.releaseChunk(chunk);
            }
        }
        else
        {
            std::cout << "Not subscribed" << std::endl;
        }

        // Sleep some time to avoid flooding the system with messages as there's basically no delay in transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // with unsubscribe we disconnect from the publisher
    mySubscriber.unsubscribe();
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
