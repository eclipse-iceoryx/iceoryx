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

#ifndef IOX_DDS_INTERNAL_GATEWAY_IOX_TO_DDS_INL
#define IOX_DDS_INTERNAL_GATEWAY_IOX_TO_DDS_INL

#include "eth/eth_config.hpp"
#include "logging.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/gateway/gateway_config.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"

#include "gateway/iox_to_eth.hpp"

namespace iox
{
namespace eth
{
// ======================================== Public ======================================== //
template <typename channel_t, typename gateway_t>
inline Iceoryx2ethGateway<channel_t, gateway_t>::Iceoryx2ethGateway() noexcept
    : gateway_t(iox::capro::Interfaces::DDS, DISCOVERY_PERIOD, FORWARDING_PERIOD)
{
}
template <typename channel_t, typename gateway_t>
inline Iceoryx2ethGateway<channel_t, gateway_t>::~Iceoryx2ethGateway() noexcept
{
}
template <typename channel_t, typename gateway_t>
inline void Iceoryx2ethGateway<channel_t, gateway_t>::loadConfiguration(const iox::config::GatewayConfig& config) noexcept
{
    iox::LogDebug() << "[Iceoryx2ethGateway] Configuring gateway...";
    for (const auto& service : config.m_configuredServices)
    {
        if (!this->findChannel(service.m_serviceDescription).has_value())
        {
            auto serviceDescription =  service.m_serviceDescription;
            iox::LogDebug() << "[DDS2IceoryxGateway] Setting up channel for service: {"
                            << serviceDescription.getServiceIDString() << ", "
                            << serviceDescription.getInstanceIDString() << ", "
                            << serviceDescription.getEventIDString() << "}";
            setupChannel(serviceDescription);
        }
    }
}

template <typename channel_t, typename gateway_t>
inline void Iceoryx2ethGateway<channel_t, gateway_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    iox::LogDebug() << "[Iceoryx2ethGateway] <CaproMessage> "
                    << iox::capro::CaproMessageTypeString[static_cast<uint8_t>(msg.m_type)]
                    << " { Service: " << msg.m_serviceDescription.getServiceIDString()
                    << ", Instance: " << msg.m_serviceDescription.getInstanceIDString()
                    << ", Event: " << msg.m_serviceDescription.getEventIDString() << " }";

    if (msg.m_serviceDescription.getServiceIDString() == iox::capro::IdString(iox::roudi::INTROSPECTION_SERVICE_ID))
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
        if (!this->findChannel(msg.m_serviceDescription).has_value())
        {
            setupChannel(msg.m_serviceDescription);
            std::cout << "Setting up a channel" << std::endl ;
        }
        break;
    }
    case iox::capro::CaproMessageType::STOP_OFFER:
    {
        if (this->findChannel(msg.m_serviceDescription).has_value())
        {
            this->discardChannel(msg.m_serviceDescription);
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

template <typename channel_t, typename gateway_t>
inline void Iceoryx2ethGateway<channel_t, gateway_t>::forward(const channel_t& channel) noexcept
{
    auto subscriber = channel.getIceoryxTerminal();
    while (subscriber->hasNewChunks())
    {
        const iox::mepoo::ChunkHeader* header;
        subscriber->getChunk(&header);
        if (header->m_info.m_payloadSize > 0)
        {
            auto dataWriter = channel.getExternalTerminal();
            dataWriter->write(static_cast<uint8_t*>(header->payload()), header->m_info.m_payloadSize);
            std::cout << "Data received from ICE" << std::endl ;
        }
        subscriber->releaseChunk(header);
    }
}

// ======================================== Private ======================================== //

template <typename channel_t, typename gateway_t>
iox::cxx::expected<channel_t, iox::gw::GatewayError>
Iceoryx2ethGateway<channel_t, gateway_t>::setupChannel(const iox::capro::ServiceDescription& service) noexcept
{
    return this->addChannel(service).and_then([](channel_t channel) {
        auto subscriber = channel.getIceoryxTerminal();
        auto dataWriter = channel.getExternalTerminal();
        subscriber->subscribe(SUBSCRIBER_CACHE_SIZE);
        dataWriter->connect();

        std::cout <<"Address of getExternalTerminal" << &dataWriter << std::endl;
    });
}

} // namespace eth
} // namespace iox

#endif
