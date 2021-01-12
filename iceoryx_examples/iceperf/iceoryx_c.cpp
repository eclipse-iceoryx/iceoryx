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

#include "iceoryx_c.hpp"

#include <chrono>
#include <thread>

IceoryxC::IceoryxC(const iox::capro::IdString_t& publisherName, const iox::capro::IdString_t& subscriberName) noexcept
    : m_publisher(iox_pub_init(&m_publisherStorage, "Comedians", publisherName.c_str(), "Duo", 0U))
    , m_subscriber(iox_sub_init(&m_subscriberStorage, "Comedians", subscriberName.c_str(), "Duo", 10U, 0U))

{
}

IceoryxC::~IceoryxC()
{
    iox_pub_deinit(m_publisher);
    iox_sub_deinit(m_subscriber);
}

void IceoryxC::initLeader() noexcept
{
    init();
}

void IceoryxC::initFollower() noexcept
{
    init();
}

void IceoryxC::init() noexcept
{
    iox_pub_offer(m_publisher);
    iox_sub_subscribe(m_subscriber);

    std::cout << "Waiting for: subscription" << std::flush;
    while (iox_sub_get_subscription_state(m_subscriber) != SubscribeState_SUBSCRIBED)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    std::cout << ", subscriber" << std::flush;
    while (!iox_pub_has_subscribers(m_publisher))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << " [ success ]" << std::endl;
}

void IceoryxC::shutdown() noexcept
{
    iox_sub_unsubscribe(m_subscriber);

    std::cout << "Waiting for: unsubscribe " << std::flush;
    while (!iox_pub_has_subscribers(m_publisher))
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    // with stopOffer we disconnect all subscribers and the publisher is no more visible
    iox_pub_stop_offer(m_publisher);
    std::cout << " [ finished ]" << std::endl;
}

void IceoryxC::sendPerfTopic(uint32_t payloadSizeInBytes, bool runFlag) noexcept
{
    void* sample = nullptr;
    if (iox_pub_allocate_chunk(m_publisher, &sample, payloadSizeInBytes) == AllocationResult_SUCCESS)
    {
        auto sendSample = static_cast<PerfTopic*>(sample);
        sendSample->payloadSize = payloadSizeInBytes;
        sendSample->run = runFlag;
        sendSample->subPackets = 1;
        iox_pub_send_chunk(m_publisher, sample);
    }
}

PerfTopic IceoryxC::receivePerfTopic() noexcept
{
    bool hasReceivedSample{false};
    PerfTopic receivedSample;

    do
    {
        const void* sample = nullptr;
        if (iox_sub_get_chunk(m_subscriber, &sample) == ChunkReceiveResult_SUCCESS)
        {
            receivedSample = *(static_cast<const PerfTopic*>(sample));
            hasReceivedSample = true;
            iox_sub_release_chunk(m_subscriber, sample);
        }
    } while (!hasReceivedSample);

    return receivedSample;
}
