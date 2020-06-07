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

#pragma once

#include <atomic>
#include <thread>

#include <iceoryx_posh/capro/service_description.hpp>
#include <iceoryx_posh/iceoryx_posh_types.hpp>
#include <iceoryx_posh/popo/gateway_generic.hpp>
#include <iceoryx_utils/cxx/optional.hpp>
#include <iceoryx_utils/cxx/string.hpp>
#include <iceoryx_utils/cxx/vector.hpp>
#include <iceoryx_utils/internal/concurrent/smart_lock.hpp>

#include "iceoryx_dds/dds/dds_config.hpp"
#include "iceoryx_dds/gateway/gateway_config.hpp"

namespace iox
{
namespace dds
{
///
/// @brief Base class for DDS gateways containing common logic used by all implementations. Methods that are expected
/// to differ across implementations are left as pure virtual.
///
template <typename channel_t, typename gateway_t = iox::popo::GatewayGeneric>
class DDSGatewayGeneric : public gateway_t
{
    using ChannelFactory = std::function<channel_t(const iox::capro::ServiceDescription)>;
    using ChannelVector = iox::cxx::vector<channel_t, MAX_CHANNEL_NUMBER>;
    using ConcurrentChannelVector = iox::concurrent::smart_lock<ChannelVector>;

  public:
    virtual ~DDSGatewayGeneric() noexcept;

    DDSGatewayGeneric(const DDSGatewayGeneric&) = delete;
    DDSGatewayGeneric& operator=(const DDSGatewayGeneric&) = delete;
    DDSGatewayGeneric(DDSGatewayGeneric&&) = delete;
    DDSGatewayGeneric& operator=(DDSGatewayGeneric&&) = delete;

    void runMultithreaded() noexcept;
    void shutdown() noexcept;

    ///
    /// @brief loadConfiguration Load the provided configuration.
    /// @param config Generic dds gateway configuration which is applicable to all implementations.
    ///
    virtual void loadConfiguration(GatewayConfig config) noexcept = 0;
    ///
    /// @brief discover Process discovery messages coming from iceoryx.
    /// @param msg The discovery message.
    ///
    virtual void discover(const iox::capro::CaproMessage& msg) noexcept = 0;
    ///
    /// @brief forward Forward data between the two terminals of the channel used by the implementation.
    /// @param channel The channel to propogate data across.
    ///
    virtual void forward(channel_t channel) noexcept = 0;

    uint64_t getNumberOfChannels() const noexcept;

  protected:
    DDSGatewayGeneric() noexcept;

    ChannelFactory m_channelFactory;

    ///
    /// @brief addChannel Creates a channel for the given service and stores a copy of it in an internal collection for
    /// later access.
    /// @param service The service to create a channel for.
    /// @return A copy of the created channel.
    ///
    /// @note Channels are supposed to be lightweight, consisting only of pointers to the terminals and a copy of the
    /// service description, therefore a copy is provided to any entity that requires them.
    /// When no more copies of a channel exists in the system, the terminals will automatically be cleaned up via
    /// the custom deleters included in their pointers.
    ///
    /// The service description is perhaps too large for copying since they contain strings, however this should be
    /// addressed with the service description repository feature.
    ///
    channel_t addChannel(const iox::capro::ServiceDescription& service) noexcept;

    ///
    /// @brief findChannel Searches for a channel for the given service in the internally stored collection and returns
    /// it one exists.
    /// \param service The service to find a channel for.
    /// \return An optional containining the matching channel if one exists, otherwise an empty optional.
    ///
    iox::cxx::optional<channel_t> findChannel(const iox::capro::ServiceDescription& service) const noexcept;

    ///
    /// @brief forEachChannel Executs the given function for each channel in the internally stored collection.
    /// @param f The function to execute.
    /// @note This operation allows thread-safe access to the internal collection.
    ///
    void forEachChannel(const std::function<void(channel_t&)> f) const noexcept;

    ///
    /// @brief discardChannel Discard the channel for the given service in the internal collection if one exists.
    /// @param service The service whose channels hiould be discarded.
    ///
    void discardChannel(const iox::capro::ServiceDescription& service) noexcept;

  private:
    ConcurrentChannelVector m_channels;

    std::atomic_bool m_isRunning{false};

    std::thread m_discoveryThread;
    std::thread m_forwardingThread;

    void forwardingLoop() noexcept;
    void discoveryLoop() noexcept;
};

} // namespace dds
} // namespace iox

#include "iceoryx_dds/internal/gateway/dds_gateway_generic.inl"
