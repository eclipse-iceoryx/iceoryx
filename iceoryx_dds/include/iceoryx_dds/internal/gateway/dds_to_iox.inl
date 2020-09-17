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

#ifndef IOX_DDS_DDS_TO_IOX_INL
#define IOX_DDS_DDS_TO_IOX_INL

#include "iceoryx_dds/dds/dds_config.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/string.hpp"

namespace iox
{
namespace dds
{
template <typename channel_t, typename gateway_t>
inline DDS2IceoryxGateway<channel_t, gateway_t>::DDS2IceoryxGateway() noexcept
    : gateway_t(capro::Interfaces::DDS, DISCOVERY_PERIOD, FORWARDING_PERIOD)
{
}

template <typename channel_t, typename gateway_t>
inline void DDS2IceoryxGateway<channel_t, gateway_t>::loadConfiguration(const config::GatewayConfig& config) noexcept
{
    LogDebug() << "[DDS2IceoryxGateway] Configuring gateway...";
    for (const auto& service : config.m_configuredServices)
    {
        if (!this->findChannel(service.m_serviceDescription).has_value())
        {
            auto serviceDescription = service.m_serviceDescription;
            LogDebug() << "[DDS2IceoryxGateway] Setting up channel for service: {"
                       << serviceDescription.getServiceIDString() << ", " << serviceDescription.getInstanceIDString()
                       << ", " << serviceDescription.getEventIDString() << "}";
            setupChannel(serviceDescription);
        }
    }
}

template <typename channel_t, typename gateway_t>
inline void DDS2IceoryxGateway<channel_t, gateway_t>::discover([[gnu::unused]] const capro::CaproMessage& msg) noexcept
{
    /// @note not implemented - requires dds discovery which is currently not implemented in the used dds stack.
}

template <typename channel_t, typename gateway_t>
inline void DDS2IceoryxGateway<channel_t, gateway_t>::forward(const channel_t& channel) noexcept
{
    auto publisher = channel.getIceoryxTerminal();
    auto reader = channel.getExternalTerminal();

    reader->peekNextSize().and_then([&](uint64_t size) {
        // reserve a chunk for the sample
        m_reservedChunk = publisher->allocateChunk(static_cast<uint32_t>(size));
        // read sample into reserved chunk
        auto buffer = static_cast<uint8_t*>(m_reservedChunk);
        reader->takeNext(buffer, size)
            .and_then([&]() {
                // publish chunk
                publisher->sendChunk(buffer);
            })
            .or_else([&](DataReaderError err) {
                LogWarn() << "[DDS2IceoryxGateway] Encountered error reading from DDS network: "
                          << dds::DataReaderErrorString[static_cast<uint8_t>(err)];
            });
    });
}

// ======================================== Private ======================================== //
template <typename channel_t, typename gateway_t>
cxx::expected<channel_t, gw::GatewayError>
DDS2IceoryxGateway<channel_t, gateway_t>::setupChannel(const capro::ServiceDescription& service) noexcept
{
    return this->addChannel(service).and_then([&service](channel_t channel) {
        auto publisher = channel.getIceoryxTerminal();
        auto reader = channel.getExternalTerminal();
        publisher->offer();
        reader->connect();
        iox::LogDebug() << "[DDS2IceoryxGateway] Setup channel for service: {" << service.getServiceIDString() << ", "
                        << service.getInstanceIDString() << ", " << service.getEventIDString() << "}";
    });
}

} // namespace dds
} // namespace iox

#endif // IOX_DDS_DDS_TO_IOX_INL
