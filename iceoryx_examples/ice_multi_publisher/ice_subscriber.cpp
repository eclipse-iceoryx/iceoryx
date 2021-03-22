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

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iostream>

bool killswitch = false;

static void sigHandler(int sig [[gnu::unused]])
{
    killswitch = true;
}

void receive()
{
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.nodeName = "uMgungundlovu";

    iox::popo::Subscriber<CounterTopic> subscriber({"Group", "Instance", "Counter"}, subscriberOptions);

    while (!killswitch)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        while (subscriber.hasData())
        {
            subscriber.take()
                .and_then([](auto& sample) { std::cout << "Received: " << *sample.get() << std::endl; })
                .or_else([](auto&) { std::cout << "Error while receiving." << std::endl; });
        };
        std::cout << "Waiting for data ... " << std::endl;
    }
}

int main()
{
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);
    iox::runtime::PoshRuntime::initRuntime("iox-subscriber");

    std::thread receiver(receive);
    receiver.join();

    return (EXIT_SUCCESS);
}
