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
#include <functional>
#include <memory>
#include <chrono>

#include <iceoryx_posh/iceoryx_posh_types.hpp>
#include <iceoryx_posh/popo/gateway_generic.hpp>
#include <iceoryx_posh/popo/subscriber.hpp>
#include <iceoryx_posh/runtime/posh_runtime.hpp>
#include <iceoryx_utils/cxx/vector.hpp>
#include <iceoryx_utils/internal/objectpool/objectpool.hpp>

#include "ioxdds/dds/data_writer.hpp"
#include "ioxdds/dds/dds_types.hpp"

namespace iox
{
namespace gateway
{
namespace dds
{

// Configuration Parameters
static constexpr uint32_t DISCOVERY_PERIOD_MS = 1000;
static constexpr uint32_t FORWARDING_PERIOD_MS = 50;
static constexpr uint32_t MAX_CHANNEL_NUMBER = MAX_PORT_NUMBER;
static constexpr uint32_t SUBSCRIBER_CACHE_SIZE = 128;

///
/// @brief Groups and manages resources that make up a posh->dds channel
///
template <typename subscriber_t = iox::popo::Subscriber,
          typename data_writer_t = iox::dds::data_writer_t>
class Channel {
public:
    using SubscriberPtr = std::shared_ptr<subscriber_t>;
    using SubscriberPool = iox::cxx::ObjectPool<subscriber_t, MAX_CHANNEL_NUMBER>;
    using DataWriterPtr = std::shared_ptr<data_writer_t>;
    using DataWriterPool = iox::cxx::ObjectPool<data_writer_t, MAX_CHANNEL_NUMBER>;

    ///
    /// @brief Channel Constructs an object with unmanaged resources.
    /// @param service The service that the channel is connecting.
    /// @param subscriber An externally managed subscriber endpoint.
    /// @param dataWriter An externally managed data writer endpoint.
    ///
    Channel(const iox::capro::ServiceDescription& service, SubscriberPtr subscriber, DataWriterPtr dataWriter);

    ///
    /// @brief create Creates a channel with internally managed resources.
    /// @param service The service that the channel is connecting.
    /// @return Channel A channel with internally managed endpoints.
    ///
    static Channel create(const iox::capro::ServiceDescription& service);

    iox::capro::ServiceDescription getService();
    SubscriberPtr getSubscriber();
    DataWriterPtr getDataWriter();

private:
    // Store in data segment - too large to keep in stack.
    static SubscriberPool s_subscriberPool;
    static DataWriterPool s_dataWriterPool;

    iox::capro::ServiceDescription service;
    SubscriberPtr subscriber;
    DataWriterPtr dataWriter;
};

///
/// @brief A Gateway to support internode communication between iceoryx nodes in a DDS network.
///
/// Forwards data published in a local iceoryx system to an attached DDS network.
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
    /// via its interface port, specifically watching for changes related to SenderPorts.
    /// When SenderPorts are added to or removed from the system, the gateway will set up the
    /// required infrastructure to forward the data to the DDS network.
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
    /// Forwards all data received via ReceiverPorts to the DDS network.
    /// A dedicated thread is recommended for this loop.
    ///
    void forwardingLoop() noexcept;

    ///
    /// @brief forward Forwards all new data points available on the gateways receiver
    /// ports to the DDS network.
    ///
    void forward() noexcept;

    ///
    /// @brief getNumberOfChannels Get the number of active channels.
    /// @return The number of active channels.
    ///
    size_t getNumberOfChannels() noexcept;

    ///
    /// @brief shutdown the gateway, stopping all threads
    ///
    void shutdown() noexcept;

  private:

    std::atomic_bool m_runForwardingLoop{false};
    std::atomic_bool m_runDiscoveryLoop{false};

    ChannelFactory m_channelFactory;

    std::mutex m_channelAccessMutex;
    iox::cxx::vector<Channel<subscriber_t, data_writer_t>, MAX_CHANNEL_NUMBER> m_channels;

    ///
    /// @brief setupChannelUnsafe Creates a new channel for the given service without any synchronization.
    /// @param service The service for which a channel will be established.
    /// @return Channel object with subscriber and data writer for the given service.
    ///
    Channel<subscriber_t, data_writer_t> setupChannelUnsafe(const iox::capro::ServiceDescription& service);

    ///
    /// @brief takeDownChannelUnsafe Discards the channel for the given service without any synchonization.
    /// @param service The service for which a channel will be discarded.
    ///
    void takeDownChannelUnsafe(const iox::capro::ServiceDescription& service);

};

} // dds
} // gateway
} // iox

#include "ioxdds/internal/gateway/iox2dds.inl"
