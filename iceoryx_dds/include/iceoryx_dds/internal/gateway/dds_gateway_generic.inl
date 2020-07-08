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

#ifndef IOX_DDS_INTERNAL_GATEWAY_DDS_GATEWAY_GENERIC_INL
#define IOX_DDS_INTERNAL_GATEWAY_DDS_GATEWAY_GENERIC_INL

#include "iceoryx_dds/dds/dds_types.hpp"
#include "iceoryx_dds/internal/log/logging.hpp"
#include "iceoryx_utils/internal/file_reader/file_reader.hpp"

// ================================================== Public ================================================== //

template <typename channel_t, typename gateway_t>
inline iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::~DDSGatewayGeneric() noexcept
{
    shutdown();
}

template <typename channel_t, typename gateway_t>
inline void iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::runMultithreaded() noexcept
{
    m_discoveryThread = std::thread([this] { this->discoveryLoop(); });
    m_forwardingThread = std::thread([this] { this->forwardingLoop(); });
    m_isRunning.store(true, std::memory_order_relaxed);
}

template <typename channel_t, typename gateway_t>
inline void iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::shutdown() noexcept
{
    if (m_isRunning.load(std::memory_order_relaxed))
    {
        iox::dds::LogDebug() << "[DDSGatewayGeneric] Shutting down Posh2DDSGateway.";

        m_isRunning.store(false, std::memory_order_relaxed);

        m_discoveryThread.join();
        m_forwardingThread.join();
    }
}

template <typename channel_t, typename gateway_t>
inline uint64_t iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::getNumberOfChannels() const noexcept
{
    return m_channels->size();
}

// ================================================== Protected ================================================== //

template <typename channel_t, typename gateway_t>
inline iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::DDSGatewayGeneric() noexcept
    : gateway_t(iox::capro::Interfaces::DDS)
{
    LogDebug() << "[DDSGatewayGeneric] Using default channel factory.";
}

template <typename channel_t, typename gateway_t>
inline iox::cxx::expected<channel_t, iox::dds::GatewayError>
iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::addChannel(const iox::capro::ServiceDescription& service) noexcept
{
    // Filter out wildcard services
    if (service.getServiceID() == iox::capro::AnyService || service.getInstanceID() == iox::capro::AnyInstance
        || service.getEventID() == iox::capro::AnyEvent)
    {
        return iox::cxx::error<GatewayError>(GatewayError::UNSUPPORTED_SERVICE_TYPE);
    }

    // Return existing channel if one for the service already exists, otherwise create a new one
    auto existingChannel = findChannel(service);
    if (existingChannel.has_value())
    {
        return iox::cxx::success<channel_t>(existingChannel.value());
    }
    else
    {
        auto result = channel_t::create(service);
        if (result.has_error())
        {
            iox::dds::LogError() << "[DDSGatewayGeneric] Unable to set up channel for service: "
                                 << "/" << service.getServiceIDString() << "/" << service.getInstanceIDString() << "/"
                                 << service.getEventIDString();
            return iox::cxx::error<GatewayError>(GatewayError::UNSUCCESSFUL_CHANNEL_CREATION);
        }
        else
        {
            auto channel = result.get_value();
            m_channels->push_back(channel);
            iox::dds::LogDebug() << "[DDSGatewayGeneric] Channel set up for service: "
                                 << "/" << service.getServiceIDString() << "/" << service.getInstanceIDString() << "/"
                                 << service.getEventIDString();
            return iox::cxx::success<channel_t>(channel);
        }
    }
}

template <typename channel_t, typename gateway_t>
inline iox::cxx::optional<channel_t>
iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::findChannel(const iox::capro::ServiceDescription& service) const
    noexcept
{
    auto guardedVector = this->m_channels.GetScopeGuard();
    auto channel = std::find_if(guardedVector->begin(), guardedVector->end(), [&service](const channel_t& channel) {
        return channel.getServiceDescription() == service;
    });
    if (channel == guardedVector->end())
    {
        return iox::cxx::nullopt_t();
    }
    else
    {
        return iox::cxx::make_optional<channel_t>(*channel);
    }
}

template <typename channel_t, typename gateway_t>
inline void iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::forEachChannel(
    const iox::cxx::function_ref<void(channel_t&)> f) const noexcept
{
    auto guardedVector = m_channels.GetScopeGuard();
    for (auto channel = guardedVector->begin(); channel != guardedVector->end(); ++channel)
    {
        f(*channel);
    }
}

template <typename channel_t, typename gateway_t>
inline iox::cxx::expected<iox::dds::GatewayError> iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::discardChannel(
    const iox::capro::ServiceDescription& service) noexcept
{
    auto guardedVector = this->m_channels.GetScopeGuard();
    auto channel = std::find_if(guardedVector->begin(), guardedVector->end(), [&service](const channel_t& channel) {
        return channel.getServiceDescription() == service;
    });
    if (channel != guardedVector->end())
    {
        guardedVector->erase(channel);
        iox::dds::LogDebug() << "[DDSGatewayGeneric] Channel taken down for service: "
                             << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                             << service.getEventIDString();
        return iox::cxx::success<void>();
    }
    else
    {
        return iox::cxx::error<GatewayError>(GatewayError::NONEXISTANT_CHANNEL);
    }
}

// ================================================== Private ================================================== //

template <typename channel_t, typename gateway_t>
inline void iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::discoveryLoop() noexcept
{
    iox::dds::LogDebug() << "[DDSGatewayGeneric] Starting discovery.";
    while (m_isRunning.load(std::memory_order_relaxed))
    {
        auto startTime = std::chrono::steady_clock::now();
        iox::capro::CaproMessage msg;
        while (this->getCaProMessage(msg))
        {
            discover(msg);
        }
        std::this_thread::sleep_until(startTime + std::chrono::milliseconds(DISCOVERY_PERIOD.milliSeconds<int64_t>()));
    }
    iox::dds::LogDebug() << "[DDSGatewayGeneric] Stopped discovery.";
}

template <typename channel_t, typename gateway_t>
inline void iox::dds::DDSGatewayGeneric<channel_t, gateway_t>::forwardingLoop() noexcept
{
    iox::dds::LogDebug() << "[DDSGatewayGeneric] Starting forwarding.";
    while (m_isRunning.load(std::memory_order_relaxed))
    {
        auto startTime = std::chrono::steady_clock::now();
        forEachChannel([this](channel_t channel) { this->forward(channel); });
        std::this_thread::sleep_until(startTime + std::chrono::milliseconds(FORWARDING_PERIOD.milliSeconds<int64_t>()));
    };
    iox::dds::LogDebug() << "[DDSGatewayGeneric] Stopped forwarding.";
}

#endif
