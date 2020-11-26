// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
    typename PublisherPort::MemberType_t* publisherPortGeneric,
    typename PublisherPort::MemberType_t* publisherPortThroughput,
    typename PublisherPort::MemberType_t* publisherPortSubscriberPortsData)
{
    if (m_publisherPort || m_publisherPortThroughput || m_publisherPortSubscriberPortsData)
    {
        return false;
    }

    m_publisherPort.emplace(publisherPortGeneric);
    m_publisherPortThroughput.emplace(publisherPortThroughput);
    m_publisherPortSubscriberPortsData.emplace(publisherPortSubscriberPortsData);

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
    pthread_setname_np(m_thread.native_handle(), "PortIntr");
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::sendPortData()
{
    auto maybeChunkHeader = m_publisherPort->tryAllocateChunk(sizeof(PortIntrospectionFieldTopic));
    if (!maybeChunkHeader.has_error())
    {
        auto sample = static_cast<PortIntrospectionFieldTopic*>(maybeChunkHeader.get_value()->payload());
        new (sample) PortIntrospectionFieldTopic();

        m_portData.prepareTopic(*sample); // requires internal mutex (blocks
                                          // further introspection events)
        m_publisherPort->sendChunk(maybeChunkHeader.get_value());
    }
}

template <typename PublisherPort, typename SubscriberPort>
void PortIntrospection<PublisherPort, SubscriberPort>::sendThroughputData()
{
    auto maybeChunkHeader = m_publisherPortThroughput->tryAllocateChunk(sizeof(PortThroughputIntrospectionFieldTopic));
    if (!maybeChunkHeader.has_error())
    {
        auto throughputSample =
            static_cast<PortThroughputIntrospectionFieldTopic*>(maybeChunkHeader.get_value()->payload());
        new (throughputSample) PortThroughputIntrospectionFieldTopic();

        m_portData.prepareTopic(*throughputSample); // requires internal mutex (blocks
        // further introspection events)
        m_publisherPortThroughput->sendChunk(maybeChunkHeader.get_value());
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
            static_cast<SubscriberPortChangingIntrospectionFieldTopic*>(maybeChunkHeader.get_value()->payload());
        new (subscriberPortChangingDataSample) SubscriberPortChangingIntrospectionFieldTopic();

        m_portData.prepareTopic(*subscriberPortChangingDataSample); // requires internal mutex (blocks
        // further introspection events)
        m_publisherPortSubscriberPortsData->sendChunk(maybeChunkHeader.get_value());
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
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();
    capro::CaproMessageType messageType = message.m_type;

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_connectionMap.find(serviceId);
    if (iter == m_connectionMap.end())
    {
        return false; // no corresponding capro Id ...
    }

    auto& map = iter->second;

    for (auto& pair : map)
    {
        auto& connection = m_connectionContainer[pair.second];
        auto subscriberServiceId = static_cast<cxx::Serialization>(connection.subscriberInfo.service).toString();
        if (serviceId == subscriberServiceId)
        {
            // should always be true if its in the map stored at this serviceId
            connection.state = getNextState(connection.state, messageType);
        }
    }

    setNew(true);
    return true;
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::addPublisher(
    typename PublisherPort::MemberType_t* port,
    const std::string& name,
    const capro::ServiceDescription& service,
    const std::string& runnable)
{
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_publisherMap.find(serviceId);
    if (iter != m_publisherMap.end())
        return false;

    auto index = m_publisherContainer.add(PublisherInfo(port, name, service, runnable));
    if (index < 0)
        return false;

    m_publisherMap.insert(std::make_pair(serviceId, index));

    // connect publisher to all subscribers with the same Id
    PublisherInfo* publisher = m_publisherContainer.get(index);

    // find corresponding subscribers
    auto connIter = m_connectionMap.find(serviceId);
    if (connIter != m_connectionMap.end())
    {
        auto& map = connIter->second;
        for (auto& pair : map)
        {
            auto& connection = m_connectionContainer[pair.second];
            // TODO: maybe save the service string instead for efficiency
            auto subscriberServiceId = static_cast<cxx::Serialization>(connection.subscriberInfo.service).toString();
            if (serviceId == subscriberServiceId)
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
    typename SubscriberPort::MemberType_t* portData,
    const std::string& name,
    const capro::ServiceDescription& service,
    const std::string& runnable)
{
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();

    std::lock_guard<std::mutex> lock(m_mutex);

    auto index = m_connectionContainer.add(ConnectionInfo(portData, name, service, runnable));
    if (index < 0)
    {
        return false;
    }

    auto iter = m_connectionMap.find(serviceId);

    if (iter == m_connectionMap.end())
    {
        // serviceId is new, create new map
        std::map<std::string, typename ConnectionContainer::Index_t> map;
        map.insert(std::make_pair(name, index));
        m_connectionMap.insert(std::make_pair(serviceId, map));
    }
    else
    {
        // serviceId exists, add new entry
        // TODO: check existence of key (name)
        auto& map = iter->second;
        map.insert(std::make_pair(name, index));
    }

    auto& connection = m_connectionContainer[index];

    auto sendIter = m_publisherMap.find(serviceId);
    if (sendIter != m_publisherMap.end())
    {
        auto publisher = m_publisherContainer.get(sendIter->second);
        connection.publisherInfo = publisher; // set corresponding publisher if it exists
    }

    return true;
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::PortData::removePublisher(
    const std::string& name [[gnu::unused]], const capro::ServiceDescription& service)
{
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_publisherMap.find(serviceId);
    if (iter == m_publisherMap.end())
        return false;

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
    const std::string& name, const capro::ServiceDescription& service)
{
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_connectionMap.find(serviceId);
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

    int index = 0;
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
            publisherData.m_name = cxx::string<100>(cxx::TruncateToCapacity, publisherInfo.name.c_str());
            publisherData.m_runnable = cxx::string<100>(cxx::TruncateToCapacity, publisherInfo.runnable.c_str());

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

                subscriberData.m_name = cxx::string<100>(cxx::TruncateToCapacity, subscriberInfo.name.c_str());
                subscriberData.m_runnable = cxx::string<100>(cxx::TruncateToCapacity, subscriberInfo.runnable.c_str());

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
void PortIntrospection<PublisherPort, SubscriberPort>::PortData::prepareTopic(PortThroughputIntrospectionTopic& topic)
{
    /// @todo #252 re-add port throughput for v1.0?
    auto& m_throughputList = topic.m_throughputList;

    std::lock_guard<std::mutex> lock(m_mutex); // we need to lock the internal data structs

    int index = 0;
    for (auto& pair : m_publisherMap)
    {
        auto m_publisherIndex = pair.second;
        if (m_publisherIndex >= 0)
        {
            auto& publisherInfo = m_publisherContainer[m_publisherIndex];
            PortThroughputData throughputData;

            PublisherPort port(publisherInfo.portData);
            // auto introData = port.getThroughput();
            throughputData.m_publisherPortID = static_cast<uint64_t>(port.getUniqueID());
            throughputData.m_sampleSize = 0; // introData.payloadSize;
            throughputData.m_chunkSize = 0;  // introData.chunkSize;
            // using Minutes_t = std::chrono::duration<double, std::ratio<60>>;
            // Minutes_t deltaTime = introData.currentDeliveryTimestamp - publisherInfo.m_sequenceNumberTimestamp;
            // auto minutes = deltaTime.count();
            throughputData.m_chunksPerMinute = 0;
            // if (minutes != 0)
            //{
            // throughputData.m_chunksPerMinute = (introData.sequenceNumber - publisherInfo.m_sequenceNumber) /
            // minutes;
            //}
            // auto sendInterval = introData.currentDeliveryTimestamp - introData.lastDeliveryTimestamp;
            // throughputData.m_lastSendIntervalInNanoseconds = sendInterval.count();
            m_throughputList.emplace_back(throughputData);
            publisherInfo.index = index++;

            // publisherInfo.m_sequenceNumberTimestamp = introData.currentDeliveryTimestamp;
            // publisherInfo.m_sequenceNumber = introData.sequenceNumber;
        }
    }
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
                    // subscriberData.fifoCapacity = port.getDeliveryFiFoCapacity();
                    // subscriberData.fifoSize = port.getDeliveryFiFoSize();
                    subscriberData.subscriptionState = port.getSubscriptionState();
                    // subscriberData.sampleSendCallbackActive = port.AreCallbackReferencesSet();
                    // subscriberData.propagationScope = port.getCaProServiceDescription().getScope();
                }
                else
                {
                    subscriberData.fifoCapacity = 0u;
                    subscriberData.fifoSize = 0u;
                    subscriberData.subscriptionState = iox::SubscribeState::NOT_SUBSCRIBED;
                    subscriberData.sampleSendCallbackActive = false;
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
                                                                    const std::string& name,
                                                                    const capro::ServiceDescription& service,
                                                                    const std::string& runnable)
{
    return m_portData.addPublisher(port, name, service, runnable);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::addSubscriber(typename SubscriberPort::MemberType_t* portData,
                                                                     const std::string& name,
                                                                     const capro::ServiceDescription& service,
                                                                     const std::string& runnable)
{
    return m_portData.addSubscriber(portData, name, service, runnable);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::removePublisher(const std::string& name,
                                                                       const capro::ServiceDescription& service)
{
    return m_portData.removePublisher(name, service);
}

template <typename PublisherPort, typename SubscriberPort>
bool PortIntrospection<PublisherPort, SubscriberPort>::removeSubscriber(const std::string& name,
                                                                        const capro::ServiceDescription& service)
{
    return m_portData.removeSubscriber(name, service);
}

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_INL
