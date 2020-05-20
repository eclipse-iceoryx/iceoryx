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
#include <chrono>
#include <functional>
#include <memory>

#include <iceoryx_posh/iceoryx_posh_types.hpp>
#include <iceoryx_posh/popo/gateway_generic.hpp>
#include <iceoryx_posh/popo/subscriber.hpp>
#include <iceoryx_posh/runtime/posh_runtime.hpp>
#include <iceoryx_utils/cxx/vector.hpp>
#include <iceoryx_utils/internal/objectpool/objectpool.hpp>

#include "ioxdds/dds/data_writer.hpp"
#include "ioxdds/dds/dds_types.hpp"
#include "ioxdds/gateway/channel.hpp"

namespace iox
{
namespace dds
{
// Configuration Parameters
constexpr units::Duration DISCOVERY_PERIOD = 1000_ms;
constexpr units::Duration FORWARDING_PERIOD = 50_ms;
constexpr uint32_t SUBSCRIBER_CACHE_SIZE = 128;

///
/// @brief A Gateway to support internode communication between iceoryx nodes in a DDS network.
///
/// Forwards data published in a local posh runtime to an attached DDS network.
///
template <typename gateway_t = iox::popo::GatewayGeneric,
          typename subscriber_t = iox::popo::Subscriber,
          typename data_writer_t = iox::dds::data_writer_t>
class Iceoryx2DDSGateway : gateway_t
{
    using ChannelFactory = std::function<Channel<subscriber_t, data_writer_t>(const iox::capro::ServiceDescription)>;

  public:
    Iceoryx2DDSGateway();
    ~Iceoryx2DDSGateway();

    ///
    /// @brief Iceoryx2DDSGateway Enables injection of mocks during testing.
    /// @param channelFactory Factory method to create channel instances containing mocks.
    ///
    Iceoryx2DDSGateway(ChannelFactory channelFactory);
    Iceoryx2DDSGateway(const Iceoryx2DDSGateway&) = delete;
    Iceoryx2DDSGateway& operator=(const Iceoryx2DDSGateway&) = delete;
    Iceoryx2DDSGateway(Iceoryx2DDSGateway&&) = delete;
    Iceoryx2DDSGateway& operator=(Iceoryx2DDSGateway&&) = delete;

    ///
    /// @brief Starts the discovery loop.
    ///
    /// Periodically check for and process capro messages received
    /// via the GenericGateway.
    /// When new publishers are offered in the system, the required DDS components for
    /// data fotwarding are initialized. Converseley, when publishers stop offering, these
    /// components are destroyed.
    ///
    /// A dedicated thread is recommended for this loop.
    ///
    void discoveryLoop() noexcept;

    ///
    /// @brief discover Run discovery logic for the given CaproMessage.
    /// @param msg
    ///
    void discover(const iox::capro::CaproMessage& msg) noexcept;

    ///
    /// @brief Starts the data forwarding loop.
    ///
    /// Periodically processes data publsihed by local posh publishers to the DDS network.
    /// A dedicated thread is recommended for this loop.
    ///
    void forwardingLoop() noexcept;

    ///
    /// @brief forward Forwarding logic..
    ///
    void forward() noexcept;

    ///
    /// @brief getNumberOfChannels Get the number of active channels.
    /// @return The number of active channels.
    ///
    uint64_t getNumberOfChannels() noexcept;

    ///
    /// @brief shutdown the gateway, stopping all threads
    ///
    void shutdown() noexcept;

  private:
    std::atomic_bool m_runForwardingLoop{false};
    std::atomic_bool m_runDiscoveryLoop{false};

    ChannelFactory m_channelFactory;

    // This mutex is required for synchronized access to the channels list.
    std::mutex m_channelAccessMutex;
    iox::cxx::vector<Channel<subscriber_t, data_writer_t>, MAX_CHANNEL_NUMBER> m_channels;

    ///
    /// @brief setupChannelUnsafe Creates a new channel for a service.
    ///
    /// @param service The service for which a channel will be established.
    /// @return Channel object with subscriber and data writer for the given service.
    ///
    Channel<subscriber_t, data_writer_t> setupChannel(const iox::capro::ServiceDescription& service) noexcept;

    ///
    /// @brief discardChannel Discards the channel for the given service.
    ///
    /// @param service The service for which a channel will be discarded.
    ///
    void discardChannel(const iox::capro::ServiceDescription& service) noexcept;
};

} // dds
} // iox

#include "ioxdds/internal/gateway/iox2dds.inl"
