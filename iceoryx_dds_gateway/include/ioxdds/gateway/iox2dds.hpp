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
#include <unordered_map>

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
    using SubscriberPtr = std::unique_ptr<subscriber_t, std::function<void(subscriber_t*)>>;
    using SubscriberFactory = std::function<SubscriberPtr(const iox::capro::ServiceDescription)>;
    using SubscriberPool = iox::cxx::ObjectPool<subscriber_t, MAX_PORT_NUMBER>;

    using DataWriterPtr = std::unique_ptr<data_writer_t, std::function<void(data_writer_t*)>>;
    using DataWriterFactory =
        std::function<DataWriterPtr(const iox::dds::IdString, const iox::dds::IdString, const iox::dds::IdString)>;
    using DataWriterPool = iox::cxx::ObjectPool<data_writer_t, MAX_PORT_NUMBER>;

  public:
    Iceoryx2DDSGateway();
    ~Iceoryx2DDSGateway();

    ///
    /// @brief Iceoryx2DDSGateway Enables injection of mocks during testing.
    /// @param subscriberFactory Factory that shall be used by the gateway to create Subscriber objects.
    /// @param dataWriterFactory Factory that shall be used by the gateway to create DataWriter objects.
    ///
    Iceoryx2DDSGateway(SubscriberFactory subscriberFactory, DataWriterFactory dataWriterFactory);
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
    /// @brief getNumberOfSubscribers Get the number of subscribers used by the gateway.
    /// @return The number of used subscribers.
    ///
    uint64_t getNumberOfSubscribers() const noexcept;

    ///
    /// @brief getNumberOfDataWriters Get the number of data writers used by the gateway.
    /// @return The number of used data writers.
    ///
    uint64_t getNumberOfDataWriters() const noexcept;

    ///
    /// @brief shutdown the gateway, stopping all threads
    ///
    void shutdown() noexcept;

  private:
    // TODO: Move this to some central gateway configuration somewhere
    static constexpr size_t s_subscriberCacheSize{128};

    std::atomic_bool m_runForwardingLoop{false};
    std::atomic_bool m_runDiscoveryLoop{false};

    SubscriberFactory m_subscriberFactory;
    DataWriterFactory m_dataWriterFactory;

    // Fixed memory pools to store subscribers & data writers. Required to avoid dynamic memory allocation.
    // Elements in these pools should not be directly accessed, but should instead be accessed
    // via smart pointers defined above (which provides a custom deleter).
    // This ensures memory is automatically "freed" back to the pool when no longer needed.
    SubscriberPool m_subscriberPool = SubscriberPool();
    DataWriterPool m_dataWriterPool = DataWriterPool();

    // References to objects in the fixed memory pools.
    // Vector semantics may be used, allowing objects to be logically reordered without changing their physical
    // memory location.
    // This is a little deceiving, as vectors imply a particular in-memory ordering, however there is no
    // "List" implementation provided in iox.
    iox::cxx::vector<SubscriberPtr, MAX_PORT_NUMBER> m_subscribers;
    iox::cxx::vector<DataWriterPtr, MAX_PORT_NUMBER> m_writers;

    subscriber_t& addSubscriber(const iox::capro::ServiceDescription& service) noexcept;
    void removeSubscriber(const iox::capro::ServiceDescription& service) noexcept;
    data_writer_t& addDataWriter(const iox::capro::ServiceDescription& service) noexcept;
    void removeDataWriter(const iox::capro::ServiceDescription& service) noexcept;
};

} // dds
} // gateway
} // iox

#include "ioxdds/internal/gateway/iox2dds.inl"
