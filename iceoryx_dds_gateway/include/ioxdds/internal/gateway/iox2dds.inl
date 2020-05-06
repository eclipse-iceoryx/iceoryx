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

#include <chrono>
#include <thread>

#include <iceoryx_posh/mepoo/chunk_header.hpp>
#include <iceoryx_utils/cxx/string.hpp>

#include "ioxdds/gateway/iox2dds.hpp"
#include "ioxdds/internal/log/logging.hpp"

namespace iox
{
namespace gateway
{
namespace dds
{
// ======================================== Public ======================================== //
template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::Iceoryx2DDSGateway()
    : gateway_t(iox::capro::Interfaces::DDS)
{
    // Default factory methods.
    // In both cases, objects are created directly in their respective ObjectPool.
    // Custom deleters are set to clear the objects from the pool when there are no longer
    // any references to them.
    m_subscriberFactory =
        [this](iox::capro::ServiceDescription sd) {
            return SubscriberPtr(
                        m_subscriberPool.create(std::forward<iox::capro::ServiceDescription>(sd)),
                        [this](subscriber_t* p) -> void {m_subscriberPool.free(p);});
    };
    m_dataWriterFactory =
        [this](const iox::dds::IdString serviceId, const iox::dds::IdString instanceId,const iox::dds::IdString eventId)
    {
            return DataWriterPtr(
                        m_dataWriterPool.create(
                            std::forward<const iox::dds::IdString>(serviceId),
                            std::forward<const iox::dds::IdString>(instanceId),
                            std::forward<const iox::dds::IdString>(eventId)),
                        [this](data_writer_t* p) -> void {m_dataWriterPool.free(p);});
    };
};

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::Iceoryx2DDSGateway(SubscriberFactory subscriberFactory,
                                                                         DataWriterFactory dataWriterFactory)
    : gateway_t(iox::capro::Interfaces::DDS)
{
    m_subscriberFactory = subscriberFactory;
    m_dataWriterFactory = dataWriterFactory;
};

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::~Iceoryx2DDSGateway()
{
    m_writers.clear();
    m_subscribers.clear();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::discoveryLoop() noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Starting discovery.";
    m_runDiscoveryLoop.store(true, std::memory_order_relaxed);
    while (m_runDiscoveryLoop.load(std::memory_order_relaxed))
    {
        iox::capro::CaproMessage msg;
        while (static_cast<gateway_t*>(this)->getCaProMessage(msg))
        {
            discover(msg);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    iox::LogDebug() << "[Iceoryx2DDSGateway] Stopped discovery.";
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::discover(const iox::capro::CaproMessage& msg) noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] <CaproMessage> "
                    << iox::capro::CaproMessageTypeString[static_cast<uint8_t>(msg.m_type)]
                    << " { Service: " << msg.m_serviceDescription.getServiceIDString()
                    << ", Instance: " << msg.m_serviceDescription.getInstanceIDString()
                    << ", Event: " << msg.m_serviceDescription.getEventIDString() << " }";

    // Ignore instrospection ports
    if (msg.m_serviceDescription.getServiceIDString() == iox::capro::IdString("Introspection"))
    {
        return;
    }
    // Ignore services
    if (msg.m_subType == iox::capro::CaproMessageSubType::SERVICE)
    {
        return;
    }

    switch (msg.m_type)
    {
    case iox::capro::CaproMessageType::OFFER:
    {
        auto subscriber = addSubscriber(msg.m_serviceDescription);
        subscriber->subscribe(s_subscriberCacheSize);
        auto writer = addDataWriter(msg.m_serviceDescription);
        writer->connect();
        break;
    }
    case iox::capro::CaproMessageType::STOP_OFFER:
    {
        removeDataWriter(msg.m_serviceDescription);
        removeSubscriber(msg.m_serviceDescription);
        break;
    }
    default:
    {
        break;
    }
    }
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::forwardingLoop() noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Starting forwarding.";
    m_runForwardingLoop.store(true, std::memory_order_relaxed);
    while (m_runForwardingLoop.load(std::memory_order_relaxed))
    {
        forward();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    };
    iox::LogDebug() << "[Iceoryx2DDSGateway] Stopped forwarding.";
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::forward() noexcept
{
    auto index = 0;
    for (auto const& subscriber : m_subscribers)
    {
        if (subscriber->hasNewChunks())
        {
            const iox::mepoo::ChunkHeader* header;
            subscriber->getChunk(&header);
            if (header->m_info.m_payloadSize > 0)
            {
                m_writers[index]->write(static_cast<uint8_t*>(header->payload()), header->m_info.m_payloadSize);
            }
            subscriber->releaseChunk(header);
        }
        index++;
    }
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline uint64_t Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::getNumberOfSubscribers() const noexcept
{
    return m_subscribers.size();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline uint64_t Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::getNumberOfDataWriters() const noexcept
{
    return m_writers.size();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::shutdown() noexcept
{
    iox::LogDebug() << "[Iceoryx2DDSGateway] Shutting down Posh2DDSGateway.";
    m_runDiscoveryLoop.store(false, std::memory_order_relaxed);
    m_runForwardingLoop.store(false, std::memory_order_relaxed);
}

// ======================================== Private ======================================== //
template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<subscriber_t> Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::addSubscriber(
    const iox::capro::ServiceDescription& service) noexcept
{
    m_subscribers.emplace_back(m_subscriberFactory(service));
    return m_subscribers.back();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::removeSubscriber(
    const iox::capro::ServiceDescription& service) noexcept
{
    for(auto& s : m_subscribers){
        if(s->getServiceDescription() == service){
            m_subscribers.erase(&s);
            iox::LogDebug() << "[Iceoryx2DDSGateway] Destroyed Subscriber for service: "
                            << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                            <<  service.getEventIDString();
            break;
        }
    }
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline std::shared_ptr<data_writer_t> Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::addDataWriter(
    const iox::capro::ServiceDescription& service) noexcept
{
    m_writers.emplace_back(
                m_dataWriterFactory(
                    service.getServiceIDString(),
                    service.getInstanceIDString(),
                    service.getEventIDString())
            );
    return m_writers.back();
}

template <typename gateway_t, typename subscriber_t, typename data_writer_t>
inline void Iceoryx2DDSGateway<gateway_t, subscriber_t, data_writer_t>::removeDataWriter(
    const iox::capro::ServiceDescription& service) noexcept
{
    for(auto& w : m_writers){
        if(w->getServiceId() == service.getServiceIDString() && w->getInstanceId() == service.getInstanceIDString() && w->getEventId() == service.getEventIDString()){
            m_writers.erase(&w);
            iox::LogDebug() << "[Iceoryx2DDSGateway] Destroyed DataWriter for service: "
                            << "/" << service.getInstanceIDString() << "/" << service.getServiceIDString() << "/"
                            << service.getEventIDString();
            break;
        }
    }
}

} // namespace dds
} // namespace gateway
} // namespace iox
