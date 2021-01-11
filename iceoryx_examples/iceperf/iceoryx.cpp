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
#include <thread>

Iceoryx::Iceoryx(const iox::capro::IdString_t& publisherName, const iox::capro::IdString_t& subscriberName) noexcept
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

    std::cout << "Waiting for: subscription" << std::flush;
    while (m_subscriber.getSubscriptionState() != iox::SubscribeState::SUBSCRIBED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << ", subscriber" << std::flush;
    while (!m_publisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << " [ success ]" << std::endl;
}

void Iceoryx::shutdown() noexcept
{
    m_subscriber.unsubscribe();

    std::cout << "Waiting for: unsubscribe " << std::flush;
    while (!m_publisher.hasSubscribers())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    m_publisher.stopOffer();
    std::cout << " [ finished ]" << std::endl;
}

void Iceoryx::sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept
{
    m_publisher.loan(payloadSizeInBytes).and_then([&](auto& sample) {
        auto sendSample = static_cast<PerfTopic*>(sample.get());
        sendSample->payloadSize = payloadSizeInBytes;
        sendSample->run = runFlag;
        sendSample->subPackets = 1;
        sample.publish();
    });
}

PerfTopic Iceoryx::receivePerfTopic() noexcept
{
    bool hasReceivedSample{false};
    PerfTopic receivedSample;

    do
    {
        m_subscriber.take().and_then([&](iox::popo::Sample<const void>& sample) {
            receivedSample = *(static_cast<const PerfTopic*>(sample.get()));
            hasReceivedSample = true;
        });
    } while (!hasReceivedSample);

    return receivedSample;
}
