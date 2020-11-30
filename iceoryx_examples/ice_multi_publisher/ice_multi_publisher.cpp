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

#include "iceoryx_posh/popo/modern_api/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>
#include <string>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    killswitch = true;
}

void send(uint32_t id, const char* instanceStr, std::chrono::milliseconds delay)
{
    // iox::capro::IdString instance{Instance" + std::to_string(id)};

    // note: we consider the scenario that the instance name is only known at runtime
    // constructing strings at runtime by appending/concatenating cxx::strings has limitations
    // (this is more concise with compile time names, but runtime names are supported)
    // todo: can this be done better with the current string API?

    iox::capro::IdString instance{iox::cxx::TruncateToCapacity, instanceStr};
    iox::capro::IdString idStr{
        iox::cxx::TruncateToCapacity,
        std::to_string(id).c_str(),
    };
    instance.append(iox::cxx::TruncateToCapacity, idStr);

    iox::popo::TypedPublisher<CounterTopic> publisher({"CounterTopic", instance, "Counter"});
    publisher.offer();

    for (uint32_t counter = 0U; !killswitch; ++counter)
    {
        CounterTopic data{counter, id};
        publisher.publishCopyOf(data);

        // prevent undesired output interleaves of independent sender threads
        std::stringstream s;
        s << instance.c_str() << " sending: " << data << std::endl;
        std::cout << s.str();

        std::this_thread::sleep_for(delay);
    }

    publisher.stopOffer();
}

int main()
{
    signal(SIGINT, sigHandler);

    iox::runtime::PoshRuntime::getInstance("/iox-publisher");

    // generates multiple publishers which send the same topic (with different or same instances)
    // and different sending frequencies independently of each other
    std::thread sender1(send, 1, "Instance1", std::chrono::milliseconds(500));
    std::thread sender2(send, 2, "Instance2", std::chrono::milliseconds(1000));
    std::thread sender3(send, 3, "Instance2", std::chrono::milliseconds(2000));

    sender1.join();
    sender2.join();
    sender3.join();

    return (EXIT_SUCCESS);
}
