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

#include "a_typed_api.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
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


void sending()
{
    // Create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance("/iox-ex-publisher-simple");

    // create the templateized publisher
    TypedPublisher<CounterTopic> myTypedPublisher({"Radar", "FrontRight", "Counter"});

    uint32_t ct = 0;

    while (!killswitch)
    {
        // allocate a sample
        auto sample = myTypedPublisher.allocate();

        // write the data
        sample->counter = ct;

        std::cout << "Sending: " << ct << std::endl;

        // pass the ownership to the middleware for sending the sample
        myTypedPublisher.publish(std::move(sample));

        ct++;

        // Sleep some time to avoid flooding the system with messages as there's basically no delay in transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main()
{
    // Register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    std::thread tx(sending);
    tx.join();

    return (EXIT_SUCCESS);
}
