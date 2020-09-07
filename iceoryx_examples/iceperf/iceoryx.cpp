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

void Iceoryx::initLeader() noexcept
{
    init();
}

void Iceoryx::initFollower() noexcept
{
    init();
}

void Iceoryx::init() noexcept
{
    m_publisher.offer();
    m_subscriber.subscribe();

    std::cout << "Waiting till subscribed ... " << std::endl << std::flush;
    while (m_subscriber.getSubscriptionState() != iox::popo::SubscriptionState::SUBSCRIBED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << "Waiting for subscriber ... " << std::endl << std::flush;
    while (!m_publisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void Iceoryx::shutdown() noexcept
{
    m_subscriber.unsubscribe();

    std::cout << "Waiting for subscriber to unsubscribe ... " << std::endl << std::flush;
    while (!m_publisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    m_publisher.stopOffer();

    std::cout << "Finished!" << std::endl;
}

void Iceoryx::sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept
{
    auto sendSample = static_cast<PerfTopic*>(m_publisher.allocateChunk(payloadSizeInBytes, true));
    sendSample->payloadSize = payloadSizeInBytes;
    sendSample->run = runFlag;
    sendSample->subPackets = 1;

    m_publisher.sendChunk(sendSample);
}

PerfTopic Iceoryx::receivePerfTopic() noexcept
{
    const void* receivedChunk;
    while (!m_subscriber.getChunk(&receivedChunk))
    {
        // poll as fast as possible
    }

    auto receivedSample = *(static_cast<const PerfTopic*>(receivedChunk));
    m_subscriber.releaseChunk(receivedChunk);

    return receivedSample;
}
