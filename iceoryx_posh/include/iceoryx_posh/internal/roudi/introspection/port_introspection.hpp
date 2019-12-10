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

#pragma once

#include "fixed_size_container.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port_data.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <map>

namespace iox
{
namespace roudi
{
/**
 * @brief This class handles the port intropection for RouDi.
 *        It is recommended to use the PortIntrospectionType alias which sets
 *        the intended template parameter required for the actual introspection.
 *
 *        The class manages a thread that periodically updates a field with port
 *        introspection data to which clients may subscribe.
 */
template <typename SenderPort, typename ReceiverPort>
class PortIntrospection
{
  private:
    enum class ConnectionState
    {
        DEFAULT,
        SUB_REQUESTED,
        CONNECTED
    };

    using PortIntrospectionTopic = PortIntrospectionFieldTopic;
    using PortThroughputIntrospectionTopic = PortThroughputIntrospectionFieldTopic;

    class PortData
    {
      private:
        // internal helper classes

        struct ConnectionInfo;

        struct SenderInfo
        {
            SenderInfo() = default;

            SenderInfo(typename SenderPort::MemberType_t* portData,
                       const std::string& name,
                       const capro::ServiceDescription& service,
                       const std::string& runnable)
                : portData(portData)
                , name(name)
                , service(service)
                , runnable(runnable)
            {
            }

            typename SenderPort::MemberType_t* portData{nullptr};
            std::string name;
            capro::ServiceDescription service;
            std::string runnable;

            using TimePointNs = mepoo::TimePointNs;
            using DurationNs = mepoo::DurationNs;
            TimePointNs m_sequenceNumberTimestamp = TimePointNs(DurationNs(0));
            mepoo::SequenceNumberType m_sequenceNumber{0};

            // map from indices to object pointers
            std::map<int, ConnectionInfo*> connectionMap;
            int index{-1};
        };

        struct ReceiverInfo
        {
            ReceiverInfo()
            {
            }

            ReceiverInfo(typename ReceiverPort::MemberType_t* const portData,
                         const std::string& name,
                         const capro::ServiceDescription& service,
                         const std::string& runnable)
                : portData(portData)
                , name(name)
                , service(service)
                , runnable(runnable)
            {
            }

            typename ReceiverPort::MemberType_t* portData{nullptr};
            std::string name;
            capro::ServiceDescription service;
            std::string runnable;
        };

        struct ConnectionInfo
        {
            ConnectionInfo()
            {
            }

            ConnectionInfo(typename ReceiverPort::MemberType_t* const portData,
                           const std::string& name,
                           const capro::ServiceDescription& service,
                           const std::string& runnable)
                : receiverInfo(portData, name, service, runnable)
                , state(ConnectionState::DEFAULT)
            {
            }

            ConnectionInfo(ReceiverInfo& receiverInfo)
                : receiverInfo(receiverInfo)
                , state(ConnectionState::DEFAULT)
            {
            }

            ReceiverInfo receiverInfo;
            SenderInfo* senderInfo{nullptr};
            ConnectionState state{ConnectionState::DEFAULT};

            bool isConnected()
            {
                return senderInfo && state == ConnectionState::CONNECTED;
            }
        };

      public:
        PortData();

        /*!
         * @brief add a sender port to be tracked by introspection
         * there cannot be multiple sender ports with the same capro id
         *
         * @param[in] port to be added
         * @param[in] name of the port to be added
         * @param[in] service capro service description of the port to be added
         * @param[in] name of the runnable the port belongs to
         *
         * @return returns false if the port could not be added and true otherwise
         */
        bool addSender(typename SenderPort::MemberType_t* port,
                       const std::string& name,
                       const capro::ServiceDescription& service,
                       const std::string& runnable);


        /*!
         * @brief add a receiver port to be tracked by introspection
         * multiple receivers with the same capro id are possible as long as the names are different
         *
         * @param[in] portData to be added
         * @param[in] name name of the port to be added
         * @param[in] service capro service description of the port to be added
         * @param[in] name of the runnable the port belongs to
         *
         * @return returns false if the port could not be added and true otherwise
         */
        bool addReceiver(typename ReceiverPort::MemberType_t* const portData,
                         const std::string& name,
                         const capro::ServiceDescription& service,
                         const std::string& runnable);

        /*!
         * @brief remove a sender port from introspection
         *
         * @param[in] name name of the port to be added
         * @param[in] service capro service description of the port to be added
         *
         * @return returns false if the port could not be removed (since it did not exist)
         *              and true otherwise
         */
        bool removeSender(const std::string& name, const capro::ServiceDescription& service);

        /*!
         * @brief remove a receiver port from introspection
         *
         * @param[in] name name of the port to be added
         * @param[in] service capro service description of the port to be added
         *
         * @return returns false if the port could not be removed (since it did not exist)
         *              and true otherwise
         */
        bool removeReceiver(const std::string& name, const capro::ServiceDescription& service);

        /*!
         * @brief update the state of any connection identified by the capro id of a given message
         *        according to the message type (e.g. capro:SUB for a subscription request)
         *
         * @param[in] message capro message to be processed
         *
         * @return returns false there is no corresponding capro service and true otherwise
         */
        bool updateConnectionState(const capro::CaproMessage& message);

        /*!
         * @brief prepare the topic to be send based on the internal connection state of all tracked ports
         *
         * @param[out] topic data structure to be prepared for sending
         *
         */
        void prepareTopic(PortIntrospectionTopic& topic);

        void prepareTopic(PortThroughputIntrospectionTopic& topic);

        void prepareTopic(ReceiverPortChangingIntrospectionFieldTopic& topic);

        /*!
         * @brief compute the next connection state based on the current connection state and a capro message type
         *
         * @param[in] currentState current connection state (e.g. CONNECTED)
         * @param[in] messageType capro message type
         *
         * @return returns the new connection state
         */
        PortIntrospection::ConnectionState getNextState(ConnectionState currentState,
                                                        capro::CaproMessageType messageType);

        /*!
         * @brief indicates whether the logical object state has changed (i.e. the data is new)
         *
         * @return returns true if the data is new(e.g. new connections were established), false otherwise
         */
        bool isNew();

        /*!
         * @brief sets the internal flag indicating new data
         *
         * @param[in] value value to be set
         */
        void setNew(bool value);

      private:
        // MAX_PORT_NUMBER needs to be a compile time constant
        using SenderContainer = FixedSizeContainer<SenderInfo, MAX_PORT_NUMBER>;
        using ConnectionContainer = FixedSizeContainer<ConnectionInfo, MAX_PORT_NUMBER>;

        // index sender and connections by capro Ids
        std::map<std::string, typename SenderContainer::Index_t> m_senderMap;

        // TODO: replace inner map wih more approriate structure if possible
        // inner map maps from port names to indices in the ConnectionContainer
        std::map<std::string, std::map<std::string, typename ConnectionContainer::Index_t>> m_connectionMap;

        // Rationale: we avoid allocating the objects on the heap but can still use a map
        // to locate/remove them fast(er)
        // max number needs to be a compile time constant
        SenderContainer m_senderContainer;
        ConnectionContainer m_connectionContainer;

        std::atomic<bool> m_newData;
        std::mutex m_mutex;
    };

    // end of helper classes


  public:
    PortIntrospection();

    ~PortIntrospection();

    // delete copy constructor and assignment operator
    PortIntrospection(PortIntrospection const&) = delete;
    PortIntrospection& operator=(PortIntrospection const&) = delete;
    // delete move constructor and assignment operator
    PortIntrospection(PortIntrospection&&) = delete;
    PortIntrospection& operator=(PortIntrospection&&) = delete;

    /*!
     * @brief add a sender port to be tracked by introspection
     * there cannot be multiple sender ports with the same capro id
     *
     * @param[in] port to be added
     * @param[in] name of the port to be added
     * @param[in] service capro service description of the port to be added
     * @param[in] name of the runnable the port belongs to
     *
     * @return returns false if the port could not be added and true otherwise
     */
    bool addSender(typename SenderPort::MemberType_t* port,
                   const std::string& name,
                   const capro::ServiceDescription& service,
                   const std::string& runnable);

    /*!
     * @brief add a receiver port to be tracked by introspection
     * multiple receivers with the same capro id are possible as long as the names are different
     *
     * @param[in] portData to be added
     * @param[in] name name of the port to be added
     * @param[in] service capro service description of the port to be added
     * @param[in] name of the runnable the port belongs to
     *
     * @return returns false if the port could not be added and true otherwise
     */
    bool addReceiver(typename ReceiverPort::MemberType_t* const portData,
                     const std::string& name,
                     const capro::ServiceDescription& service,
                     const std::string& runnable);


    /*!
     * @brief remove a sender port from introspection
     *
     * @param[in] name name of the port to be added
     * @param[in] service capro service description of the port to be added
     *
     * @return returns false if the port could not be removed (since it did not exist)
     *              and true otherwise
     */
    bool removeSender(const std::string& name, const capro::ServiceDescription& service);

    /*!
     * @brief remove a receiver port from introspection
     *
     * @param[in] name name of the port to be added
     * @param[in] service capro service description of the port to be added
     *
     * @return returns false if the port could not be removed (since it did not exist)
     *              and true otherwise
     */
    bool removeReceiver(const std::string& name, const capro::ServiceDescription& service);

    /*!
     * @brief report a capro message to introspection (since this could change the state of active connections)
     *
     * @param[in] message capro message to be processed
     *
     */
    void reportMessage(const capro::CaproMessage& message);

    /*!
     * @brief register sender port used to send introspection
     *
     * @param[in] senderPort sender port to be registered
     *
     * @return true if registration was successful, false otherwise
     */
    bool registerSenderPort(SenderPort&& f_senderPortGeneric,
                            SenderPort&& f_senderPortThroughput,
                            SenderPort&& f_senderPortReceiverPortsData);

    /*!
     * @brief set the time interval used to send new introspection data
     *
     * @param[in] sendIntervalMs time interval in ms
     *
     */
    void setSendInterval(unsigned int sendIntervalMs);

    /*!
     * @brief start the internal send thread
     *
     */
    void run();

    /*!
     * @brief stop the internal send thread
     *
     */
    void stop();

  protected:
    /*!
     * @brief sends the port data; this is used from the unittests
     *
     */
    void sendPortData();
    /*!
     * @brief sends the throughput data; this is used from the unittests
     *
     */
    void sendThroughputData();
    /*!
     * @brief sends the receiverport changing data, this is used from the unittests
     *
     */
    void sendReceiverPortsData();

  private:
    SenderPort m_senderPort{nullptr};
    SenderPort m_senderPortThroughput{nullptr};
    SenderPort m_senderPortReceiverPortsData{nullptr};
    PortData m_portData;

    std::atomic<bool> m_runThread;
    std::thread m_thread;

    unsigned int m_sendIntervalCount{10};
    const std::chrono::milliseconds m_sendIntervalSleep{100};
};

/**
 * @brief typedef for the templated port introspection class that is used by RouDi for the
 * actual port introspection functionality.
 */
using PortIntrospectionType = PortIntrospection<SenderPortType, ReceiverPortType>;


} // namespace roudi
} // namespace iox

#include "port_introspection.inl"
