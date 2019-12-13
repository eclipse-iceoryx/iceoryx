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


// the callback for processing the samples
void myCallback(const CounterTopic& sample)
{
    std::cout << "Callback: " << sample.counter << std::endl;
}

void receiving()
{
    // Create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance("/subscriber-simple");

    // Create the typed subscriber and provide the callback, the rest will be executed in middleware context
    TypedSubscriber<CounterTopic> myTypedSubscriber({"Radar", "FrontRight", "Counter"}, myCallback);

    while (!killswitch)
    {
        // Sleep some time to avoid flooding the system with messages as there's basically no delay in transfer
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    std::thread rx(receiving);
    rx.join();

    return (EXIT_SUCCESS);
}
