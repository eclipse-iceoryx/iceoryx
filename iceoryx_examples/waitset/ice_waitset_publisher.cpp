// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

static void sigHandler(int f_sig [[gnu::unused]])
{
    killswitch = true;
}

void sending()
{
    iox::runtime::PoshRuntime::initRuntime("iox-ex-publisher-waitset");

    iox::popo::TypedPublisher<CounterTopic> myPublisher({"Radar", "FrontLeft", "Counter"});
    myPublisher.offer();

    for (uint32_t counter = 0U; !killswitch; ++counter)
    {
        myPublisher.publishCopyOf(CounterTopic{counter});
        std::cout << "Sending: " << counter << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    myPublisher.stopOffer();
}

int main()
{
    signal(SIGINT, sigHandler);

    std::thread tx(sending);
    tx.join();

    return (EXIT_SUCCESS);
}
