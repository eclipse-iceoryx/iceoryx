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

namespace iox
{
namespace roudi
{
template <typename SenderPort, typename ReceiverPort>
PortIntrospection<SenderPort, ReceiverPort>::PortIntrospection()
    : m_runThread(false)
{
}

template <typename SenderPort, typename ReceiverPort>
PortIntrospection<SenderPort, ReceiverPort>::PortData::PortData()
    : m_newData(true)
{
}

template <typename SenderPort, typename ReceiverPort>
PortIntrospection<SenderPort, ReceiverPort>::~PortIntrospection()
{
    stop();
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::reportMessage(const capro::CaproMessage& message)
{
    m_portData.updateConnectionState(message);
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::registerSenderPort(SenderPort&& f_senderPortGeneric,
                                                                     SenderPort&& f_senderPortThroughput,
                                                                     SenderPort&& f_senderPortReceiverPortsData)
{
    if (m_senderPort || m_senderPortThroughput)
    {
        return false;
    }

    m_senderPort = std::move(f_senderPortGeneric);
    m_senderPort.enableDoDeliverOnSubscription();

    m_senderPortThroughput = std::move(f_senderPortThroughput);
    m_senderPortThroughput.enableDoDeliverOnSubscription();

    m_senderPortReceiverPortsData = std::move(f_senderPortReceiverPortsData);
    m_senderPortReceiverPortsData.enableDoDeliverOnSubscription();

    return true;
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::run()
{
    cxx::Expects(m_senderPort);
    cxx::Expects(m_senderPortThroughput);

    // this is a field, there needs to be a sample before activate is called
    sendPortData();
    sendThroughputData();
    sendReceiverPortsData();
    m_senderPort.activate();
    m_senderPortThroughput.activate();
    m_senderPortReceiverPortsData.activate();

    /// @todo the thread sleep mechanism needs to be redone with a trigger queue with a try_pop with timeout
    /// functionality
    m_runThread = true;
    static uint32_t ct = 0;
    m_thread = std::thread([this] {
        while (m_runThread)
        {
            if (0 == (ct % m_sendIntervalCount))
            {
                if (m_portData.isNew())
                {
                    sendPortData();
                }
                sendThroughputData();
                sendReceiverPortsData();
            }

            ++ct;

            std::this_thread::sleep_for(m_sendIntervalSleep);
        }
    });

    // set thread name
    pthread_setname_np(m_thread.native_handle(), "PortIntr");
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::sendPortData()
{
    auto chunkHeader = m_senderPort.reserveChunk(sizeof(PortIntrospectionFieldTopic));
    auto sample = static_cast<PortIntrospectionFieldTopic*>(chunkHeader->payload());
    new (sample) PortIntrospectionFieldTopic();

    m_portData.prepareTopic(*sample); // requires internal mutex (blocks
                                      // further introspection events)
    m_senderPort.deliverChunk(chunkHeader);
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::sendThroughputData()
{
    auto chunkHeader = m_senderPortThroughput.reserveChunk(sizeof(PortThroughputIntrospectionFieldTopic));
    auto throughputSample = static_cast<PortThroughputIntrospectionFieldTopic*>(chunkHeader->payload());
    new (throughputSample) PortThroughputIntrospectionFieldTopic();

    m_portData.prepareTopic(*throughputSample); // requires internal mutex (blocks
    // further introspection events)
    m_senderPortThroughput.deliverChunk(chunkHeader);
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::sendReceiverPortsData()
{
    auto chunkInfo = m_senderPortReceiverPortsData.reserveChunk(sizeof(ReceiverPortChangingIntrospectionFieldTopic));
    auto receiverPortChangingDataSample =
    static_cast<ReceiverPortChangingIntrospectionFieldTopic*>(chunkInfo->payload());
    new (receiverPortChangingDataSample) ReceiverPortChangingIntrospectionFieldTopic();

    m_portData.prepareTopic(*receiverPortChangingDataSample); // requires internal mutex (blocks
    // further introspection events)
    m_senderPortReceiverPortsData.deliverChunk(chunkInfo);
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::setSendInterval(unsigned int interval_ms)
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

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::stop()
{
    m_runThread.store(false, std::memory_order_relaxed);
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::PortData::updateConnectionState(const capro::CaproMessage& message)
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
        auto receiverServiceId = static_cast<cxx::Serialization>(connection.receiverInfo.service).toString();
        if (serviceId == receiverServiceId)
        {
            // should always be true if its in the map stored at this serviceId
            connection.state = getNextState(connection.state, messageType);
        }
    }

    setNew(true);
    return true;
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::PortData::addSender(typename SenderPort::MemberType_t* port,
                                                                      const std::string& name,
                                                                      const capro::ServiceDescription& service,
                                                                      const std::string& runnable)
{
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_senderMap.find(serviceId);
    if (iter != m_senderMap.end())
        return false;

    auto index = m_senderContainer.add(SenderInfo(port, name, service, runnable));
    if (index < 0)
        return false;

    m_senderMap.insert(std::make_pair(serviceId, index));

    // connect sender to all receivers with the same Id
    SenderInfo* sender = m_senderContainer.get(index);

    // find corresponding receivers
    auto connIter = m_connectionMap.find(serviceId);
    if (connIter != m_connectionMap.end())
    {
        auto& map = connIter->second;
        for (auto& pair : map)
        {
            auto& connection = m_connectionContainer[pair.second];
            // TODO: maybe save the service string instead for efficiency
            auto receiverServiceId = static_cast<cxx::Serialization>(connection.receiverInfo.service).toString();
            if (serviceId == receiverServiceId)
            {
                connection.senderInfo = sender;
            }
        }
    }

    setNew(true);
    return true;
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::PortData::addReceiver(typename ReceiverPort::MemberType_t* portData,
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

    auto sendIter = m_senderMap.find(serviceId);
    if (sendIter != m_senderMap.end())
    {
        auto sender = m_senderContainer.get(sendIter->second);
        connection.senderInfo = sender; // set corresponding sender if it exists
    }

    return true;
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::PortData::removeSender(const std::string& name[[gnu::unused]],
                                                                         const capro::ServiceDescription& service)
{
    std::string serviceId = static_cast<cxx::Serialization>(service).toString();

    std::lock_guard<std::mutex> lock(m_mutex);

    auto iter = m_senderMap.find(serviceId);
    if (iter == m_senderMap.end())
        return false;

    auto m_senderIndex = iter->second;
    auto& sender = m_senderContainer[m_senderIndex];

    // disconnect sender from all its receivers
    for (auto& pair : sender.connectionMap)
    {
        pair.second->senderInfo = nullptr;             // sender is disconnected
        pair.second->state = ConnectionState::DEFAULT; // connection state is now default
    }

    m_senderMap.erase(iter);
    m_senderContainer.remove(m_senderIndex);
    setNew(true); // indicates we have to send new data because
                  // something changed

    return true;
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::PortData::removeReceiver(const std::string& name,
                                                                           const capro::ServiceDescription& service)
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

    // remove receiver in corresponding sender
    auto connectionIndex = mapIter->second;
    auto& connection = m_connectionContainer[connectionIndex];
    auto& sender = connection.senderInfo;

    if (sender)
    {
        auto connIter = sender->connectionMap.find(connectionIndex);
        if (connIter != sender->connectionMap.end())
        {
            sender->connectionMap.erase(connIter);
        }
    }

    map.erase(mapIter);
    m_connectionContainer.remove(connectionIndex);

    setNew(true);
    return true;
}

template <typename SenderPort, typename ReceiverPort>
typename PortIntrospection<SenderPort, ReceiverPort>::ConnectionState
PortIntrospection<SenderPort, ReceiverPort>::PortData::getNextState(ConnectionState currentState,
                                                                    capro::CaproMessageType messageType)
{
    ConnectionState nextState = currentState; // stay in currentState as default transition

    // sender and receiver can only send a subset of messages (e.g. no
    // sub request from sender), so it is not required to check whether
    // sender or receiver has sent the message...

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

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::PortData::prepareTopic(PortIntrospectionTopic& topic)
{
    auto& m_senderList = topic.m_senderList;

    std::lock_guard<std::mutex> lock(m_mutex); // we need to lock the internal data structs

    int index = 0;
    for (auto& pair : m_senderMap)
    {
        auto m_senderIndex = pair.second;
        if (m_senderIndex >= 0)
        {
            auto& senderInfo = m_senderContainer[m_senderIndex];
            SenderPortData senderData;
            SenderPort port(senderInfo.portData);
            senderData.m_senderPortID = port.getUniqueID();
            senderData.m_name = cxx::CString100(senderInfo.name.c_str());
            senderData.m_runnable = cxx::CString100(senderInfo.runnable.c_str());

            senderData.m_caproInstanceID = senderInfo.service.getInstanceIDString();
            senderData.m_caproServiceID = senderInfo.service.getServiceIDString();
            senderData.m_caproEventMethodID = senderInfo.service.getEventIDString();

            m_senderList.emplace_back(senderData);
            senderInfo.index = index++;
        }
    }

    auto& m_receiverList = topic.m_receiverList;
    for (auto& connPair : m_connectionMap)
    {
        for (auto& pair : connPair.second)
        {
            auto connectionIndex = pair.second;
            if (connectionIndex >= 0)
            {
                auto& connection = m_connectionContainer[connectionIndex];
                ReceiverPortData receiverData;
                bool connected = connection.isConnected();
                auto& receiverInfo = connection.receiverInfo;

                receiverData.m_name = cxx::CString100(receiverInfo.name.c_str());
                receiverData.m_runnable = cxx::CString100(receiverInfo.runnable.c_str());

                receiverData.m_caproInstanceID = receiverInfo.service.getInstanceIDString();
                receiverData.m_caproServiceID = receiverInfo.service.getServiceIDString();
                receiverData.m_caproEventMethodID = receiverInfo.service.getEventIDString();
                if (connected)
                { // senderInfo is not nullptr, otherwise we would not be
                    // connected
                    receiverData.m_senderIndex = connection.senderInfo->index;
                } // remark: index is -1 for not connected
                m_receiverList.emplace_back(receiverData);
            }
        }
    }

    // needs to be done while holding the lock
    setNew(false);
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::PortData::prepareTopic(PortThroughputIntrospectionTopic& topic)
{
    auto& m_throughputList = topic.m_throughputList;

    std::lock_guard<std::mutex> lock(m_mutex); // we need to lock the internal data structs

    int index = 0;
    for (auto& pair : m_senderMap)
    {
        auto m_senderIndex = pair.second;
        if (m_senderIndex >= 0)
        {
            auto& senderInfo = m_senderContainer[m_senderIndex];
            PortThroughputData throughputData;

            SenderPort port(senderInfo.portData);
            auto introData = port.getThroughput();
            throughputData.m_senderPortID = port.getUniqueID();
            throughputData.m_isField = port.doesDeliverOnSubscribe();
            throughputData.m_sampleSize = introData.payloadSize;
            throughputData.m_chunkSize = introData.chunkSize;
            using Minutes_t = std::chrono::duration<double, std::ratio<60>>;
            Minutes_t deltaTime = introData.currentDeliveryTimestamp - senderInfo.m_sequenceNumberTimestamp;
            auto minutes = deltaTime.count();
            throughputData.m_chunksPerMinute = 0;
            if (minutes != 0)
            {
                throughputData.m_chunksPerMinute = (introData.sequenceNumber - senderInfo.m_sequenceNumber) / minutes;
            }
            auto sendInterval = introData.currentDeliveryTimestamp - introData.lastDeliveryTimestamp;
            throughputData.m_lastSendIntervalInNanoseconds = sendInterval.count();
            m_throughputList.emplace_back(throughputData);
            senderInfo.index = index++;

            senderInfo.m_sequenceNumberTimestamp = introData.currentDeliveryTimestamp;
            senderInfo.m_sequenceNumber = introData.sequenceNumber;
        }
    }
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::PortData::prepareTopic(
    ReceiverPortChangingIntrospectionFieldTopic& topic)
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
                auto& receiverInfo = connection.receiverInfo;
                ReceiverPortChangingData receiverData;
                if (receiverInfo.portData != nullptr)
                {
                    ReceiverPort port(receiverInfo.portData);
                    receiverData.fifoCapacity = port.getDeliveryFiFoCapacity();
                    receiverData.fifoSize = port.getDeliveryFiFoSize();
                    receiverData.subscriptionState = port.getSubscribeState();
                    receiverData.sampleSendCallbackActive = port.AreCallbackReferencesSet();
                    receiverData.propagationScope = port.getCaProServiceDescription().getScope();
                }
                else
                {
                    receiverData.fifoCapacity = 0;
                    receiverData.fifoSize = 0;
                    receiverData.subscriptionState = iox::SubscribeState::NOT_SUBSCRIBED;
                    receiverData.sampleSendCallbackActive = false;
                    receiverData.propagationScope = capro::Scope::INVALID;
                }
                topic.receiverPortChangingDataList.push_back(receiverData);
            }
        }
    }
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::PortData::isNew()
{
    return m_newData.load(std::memory_order_acquire);
}

template <typename SenderPort, typename ReceiverPort>
void PortIntrospection<SenderPort, ReceiverPort>::PortData::setNew(bool value)
{
    m_newData.store(value, std::memory_order_release);
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::addSender(typename SenderPort::MemberType_t* port,
                                                            const std::string& name,
                                                            const capro::ServiceDescription& service,
                                                            const std::string& runnable)
{
    return m_portData.addSender(port, name, service, runnable);
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::addReceiver(typename ReceiverPort::MemberType_t* portData,
                                                              const std::string& name,
                                                              const capro::ServiceDescription& service,
                                                              const std::string& runnable)
{
    return m_portData.addReceiver(portData, name, service, runnable);
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::removeSender(const std::string& name,
                                                               const capro::ServiceDescription& service)
{
    return m_portData.removeSender(name, service);
}

template <typename SenderPort, typename ReceiverPort>
bool PortIntrospection<SenderPort, ReceiverPort>::removeReceiver(const std::string& name,
                                                                 const capro::ServiceDescription& service)
{
    return m_portData.removeReceiver(name, service);
}


} // namespace roudi
} // namespace iox
