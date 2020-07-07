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

#ifndef IOX_DDS_INTERNAL_GATEWAY_DDS_TO_IOX_INL
#define IOX_DDS_INTERNAL_GATEWAY_DDS_TO_IOX_INL

#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/string.hpp"

#include "iceoryx_dds/gateway/dds_to_iox.hpp"

namespace iox
{
namespace dds
{
template <typename channel_t, typename gateway_t>
inline DDS2IceoryxGateway<channel_t, gateway_t>::DDS2IceoryxGateway() noexcept
    : gateway_t()
{
}

template <typename channel_t, typename gateway_t>
inline void DDS2IceoryxGateway<channel_t, gateway_t>::loadConfiguration(const GatewayConfig& config) noexcept
{
    iox::LogDebug() << "[DDS2IceoryxGateway] Configuring gateway.";
    for (const auto& service : config.m_configuredServices)
    {
        if (!this->findChannel(service.m_serviceDescription).has_value())
        {
            setupChannel(service.m_serviceDescription, service.m_dataSize);
        }
    }
}

template <typename channel_t, typename gateway_t>
inline void DDS2IceoryxGateway<channel_t, gateway_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    /// @note not implemented - requires dds discovery which is currently not implemented in the used dds stack.
}

template <typename channel_t, typename gateway_t>
inline void DDS2IceoryxGateway<channel_t, gateway_t>::forward(const channel_t& channel) noexcept
{

    auto publisher = channel.getIceoryxTerminal();
    auto reader = channel.getDDSTerminal();

    auto peekResult = reader->peekNext();
    if(peekResult.has_value())
    {
        // reserve a chunk for the sample
        auto size = peekResult.value();

        m_reservedChunk = publisher->allocateChunk(static_cast<uint32_t>(size));

        // read sample into reserved chunk
        auto buffer = static_cast<uint8_t*>(m_reservedChunk);
        auto takeResult = reader->takeNext(buffer, size);
        if(takeResult.has_error())
        {
            LogWarn() << "[DDS2IceoryxGateway] Encountered error reading from DDS network: " << iox::dds::DataReaderErrorString[static_cast<uint8_t>(takeResult.get_error())];
        }

        // publish the sample
        publisher->sendChunk(buffer);
    }

}

// ======================================== Private ======================================== //
template <typename channel_t, typename gateway_t>
iox::cxx::expected<channel_t, iox::dds::GatewayError>
DDS2IceoryxGateway<channel_t, gateway_t>::setupChannel(const iox::capro::ServiceDescription& service,
                                                       const uint64_t& sampleSize) noexcept
{
    return this->addChannel(service, sampleSize)
        .on_success([&service](iox::cxx::expected<channel_t, iox::dds::GatewayError> result) {
            auto channel = result.get_value();
            auto publisher = channel.getIceoryxTerminal();
            auto reader = channel.getDDSTerminal();
            publisher->offer();
            reader->connect();
            iox::LogDebug() << "[DDS2IceoryxGateway] Setup channel for service: {" << service.getServiceIDString()
                            << ", " << service.getInstanceIDString() << ", " << service.getEventIDString() << "}";
        });
}

} // namespace dds
} // namespace iox

#endif
