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

#include <chrono>
#include <thread>

#include <iceoryx_posh/mepoo/chunk_header.hpp>

#include "ioxdds/internal/log/logging.hpp"

#include "ioxdds/gateway/iox_to_dds.hpp"

namespace iox
{
namespace dds
{
// ======================================== Public ======================================== //
template <typename channel_t>
inline Iceoryx2DDSGateway<channel_t>::Iceoryx2DDSGateway() noexcept : iox::dds::DDSGatewayGeneric<channel_t>()
{
    // Create channels for all pre-configured services.
    this->loadConfiguration();

    // Connect the terminals of the created channels.
    for (auto channel = this->m_channels->begin(); channel != this->m_channels->end(); channel++)
    {
        auto subscriber = channel->getIceoryxTerminal();
        auto dataWriter = channel->getDDSTerminal();
        subscriber->subscribe(SUBSCRIBER_CACHE_SIZE);
        dataWriter->connect();
    }
}

template <typename channel_t>
inline void
Iceoryx2DDSGateway<channel_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] <CaproMessage> "
                    << iox::capro::CaproMessageTypeString[static_cast<uint8_t>(msg.m_type)]
                    << " { Service: " << msg.m_serviceDescription.getServiceIDString()
                    << ", Instance: " << msg.m_serviceDescription.getInstanceIDString()
                    << ", Event: " << msg.m_serviceDescription.getEventIDString() << " }";

    if (msg.m_serviceDescription.getServiceIDString() == iox::capro::IdString("Introspection"))
    {
        return;
    }
    if (msg.m_subType == iox::capro::CaproMessageSubType::SERVICE)
    {
        return;
    }

    switch (msg.m_type)
    {
    case iox::capro::CaproMessageType::OFFER:
    {
        // Check if channel already exists using a predicate that checks the channel service desription.
        if(std::find_if(
                this->m_channels->begin(),
                this->m_channels->end(),
                [&msg](const channel_t& channel) {
                    return channel.getService() == msg.m_serviceDescription;
                }) == this->m_channels->end())
        {
            auto channel = this->setupChannel(msg.m_serviceDescription);
            auto subscriber = channel.getIceoryxTerminal();
            auto dataWriter = channel.getDDSTerminal();
            subscriber->subscribe(SUBSCRIBER_CACHE_SIZE);
            dataWriter->connect();
        }
        break;
    }
    case iox::capro::CaproMessageType::STOP_OFFER:
    {
        this->discardChannel(msg.m_serviceDescription);
        break;
    }
    default:
    {
        break;
    }
    }
}

template <typename channel_t>
inline void Iceoryx2DDSGateway<channel_t>::forward() noexcept
{
    auto guardedVector = this->m_channels.GetScopeGuard();
    for (auto channel = guardedVector->begin(); channel != guardedVector->end(); channel++)
    {
        auto subscriber = channel->getIceoryxTerminal();
        auto dataWriter = channel->getDDSTerminal();
        if (subscriber->hasNewChunks())
        {
            const iox::mepoo::ChunkHeader* header;
            subscriber->getChunk(&header);
            if (header->m_info.m_payloadSize > 0)
            {
                dataWriter->write(static_cast<uint8_t*>(header->payload()), header->m_info.m_payloadSize);
            }
            subscriber->releaseChunk(header);
        }
    }
}

} // namespace dds
} // namespace iox
