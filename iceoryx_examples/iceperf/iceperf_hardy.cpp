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
#include "iceoryx_posh/popo/subscriber.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "topic_data.hpp"

#include <chrono>
#include <iostream>

constexpr char APP_NAME[] = "/hardy";
constexpr char PUBLISHER[] = "Hardy";
constexpr char SUBSCRIBER[] = "Laurel";

int main()
{
    // Create the runtime for registering with the RouDi daemon
    iox::runtime::PoshRuntime::getInstance(APP_NAME);

    // Create a publisher and offer the service
    iox::popo::Publisher myPublisher({"Comedians", "Duo", PUBLISHER});
    myPublisher.offer();

    // Create the subscriber and subscribe to the service
    iox::popo::Subscriber mySubscriber({"Comedians", "Duo", SUBSCRIBER});
    mySubscriber.subscribe(1);

    std::cout << "Waiting to subscribe to " << SUBSCRIBER << " ... " << std::flush;
    while (mySubscriber.getSubscriptionState() != iox::popo::SubscriptionState::SUBSCRIBED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;

    std::cout << "Waiting for subscriber to " << PUBLISHER << " ... " << std::flush;
    while (!myPublisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;

    bool run{true};
    while (run)
    {
        const void* receivedChunk;
        while (!mySubscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }

        auto receivedSample = static_cast<const PerfTopic*>(receivedChunk);

        auto sendSample = static_cast<PerfTopic*>(myPublisher.allocateChunk(receivedSample->payloadSize, true));
        sendSample->payloadSize = receivedSample->payloadSize;

        myPublisher.sendChunk(sendSample);

        run = receivedSample->run;
        mySubscriber.releaseChunk(receivedChunk);
    }

    mySubscriber.unsubscribe();

    std::cout << "Waiting for subscriber to unsubscribe from " << PUBLISHER << " ... " << std::flush;
    while (!myPublisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    myPublisher.stopOffer();

    std::cout << "Finished!" << std::endl;

    return (EXIT_SUCCESS);
}
