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

#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_utils/cxx/string.hpp"

#include "iceoryx_dds/gateway/dds_to_iox.hpp"

namespace iox
{
namespace dds
{

template <typename channel_t>
inline DDS2IceoryxGateway<channel_t>::DDS2IceoryxGateway() noexcept
    : iox::dds::DDSGatewayGeneric<channel_t>()
{
}

template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::loadConfiguration(const GatewayConfig& config) noexcept
{
    iox::LogDebug() << "[DDS2IceoryxGateway] Configuring gateway.";
    for (const auto& service : config.m_configuredServices)
    {
        if (!this->findChannel(service.m_serviceDescription).has_value())
        {
            setupChannel(service.m_serviceDescription);
        }
    }
}

template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    /// @note Not implemented - requires dds discovery which is currently unavailable.
}

template <typename channel_t>
inline void DDS2IceoryxGateway<channel_t>::forward(const channel_t& channel) noexcept
{
    auto publisher = channel.getIceoryxTerminal();
    auto reader = channel.getDDSTerminal();

    // reserve a chunk for initial sample
    if(m_reservedChunk == nullptr)
    {
        m_reservedChunk = publisher->allocateChunk(channel.sampleSize());
    }

    // read exactly one sample into the reserved chunk
    auto buffer = static_cast<uint8_t*>(m_reservedChunk);
    auto const numToRead = 1u;
    auto result = reader->read(buffer, channel.sampleSize(), channel.sampleSize(), numToRead);
    if(!result.has_error())
    {
        auto num = result.get_value();
        if(num == 0)
        {
            // Nothing to do.
        }
        else if(num == 1)
        {
            // publish the sample
            publisher->sendChunk(buffer);
            // reserve a new chunk for the next sample
            m_reservedChunk = publisher->allocateChunk(channel.sampleSize());
        }
        else {
            // sample is corrupt, don't publish.
            LogWarn() << "[DDS2IceoryxGateway] Received corrupt sample. Buffer is larger than expected. Skipping.";
        }
    }
    else{
        LogWarn() << "[DDS2IceoryxGateway] Encountered error reading from DDS network.";
    }
}

// ======================================== Private ======================================== //

template <typename channel_t>
void iox::dds::DDS2IceoryxGateway<channel_t>::setupChannel(const iox::capro::ServiceDescription& service) noexcept
{
    this->addChannel(service).on_success([](iox::cxx::expected<channel_t, iox::dds::GatewayError> result) {
        auto channel = result.get_value();
        auto publisher = channel.getIceoryxTerminal();
        publisher->offer();
        auto reader = channel.getDDSTerminal();
        reader->connect();
    });
}

} // namespace dds
} // namespace iox
