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

#include "topic_data.hpp"

#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/posix_wrapper/signal_handler.hpp"

#include <iostream>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT or SIGTERM, now exit gracefully
    killswitch = true;
}

int main()
{
    // register sigHandler
    auto signalIntGuard = iox::posix::registerSignalHandler(iox::posix::Signal::INT, sigHandler);
    auto signalTermGuard = iox::posix::registerSignalHandler(iox::posix::Signal::TERM, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime("iox-ex-subscriber-with-history");

    // initialized subscriber
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueCapacity = 10U;
    // When starting the subscriber late it will miss the first samples which the
    // publisher has send. The history ensures that we at least get the last 5
    // samples sent by the publisher when we subscribe (if at least 5 were already sent).
    subscriberOptions.historyRequest = 5U;
    iox::popo::Subscriber<RadarObject> subscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);
    subscriber.subscribe();

    // run until interrupted by Ctrl-C
    while (!killswitch)
    {
        if (subscriber.getSubscriptionState() == iox::SubscribeState::SUBSCRIBED)
        {
            bool hasMoreSamples = true;
            // Since we are checking only every second but the publisher is sending every
            // 400ms a new sample we will receive here more then one sample.
            do
            {
                subscriber.take()
                    .and_then([](auto& object) { std::cout << "Got value: " << object->x << std::endl; })
                    .or_else([&](auto&) { hasMoreSamples = false; });
            } while (hasMoreSamples);
        }
        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    subscriber.unsubscribe();

    return (EXIT_SUCCESS);
}
