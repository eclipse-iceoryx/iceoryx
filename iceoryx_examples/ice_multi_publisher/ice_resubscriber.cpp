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

#include "iceoryx_posh/popo/typed_subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <csignal>
#include <iostream>

bool killswitch = false;

static void sigHandler(int sig [[gnu::unused]])
{
    killswitch = true;
}

constexpr uint64_t UNSUBSCRIBED_TIME_SECONDS{3U};

void receive()
{
    iox::popo::SubscriberOptions options;
    // the maximum number of samples the subscriber can hold before discarding the least
    // recent sample (i.e. the capacity of the sample queue on subscriber side)
    options.queueCapacity = 4U;
    iox::popo::TypedSubscriber<CounterTopic> subscriber({"Group", "Instance", "Counter"}, options);

    subscriber.subscribe();
    while (!killswitch)
    {
        // unsubscribe and resubscribe
        subscriber.unsubscribe();
        std::cout << "Unsubscribed ... Subscribe in " << UNSUBSCRIBED_TIME_SECONDS << " seconds" << std::endl;

        // we will probably miss some data while unsubscribed
        std::this_thread::sleep_for(std::chrono::seconds(UNSUBSCRIBED_TIME_SECONDS));

        // we (re)subscribe and should see at most the latest options.queueCapacity
        subscriber.subscribe();

        std::this_thread::sleep_for(std::chrono::seconds(1));

        while (subscriber.hasSamples())
        {
            subscriber.take()
                .and_then([](iox::popo::Sample<const CounterTopic>& sample) {
                    std::cout << "Received: " << *sample.get() << std::endl;
                })
                .or_else([](iox::popo::ChunkReceiveError) { std::cout << "Error while receiving." << std::endl; });
        };
        std::cout << "Waiting for data ... " << std::endl;
    }
    subscriber.unsubscribe();
}

int main()
{
    signal(SIGINT, sigHandler);
    iox::runtime::PoshRuntime::initRuntime("/iox-resubscriber");

    std::thread receiver(receive);
    receiver.join();

    return (EXIT_SUCCESS);
}
