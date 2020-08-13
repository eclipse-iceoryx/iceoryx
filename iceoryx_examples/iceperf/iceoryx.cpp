// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx.hpp"

#include <chrono>

Iceoryx::Iceoryx(const iox::capro::IdString& publisherName, const iox::capro::IdString& subscriberName) noexcept
    : m_publisher({"Comedians", publisherName, "Duo"})
    , m_subscriber({"Comedians", subscriberName, "Duo"})
{
}

void Iceoryx::init() noexcept
{
    m_publisher.offer();
    m_subscriber.subscribe();

    std::cout << "Waiting till subscribed ... " << std::flush;
    while (m_subscriber.getSubscriptionState() != iox::popo::SubscriptionState::SUBSCRIBED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;

    std::cout << "Waiting for subscriber ... " << std::flush;
    while (!m_publisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;
}

void Iceoryx::initLeader() noexcept
{
    init();
}

void Iceoryx::initFollower() noexcept
{
    init();
}

void Iceoryx::shutdown() noexcept
{
    m_subscriber.unsubscribe();

    std::cout << "Waiting for subscriber to unsubscribe ... " << std::flush;
    while (!m_publisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "done" << std::endl;

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    m_publisher.stopOffer();

    std::cout << "Finished!" << std::endl;
}

void Iceoryx::prePingPongLeader(uint32_t payloadSizeInBytes) noexcept
{
    // Allocate a memory chunk for the sample to be sent and allow dynamic sample size, as we dynamically change the
    // payload
    auto sample = static_cast<PerfTopic*>(m_publisher.allocateChunk(payloadSizeInBytes, true));

    // Specify the payload size for the measurement
    sample->payloadSize = payloadSizeInBytes;
    sample->run = true;

    // Send the initial sample to start the round-trips
    m_publisher.sendChunk(sample);
}

void Iceoryx::postPingPongLeader() noexcept
{
    // Wait for the last response
    const void* receivedChunk;
    while (!m_subscriber.getChunk(&receivedChunk))
    {
        // poll as fast as possible
    }
    m_subscriber.releaseChunk(receivedChunk);
    std::cout << "done" << std::endl;
}

void Iceoryx::triggerEnd() noexcept
{
    const int64_t payloadSize = sizeof(PerfTopic);
    auto stopSample = static_cast<PerfTopic*>(m_publisher.allocateChunk(payloadSize, true));

    // Write sample data
    stopSample->payloadSize = payloadSize;
    stopSample->run = false;
    m_publisher.sendChunk(stopSample);
}

double Iceoryx::pingPongLeader(int64_t numRoundTrips) noexcept
{
    auto start = std::chrono::high_resolution_clock::now();
    // run the performance test
    for (auto i = 0; i < numRoundTrips; ++i)
    {
        const void* receivedChunk;
        while (!m_subscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }

        auto receivedSample = static_cast<const PerfTopic*>(receivedChunk);

        auto sendSample = static_cast<PerfTopic*>(m_publisher.allocateChunk(receivedSample->payloadSize, true));
        sendSample->payloadSize = receivedSample->payloadSize;
        sendSample->run = true;

        m_publisher.sendChunk(sendSample);

        m_subscriber.releaseChunk(receivedChunk);
    }

    auto finish = std::chrono::high_resolution_clock::now();

    constexpr int64_t TRANSMISSIONS_PER_ROUNDTRIP{2};
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(finish - start);
    auto latencyInNanoSeconds = (duration.count() / (numRoundTrips * TRANSMISSIONS_PER_ROUNDTRIP));
    auto latencyInMicroSeconds = static_cast<double>(latencyInNanoSeconds) / 1000;
    return latencyInMicroSeconds;
}

void Iceoryx::pingPongFollower() noexcept
{
    bool run{true};
    while (run)
    {
        const void* receivedChunk;
        while (!m_subscriber.getChunk(&receivedChunk))
        {
            // poll as fast as possible
        }

        auto receivedSample = static_cast<const PerfTopic*>(receivedChunk);

        auto sendSample = static_cast<PerfTopic*>(m_publisher.allocateChunk(receivedSample->payloadSize, true));
        sendSample->payloadSize = receivedSample->payloadSize;

        m_publisher.sendChunk(sendSample);

        run = receivedSample->run;
        m_subscriber.releaseChunk(receivedChunk);
    }
}