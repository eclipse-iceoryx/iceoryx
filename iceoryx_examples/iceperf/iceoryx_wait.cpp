// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_wait.hpp"

//#include <chrono>
//#include <thread>

IceoryxWait::IceoryxWait(const iox::capro::IdString_t& publisherName, const iox::capro::IdString_t& subscriberName) noexcept
    : Iceoryx(publisherName, subscriberName, "C++-Wait-API")
{
}
/*
void Iceoryx::initLeader() noexcept
{
    init();
}

void Iceoryx::initFollower() noexcept
{
    init();
}
*/
void IceoryxWait::init() noexcept
{
    Iceoryx::init();
/*    std::cout << "Waiting for: subscription" << std::flush;
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
*/
    waitset.attachState(m_subscriber, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
        std::cerr << "failed to attach subscriber" << std::endl;
        std::exit(EXIT_FAILURE);
    });
}
/*
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

void Iceoryx::sendPerfTopic(const uint32_t payloadSizeInBytes, const RunFlag runFlag) noexcept
{
    m_publisher.loan(payloadSizeInBytes).and_then([&](auto& userPayload) {
        auto sendSample = static_cast<PerfTopic*>(userPayload);
        sendSample->payloadSize = payloadSizeInBytes;
        sendSample->runFlag = runFlag;
        sendSample->subPackets = 1;

        m_publisher.publish(userPayload);
    });
}
*/
PerfTopic IceoryxWait::receivePerfTopic() noexcept
{
    bool hasReceivedSample{false};
    PerfTopic receivedSample;

    auto notificationVector = waitset.wait();
    for (auto& notification : notificationVector)
    {
        if (notification->doesOriginateFrom(&m_subscriber))
        {
            m_subscriber.take().and_then([&](const void* data) {
                receivedSample = *(static_cast<const PerfTopic*>(data));
                hasReceivedSample = true;
                m_subscriber.release(data);
            });
        }
    }

    return receivedSample;
}
