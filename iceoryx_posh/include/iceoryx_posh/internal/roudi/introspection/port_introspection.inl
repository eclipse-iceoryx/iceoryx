// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_INL
#define IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_INL

#include "iceoryx_utils/posix_wrapper/thread.hpp"

namespace iox
{
namespace roudi
{
template <typename PublisherPort, typename SubscriberPort>
PortIntrospection<PublisherPort, SubscriberPort>::PortIntrospection()
    : m_runThread(false)
{
}

template <typename PublisherPort, typename SubscriberPort>
PortIntrospection<PublisherPort, SubscriberPort>::PortData::PortData()
    : m_newData(true)
{
}

template <typename PublisherPort, typename SubscriberPort>
PortIntrospection<PublisherPort, SubscriberPort>::~PortIntrospection()
{
    stop();
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::reportMessage(const capro::CaproMessage& message)
{
    m_portData.updateConnectionState(message);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::registerPublisherPort(
    PublisherPort&& publisherPortGeneric,
    PublisherPort&& publisherPortThroughput,
    PublisherPort&& publisherPortSubscriberPortsData)
{
    if (m_publisherPort || m_publisherPortThroughput || m_publisherPortSubscriberPortsData)
    {
        return false;
    }

    m_publisherPort.emplace(std::move(publisherPortGeneric));
    m_publisherPortThroughput.emplace(std::move(publisherPortThroughput));
    m_publisherPortSubscriberPortsData.emplace(std::move(publisherPortSubscriberPortsData));

    return true;
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::run()
{
    cxx::Expects(m_publisherPort.has_value());
    cxx::Expects(m_publisherPortThroughput.has_value());
    cxx::Expects(m_publisherPortSubscriberPortsData.has_value());

    // this is a field, there needs to be a sample before activate is called
    sendPortData();
    sendThroughputData();
    sendSubscriberPortsData();
    m_publisherPort->offer();
    m_publisherPortThroughput->offer();
    m_publisherPortSubscriberPortsData->offer();

    /// @todo the thread sleep mechanism needs to be redone with a trigger queue with a try_pop with timeout
    /// functionality
    m_runThread = true;
    static uint32_t ct = 0u;
    m_thread = std::thread([this] {
        while (m_runThread)
        {
            if (0u == (ct % m_sendIntervalCount))
            {
                if (m_portData.isNew())
                {
                    sendPortData();
                }
                sendThroughputData();
                sendSubscriberPortsData();
            }

            ++ct;

            std::this_thread::sleep_for(m_sendIntervalSleep);
        }
    });

    // set thread name
    posix::setThreadName(m_thread.native_handle(), "PortIntr");
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::sendPortData()
{
    auto maybeChunkHeader = m_publisherPort->tryAllocateChunk(sizeof(PortIntrospectionFieldTopic));
    if (!maybeChunkHeader.has_error())
    {
        auto sample = static_cast<PortIntrospectionFieldTopic*>(maybeChunkHeader.value()->payload());
        new (sample) PortIntrospectionFieldTopic();

        m_portData.prepareTopic(*sample); // requires internal mutex (blocks
                                          // further introspection events)
        m_publisherPort->sendChunk(maybeChunkHeader.value());
    }
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::sendThroughputData()
{
    auto maybeChunkHeader = m_publisherPortThroughput->tryAllocateChunk(sizeof(PortThroughputIntrospectionFieldTopic));
    if (!maybeChunkHeader.has_error())
    {
        auto throughputSample =
            static_cast<PortThroughputIntrospectionFieldTopic*>(maybeChunkHeader.value()->payload());
        new (throughputSample) PortThroughputIntrospectionFieldTopic();

        m_portData.prepareTopic(*throughputSample); // requires internal mutex (blocks
        // further introspection events)
        m_publisherPortThroughput->sendChunk(maybeChunkHeader.value());
    }
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::sendSubscriberPortsData()
{
    auto maybeChunkHeader =
        m_publisherPortSubscriberPortsData->tryAllocateChunk(sizeof(SubscriberPortChangingIntrospectionFieldTopic));
    if (!maybeChunkHeader.has_error())
    {
        auto subscriberPortChangingDataSample =
            static_cast<SubscriberPortChangingIntrospectionFieldTopic*>(maybeChunkHeader.value()->payload());
        new (subscriberPortChangingDataSample) SubscriberPortChangingIntrospectionFieldTopic();

        m_portData.prepareTopic(*subscriberPortChangingDataSample); // requires internal mutex (blocks
        // further introspection events)
        m_publisherPortSubscriberPortsData->sendChunk(maybeChunkHeader.value());
    }
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::setSendInterval(unsigned int interval_ms)
{
    if (std::chrono::milliseconds(interval_ms) >= m_sendIntervalSleep)
    {
        m_sendIntervalCount = static_cast<unsigned int>(std::chrono::milliseconds(interval_ms) / m_sendIntervalSleep);
    }
    else
    {
        m_sendIntervalCount = 1;
    }
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::stop()
{
    m_runThread.store(false, std::memory_order_relaxed);
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::updateConnectionState(
    const capro::CaproMessage& message)
{
    const capro::ServiceDescription& service = message.m_serviceDescription;
    capro::CaproMessageType messageType = message.m_type;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_connectionMap.find(service);
    if (iter == m_connectionMap.end())
    {
        return false; // no corresponding capro Id ...
    }

    auto& map = iter->second;

    for (auto& pair : map)
    {
        auto& connection = m_connectionContainer[pair.second];
        if (service == connection.subscriberInfo.service)
        {
            // should always be true if its in the map stored at this service key
            connection.state = getNextState(connection.state, messageType);
        }
    }

    setNew(true);
    return true;
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::addPublisher(
    typename PublisherPort::MemberType_t* const port,
    const ProcessName_t& name,
    const capro::ServiceDescription& service,
    const NodeName_t& node)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_publisherMap.find(service);
    if (iter != m_publisherMap.end())
    {
        return false;
    }

    auto index = m_publisherContainer.add(PublisherInfo(port, name, service, node));
    if (index < 0)
    {
        return false;
    }

    m_publisherMap.insert(std::make_pair(service, index));

    // connect publisher to all subscribers with the same Id
    PublisherInfo* publisher = m_publisherContainer.get(index);

    // find corresponding subscribers
    auto connIter = m_connectionMap.find(service);
    if (connIter != m_connectionMap.end())
    {
        auto& map = connIter->second;
        for (auto& pair : map)
        {
            auto& connection = m_connectionContainer[pair.second];
            if (service == connection.subscriberInfo.service)
            {
                connection.publisherInfo = publisher;
            }
        }
    }

    setNew(true);
    return true;
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::addSubscriber(
    typename SubscriberPort::MemberType_t* const portData,
    const ProcessName_t& name,
    const capro::ServiceDescription& service,
    const NodeName_t& node)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto index = m_connectionContainer.add(ConnectionInfo(portData, name, service, node));
    if (index < 0)
    {
        return false;
    }

    auto iter = m_connectionMap.find(service);

    if (iter == m_connectionMap.end())
    {
        // service is new, create new map
        std::map<ProcessName_t, typename ConnectionContainer::Index_t> map;
        map.insert(std::make_pair(name, index));
        m_connectionMap.insert(std::make_pair(service, map));
    }
    else
    {
        // service exists, add new entry
        // TODO: check existence of key (name)
        auto& map = iter->second;
        map.insert(std::make_pair(name, index));
    }

    auto& connection = m_connectionContainer[index];

    auto sendIter = m_publisherMap.find(service);
    if (sendIter != m_publisherMap.end())
    {
        auto publisher = m_publisherContainer.get(sendIter->second);
        connection.publisherInfo = publisher; // set corresponding publisher if it exists
    }

    return true;
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::removePublisher(
    const ProcessName_t& name [[gnu::unused]], const capro::ServiceDescription& service)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_publisherMap.find(service);
    if (iter == m_publisherMap.end())
    {
        return false;
    }

    auto m_publisherIndex = iter->second;
    auto& publisher = m_publisherContainer[m_publisherIndex];

    // disconnect publisher from all its subscribers
    for (auto& pair : publisher.connectionMap)
    {
        pair.second->publisherInfo = nullptr;          // publisher is disconnected
        pair.second->state = ConnectionState::DEFAULT; // connection state is now default
    }

    m_publisherMap.erase(iter);
    m_publisherContainer.remove(m_publisherIndex);
    setNew(true); // indicates we have to send new data because
                  // something changed

    return true;
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::removeSubscriber(
    const ProcessName_t& name, const capro::ServiceDescription& service)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_connectionMap.find(service);
    if (iter == m_connectionMap.end())
    {
        return false; // not found and therefore not removed
    }

    auto& map = iter->second;
    auto mapIter = map.find(name);

    if (mapIter == map.end())
    {
        return false; // not found and therefore not removed
    }

    // remove subscriber in corresponding publisher
    auto connectionIndex = mapIter->second;
    auto& connection = m_connectionContainer[connectionIndex];
    auto& publisher = connection.publisherInfo;

    if (publisher)
    {
        auto connIter = publisher->connectionMap.find(connectionIndex);
        if (connIter != publisher->connectionMap.end())
        {
            publisher->connectionMap.erase(connIter);
        }
    }

    map.erase(mapIter);
    m_connectionContainer.remove(connectionIndex);

    setNew(true);
    return true;
}

template <typename PublisherPort, typename SubscriberPort>
typename PortIntrospection<PublisherPort, SubscriberPort>::ConnectionState
PortIntrospection<PublisherPort, SubscriberPort>::PortData::getNextState(ConnectionState currentState,
                                                                         capro::CaproMessageType messageType)
{
    ConnectionState nextState = currentState; // stay in currentState as default transition

    // publisher and subscriber can only send a subset of messages (e.g. no
    // sub request from publisher), so it is not required to check whether
    // publisher or subscriber has sent the message...

    switch (currentState)
    {
    case ConnectionState::DEFAULT:
    {
        if (messageType == capro::CaproMessageType::SUB)
        {
            nextState = ConnectionState::SUB_REQUESTED;
        }
        break;
    }

    case ConnectionState::SUB_REQUESTED:
    {
        if (messageType == capro::CaproMessageType::ACK)
        {
            nextState = ConnectionState::CONNECTED;
        }
        else if (messageType == capro::CaproMessageType::NACK)
        {
            nextState = ConnectionState::DEFAULT;
        }
        break;
    }

    case ConnectionState::CONNECTED:
    {
        if (messageType == capro::CaproMessageType::STOP_OFFER)
        {
            nextState = ConnectionState::DEFAULT;
        }
        else if (messageType == capro::CaproMessageType::UNSUB)
        {
            nextState = ConnectionState::DEFAULT;
        }
        break;
    }

    default:
    { // stay in current state
    }
    };

    return nextState;
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::PortData::prepareTopic(PortIntrospectionTopic& topic)
{
    auto& m_publisherList = topic.m_publisherList;

    std::lock_guard<std::mutex> lock(m_mutex); // we need to lock the internal data structs

    int32_t index{0};
    for (auto& pair : m_publisherMap)
    {
        auto m_publisherIndex = pair.second;
        if (m_publisherIndex >= 0)
        {
            auto& publisherInfo = m_publisherContainer[m_publisherIndex];
            PublisherPortData publisherData;
            PublisherPort port(publisherInfo.portData);
            publisherData.m_publisherPortID = static_cast<uint64_t>(port.getUniqueID());
            publisherData.m_sourceInterface = publisherInfo.service.getSourceInterface();
            publisherData.m_name = publisherInfo.name;
            publisherData.m_node = publisherInfo.node;

            publisherData.m_caproInstanceID = publisherInfo.service.getInstanceIDString();
            publisherData.m_caproServiceID = publisherInfo.service.getServiceIDString();
            publisherData.m_caproEventMethodID = publisherInfo.service.getEventIDString();

            m_publisherList.emplace_back(publisherData);
            publisherInfo.index = index++;
        }
    }

    auto& m_subscriberList = topic.m_subscriberList;
    for (auto& connPair : m_connectionMap)
    {
        for (auto& pair : connPair.second)
        {
            auto connectionIndex = pair.second;
            if (connectionIndex >= 0)
            {
                auto& connection = m_connectionContainer[connectionIndex];
                SubscriberPortData subscriberData;
                bool connected = connection.isConnected();
                auto& subscriberInfo = connection.subscriberInfo;

                subscriberData.m_name = subscriberInfo.name;
                subscriberData.m_node = subscriberInfo.node;

                subscriberData.m_caproInstanceID = subscriberInfo.service.getInstanceIDString();
                subscriberData.m_caproServiceID = subscriberInfo.service.getServiceIDString();
                subscriberData.m_caproEventMethodID = subscriberInfo.service.getEventIDString();
                if (connected)
                { // publisherInfo is not nullptr, otherwise we would not be
                    // connected
                    subscriberData.m_publisherIndex = connection.publisherInfo->index;
                } // remark: index is -1 for not connected
                m_subscriberList.emplace_back(subscriberData);
            }
        }
    }

    // needs to be done while holding the lock
    setNew(false);
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::PortData::prepareTopic(PortThroughputIntrospectionTopic& topic
                                                                              [[gnu::unused]])
{
    /// @todo #402 re-add port throughput
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::PortData::prepareTopic(
    SubscriberPortChangingIntrospectionFieldTopic& topic)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto& connPair : m_connectionMap)
    {
        for (auto& pair : connPair.second)
        {
            auto connectionIndex = pair.second;
            if (connectionIndex >= 0)
            {
                auto& connection = m_connectionContainer[connectionIndex];
                auto& subscriberInfo = connection.subscriberInfo;
                SubscriberPortChangingData subscriberData;
                if (subscriberInfo.portData != nullptr)
                {
                    SubscriberPort port(subscriberInfo.portData);
                    subscriberData.subscriptionState = port.getSubscriptionState();

                    // subscriberData.fifoCapacity = port .getDeliveryFiFoCapacity();
                    // subscriberData.fifoSize = port.getDeliveryFiFoSize();
                    subscriberData.propagationScope = port.getCaProServiceDescription().getScope();
                }
                else
                {
                    subscriberData.fifoCapacity = 0u;
                    subscriberData.fifoSize = 0u;
                    subscriberData.subscriptionState = iox::SubscribeState::NOT_SUBSCRIBED;
                    subscriberData.propagationScope = capro::Scope::INVALID;
                }
                topic.subscriberPortChangingDataList.push_back(subscriberData);
            }
        }
    }
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::isNew()
{
    return m_newData.load(std::memory_order_acquire);
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::PortData::setNew(bool value)
{
    m_newData.store(value, std::memory_order_release);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::addPublisher(typename PublisherPort::MemberType_t* port,
                                                                    const ProcessName_t& name,
                                                                    const capro::ServiceDescription& service,
                                                                    const NodeName_t& node)
{
    return m_portData.addPublisher(std::move(port), name, service, node);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::addSubscriber(typename SubscriberPort::MemberType_t* port,
                                                                     const ProcessName_t& name,
                                                                     const capro::ServiceDescription& service,
                                                                     const NodeName_t& node)
{
    return m_portData.addSubscriber(std::move(port), name, service, node);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::removePublisher(const ProcessName_t& name,
                                                                       const capro::ServiceDescription& service)
{
    return m_portData.removePublisher(name, service);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::removeSubscriber(const ProcessName_t& name,
                                                                        const capro::ServiceDescription& service)
{
    return m_portData.removeSubscriber(name, service);
}

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_INL
