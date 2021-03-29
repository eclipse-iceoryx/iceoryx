// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/popo/publisher.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iostream>
#include <string>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    killswitch = true;
}

void send(uint32_t id, const char* instanceName, std::chrono::milliseconds delay, const char* node)
{
    iox::capro::IdString_t instance{iox::cxx::TruncateToCapacity, instanceName};

    iox::popo::PublisherOptions publisherOptions;
    iox::NodeName_t nodeName{iox::cxx::TruncateToCapacity, node};
    publisherOptions.nodeName = nodeName;

    // All three of the string identifiers together uniquely identify a topic
    // and can also depend on values known only at runtime (like instance in this case).
    iox::popo::Publisher<CounterTopic> publisher({"Group", instance, "Counter"}, publisherOptions);

    for (uint32_t counter = 0U; !killswitch; ++counter)
    {
        CounterTopic data{counter, id};
        publisher.publishCopyOf(data).or_else([](auto) { std::cerr << "failed to send data" << std::endl; });

        // prevent undesired output interleaves of independent sender threads
        std::stringstream s;
        s << "Counter " << instance.c_str() << " sending: " << data << std::endl;
        std::cout << s.str();

        std::this_thread::sleep_for(delay);
    }

    publisher.stopOffer();
}

int main()
{
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    iox::runtime::PoshRuntime::initRuntime("iox-publisher");

    // generates multiple publishers which send the same topic
    // at different sending frequencies independently of each other
    std::thread sender1(send, 1, "Instance", std::chrono::milliseconds(500), "Node1");
    std::thread sender2(send, 2, "Instance", std::chrono::milliseconds(1000), "Node2");

    sender1.join();
    sender2.join();

    return (EXIT_SUCCESS);
}
