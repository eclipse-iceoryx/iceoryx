// Copyright (c) 2020 by Apex.AI. All rights reserved.
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

#include "topic_data.hpp"

#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

#include <csignal>
#include <iostream>

bool killswitch = false;

static void sigHandler(int f_sig [[gnu::unused]])
{
    // caught SIGINT, now exit gracefully
    killswitch = true;
}

int main()
{
    // register sigHandler for SIGINT
    signal(SIGINT, sigHandler);

    // initialize runtime
    iox::runtime::PoshRuntime::initRuntime("iox-ex-subscriber-with-history");

    // initialized subscriber
    iox::popo::SubscriberOptions subscriberOptions;
    subscriberOptions.queueCapacity = 10U;
    // When starting the subscriber late it will miss the first samples which the
    // publisher has send. The history ensures that we at least get the last 5
    // samples sent by the publisher when we subscribe (if at least 5 were already sent).
    subscriberOptions.historyRequest = 5U;
    iox::popo::TypedSubscriber<RadarObject> typedSubscriber({"Radar", "FrontLeft", "Object"}, subscriberOptions);
    typedSubscriber.subscribe();

    // run until interrupted by Ctrl-C
    while (!killswitch)
    {
        if (typedSubscriber.getSubscriptionState() == iox::SubscribeState::SUBSCRIBED)
        {
            bool hasMoreSamples = true;
            // Since we are checking only every second but the publisher is sending every
            // 400ms a new sample we will receive here more then one sample.
            do
            {
                typedSubscriber.take()
                    .and_then([](iox::popo::Sample<const RadarObject>& object) {
                        std::cout << "Got value: " << object->x << std::endl;
                    })
                    .if_empty([&] { hasMoreSamples = false; });
            } while (hasMoreSamples);
        }
        std::cout << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    typedSubscriber.unsubscribe();

    return (EXIT_SUCCESS);
}
