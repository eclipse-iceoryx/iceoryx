// Copyright 2023, Eclipse Foundation and the iceoryx contributors. All rights reserved.
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

IceoryxWait::IceoryxWait(const iox::capro::IdString_t& publisherName,
                         const iox::capro::IdString_t& subscriberName) noexcept
    : Iceoryx(publisherName, subscriberName, "C++-Wait-API")
{
}

void IceoryxWait::init() noexcept
{
    Iceoryx::init();

    waitset.attachState(m_subscriber, iox::popo::SubscriberState::HAS_DATA).or_else([](auto) {
        std::cerr << "failed to attach subscriber" << std::endl;
        std::exit(EXIT_FAILURE);
    });
}

PerfTopic IceoryxWait::receivePerfTopic() noexcept
{
    PerfTopic receivedSample;

    auto notificationVector = waitset.wait();
    for (auto& notification : notificationVector)
    {
        if (notification->doesOriginateFrom(&m_subscriber))
        {
            m_subscriber.take().and_then([&](const void* data) {
                receivedSample = *(static_cast<const PerfTopic*>(data));
                m_subscriber.release(data);
            });
        }
    }

    return receivedSample;
}
