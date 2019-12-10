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
#include <iomanip>
#include <iostream>

constexpr uint64_t NUMBER_OF_ROUNDTRIPS{1000000};
constexpr char APP_NAME[] = "/laurel";
constexpr char PUBLISHER[] = "Laurel";
constexpr char SUBSCRIBER[] = "Hardy";

double measureLatency(iox::popo::Publisher& publisher, iox::popo::Subscriber& subscriber)
{
    auto start = std::chrono::high_resolution_clock::now();
    // run the performance test
    for (uint64_t i = 0; i < NUMBER_OF_ROUNDTRIPS; ++i)
    {
        const void* receivedChunk;
        while (!subscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }

        auto receivedSample = static_cast<const PerfTopic*>(receivedChunk);

        auto sendSample = static_cast<PerfTopic*>(publisher.allocateChunk(receivedSample->payloadSize, true));
        sendSample->payloadSize = receivedSample->payloadSize;
        sendSample->run = true;

        publisher.sendChunk(sendSample);

        subscriber.releaseChunk(receivedChunk);
    }

    auto finish = std::chrono::high_resolution_clock::now();

    constexpr uint64_t TRANSMISSIONS_PER_ROUNDTRIP{2};
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
    auto latencyInNanoSeconds = (duration.count() / (NUMBER_OF_ROUNDTRIPS * TRANSMISSIONS_PER_ROUNDTRIP));
    auto latencyInMicroSeconds = latencyInNanoSeconds / 1000.;
    return latencyInMicroSeconds;
}

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

    std::vector<double> latencyInMicroSeconds;
    const std::vector<int64_t> payloadSizesInKB{1, 2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096};
    for (const auto payloadSizeInKB : payloadSizesInKB)
    {
        std::cout << "Measurement for " << payloadSizeInKB << " kB payload ... " << std::flush;
        auto payloadSizeInBytes = payloadSizeInKB * 1024;
        // Allocate a memory chunk for the sample to be sent and allow dynamic sample size, as we dynamically change the
        // payload
        auto sample = static_cast<PerfTopic*>(myPublisher.allocateChunk(payloadSizeInBytes, true));

        // Specify the payload size for the measurement
        sample->payloadSize = payloadSizeInBytes;
        sample->run = true;

        // Send the initial sample to start the round-trips
        myPublisher.sendChunk(sample);

        auto latency = measureLatency(myPublisher, mySubscriber);
        latencyInMicroSeconds.push_back(latency);

        // Wait for hardy to send the last response
        const void* receivedChunk;
        while (!mySubscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }
        mySubscriber.releaseChunk(receivedChunk);
        std::cout << "done" << std::endl;
    }

    const int64_t payloadSize = sizeof(PerfTopic);
    auto stopSample = static_cast<PerfTopic*>(myPublisher.allocateChunk(payloadSize, true));

    // stop iceoryx-hardy
    // Write sample data
    stopSample->payloadSize = payloadSize;
    stopSample->run = false;
    myPublisher.sendChunk(stopSample);

    mySubscriber.unsubscribe();

    std::cout << "Waiting for subscriber to unsubscribe from " << PUBLISHER << " ... " << std::flush;
    while (!myPublisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    myPublisher.stopOffer();

    std::cout << std::endl;
    std::cout << "#### Measurement Result ####" << std::endl;
    std::cout << NUMBER_OF_ROUNDTRIPS << " round trips for each payload." << std::endl;
    std::cout << std::endl;
    std::cout << "| Payload Size [kB] | Average Latency [µs] |" << std::endl;
    std::cout << "|------------------:|---------------------:|" << std::endl;
    for (size_t i = 0; i < latencyInMicroSeconds.size(); ++i)
    {
        std::cout << "| " << std::setw(17) << payloadSizesInKB.at(i) << " | " << std::setw(20) << std::setprecision(2)
                  << latencyInMicroSeconds.at(i) << " |" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "Finished!" << std::endl;

    return (EXIT_SUCCESS);
}
