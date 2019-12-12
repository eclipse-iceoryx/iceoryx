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

#include "iceoryx_posh/popo/publisher.hpp"
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

void sending()
{
    // Create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance("/publisher-bare-metal");

    // Create a publisher
    iox::popo::Publisher myPublisher({"Radar", "FrontLeft", "Counter"});

    // With offer() the publisher gets visible to potential subscribers
    myPublisher.offer();

    uint32_t ct = 0;

    while (!killswitch)
    {
        // Allocate a memory chunk for the sample to be sent
        auto sample = static_cast<CounterTopic*>(myPublisher.allocateChunk(sizeof(CounterTopic)));

        // Write sample data
        sample->counter = ct;

        std::cout << "Sending: " << ct << std::endl;

        // Send the sample
        myPublisher.sendChunk(sample);

        ct++;

        // Sleep some time to avoid flooding the system with messages as there's basically no delay in transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    myPublisher.stopOffer();
}

int main()
{
    // Register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    std::thread tx(sending);
    tx.join();

    return (EXIT_SUCCESS);
}
