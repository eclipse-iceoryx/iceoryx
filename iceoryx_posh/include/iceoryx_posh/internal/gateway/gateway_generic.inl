// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
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

#ifndef IOX_POSH_GW_GATEWAY_GENERIC_INL
#define IOX_POSH_GW_GATEWAY_GENERIC_INL

#include "iceoryx_dust/cxx/file_reader.hpp"
#include "iceoryx_posh/gateway/gateway_generic.hpp"
#include "iox/logging.hpp"

// ================================================== Public ================================================== //

namespace iox
{
namespace gw
{
template <typename channel_t, typename gateway_t>
inline GatewayGeneric<channel_t, gateway_t>::~GatewayGeneric() noexcept
{
    shutdown();
}

template <typename channel_t, typename gateway_t>
inline void GatewayGeneric<channel_t, gateway_t>::runMultithreaded() noexcept
{
    m_isRunning.store(true);
    m_discoveryThread = std::thread([this] { this->discoveryLoop(); });
    m_forwardingThread = std::thread([this] { this->forwardingLoop(); });
}

template <typename channel_t, typename gateway_t>
inline void GatewayGeneric<channel_t, gateway_t>::shutdown() noexcept
{
    m_isRunning.store(false);
    if (m_discoveryThread.joinable())
    {
        m_discoveryThread.join();
    }
    if (m_forwardingThread.joinable())
    {
        m_forwardingThread.join();
    }
}

template <typename channel_t, typename gateway_t>
inline uint64_t GatewayGeneric<channel_t, gateway_t>::getNumberOfChannels() const noexcept
{
    return m_channels->size();
}

// ================================================== Protected ================================================== //

template <typename channel_t, typename gateway_t>
inline GatewayGeneric<channel_t, gateway_t>::GatewayGeneric(capro::Interfaces interface,
                                                            units::Duration discoveryPeriod,
                                                            units::Duration forwardingPeriod) noexcept
    : gateway_t(interface)
    , m_discoveryPeriod(discoveryPeriod)
    , m_forwardingPeriod(forwardingPeriod)
{
}

template <typename channel_t, typename gateway_t>
template <typename IceoryxPubSubOptions>
inline expected<channel_t, GatewayError>
GatewayGeneric<channel_t, gateway_t>::addChannel(const capro::ServiceDescription& service,
                                                 const IceoryxPubSubOptions& options) noexcept
{
    // Filter out wildcard services
    if (service.getServiceIDString() == capro::IdString_t(TruncateToCapacity, "*")
        || service.getInstanceIDString() == capro::IdString_t(TruncateToCapacity, "*")
        || service.getEventIDString() == capro::IdString_t(TruncateToCapacity, "*"))
    {
        return err(GatewayError::UNSUPPORTED_SERVICE_TYPE);
    }

    // Return existing channel if one for the service already exists, otherwise create a new one
    auto existingChannel = findChannel(service);
    if (existingChannel.has_value())
    {
        return ok(existingChannel.value());
    }
    else
    {
        auto result = channel_t::create({service.getServiceIDString(),
                                         service.getInstanceIDString(),
                                         service.getEventIDString(),
                                         {0U, 0U, 0U, 0U},
                                         this->getInterface()},
                                        options);
        if (result.has_error())
        {
            return err(GatewayError::UNSUCCESSFUL_CHANNEL_CREATION);
        }
        else
        {
            auto channel = result.value();
            m_channels->push_back(channel);
            return ok(channel);
        }
    }
}

template <typename channel_t, typename gateway_t>
inline optional<channel_t>
GatewayGeneric<channel_t, gateway_t>::findChannel(const iox::capro::ServiceDescription& service) const noexcept
{
    auto guardedVector = this->m_channels.getScopeGuard();
    auto channel = std::find_if(guardedVector->begin(), guardedVector->end(), [&service](const channel_t& channel) {
        return channel.getServiceDescription() == service;
    });
    if (channel == guardedVector->end())
    {
        return nullopt_t();
    }
    else
    {
        return make_optional<channel_t>(*channel);
    }
}

template <typename channel_t, typename gateway_t>
inline void GatewayGeneric<channel_t, gateway_t>::forEachChannel(const function_ref<void(channel_t&)> f) const noexcept
{
    auto guardedVector = m_channels.getScopeGuard();
    for (auto channel = guardedVector->begin(); channel != guardedVector->end(); ++channel)
    {
        f(*channel);
    }
}

template <typename channel_t, typename gateway_t>
inline expected<void, GatewayError>
GatewayGeneric<channel_t, gateway_t>::discardChannel(const capro::ServiceDescription& service) noexcept
{
    auto guardedVector = this->m_channels.getScopeGuard();
    auto channel = std::find_if(guardedVector->begin(), guardedVector->end(), [&service](const channel_t& channel) {
        return channel.getServiceDescription() == service;
    });
    if (channel != guardedVector->end())
    {
        guardedVector->erase(channel);
        return ok();
    }
    else
    {
        return err(GatewayError::NONEXISTANT_CHANNEL);
    }
}

// ================================================== Private ================================================== //

template <typename channel_t, typename gateway_t>
inline void GatewayGeneric<channel_t, gateway_t>::discoveryLoop() noexcept
{
    while (m_isRunning.load(std::memory_order_relaxed))
    {
        auto startTime = std::chrono::steady_clock::now();
        capro::CaproMessage msg;
        while (this->getCaProMessage(msg))
        {
            discover(msg);
        }
        std::this_thread::sleep_until(startTime + std::chrono::milliseconds(m_discoveryPeriod.toMilliseconds()));
    }
}

template <typename channel_t, typename gateway_t>
inline void GatewayGeneric<channel_t, gateway_t>::forwardingLoop() noexcept
{
    while (m_isRunning.load(std::memory_order_relaxed))
    {
        auto startTime = std::chrono::steady_clock::now();
        forEachChannel([this](channel_t channel) { this->forward(channel); });
        std::this_thread::sleep_until(startTime + std::chrono::milliseconds(m_forwardingPeriod.toMilliseconds()));
    };
}

} // namespace gw
} // namespace iox

#endif // IOX_POSH_GW_GATEWAY_GENERIC_INL
