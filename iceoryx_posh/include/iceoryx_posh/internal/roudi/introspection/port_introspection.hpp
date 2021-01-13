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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_HPP
#define IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_HPP

#include "fixed_size_container.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/roudi/introspection_types.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/posix_wrapper/thread.hpp"

#include <atomic>
#include <mutex>
#include <thread>
#include <vector>

#include <map>

namespace iox
{
namespace roudi
{
/// @brief This class handles the port introspection for RouDi.
///        It is recommended to use the PortIntrospectionType alias which sets
///        the intended template parameter required for the actual introspection.
///        The class manages a thread that periodically updates a field with port
///        introspection data to which clients may subscribe.
template <typename PublisherPort, typename SubscriberPort>
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
        /// internal helper classes

        struct ConnectionInfo;

        struct PublisherInfo
        {
            PublisherInfo() = default;

            PublisherInfo(typename PublisherPort::MemberType_t* const portData,
                          const ProcessName_t& name,
                          const capro::ServiceDescription& service,
                          const NodeName_t& node)
                : portData(portData)
                , name(name)
                , service(service)
                , node(node)
            {
            }

            typename PublisherPort::MemberType_t* portData{nullptr};
            ProcessName_t name;
            capro::ServiceDescription service;
            NodeName_t node;

            using TimePointNs_t = mepoo::TimePointNs_t;
            using DurationNs_t = mepoo::DurationNs_t;
            TimePointNs_t m_sequenceNumberTimestamp{DurationNs_t(0)};
            mepoo::SequenceNumber_t m_sequenceNumber{0U};

            /// map from indices to object pointers
            std::map<int, ConnectionInfo*> connectionMap;
            int index{-1};
        };

        struct SubscriberInfo
        {
            SubscriberInfo()
            {
            }

            SubscriberInfo(typename SubscriberPort::MemberType_t* const portData,
                           const ProcessName_t& name,
                           const capro::ServiceDescription& service,
                           const NodeName_t& node)
                : portData(portData)
                , name(name)
                , service(service)
                , node(node)
            {
            }

            typename SubscriberPort::MemberType_t* portData{nullptr};
            ProcessName_t name;
            capro::ServiceDescription service;
            NodeName_t node;
        };

        struct ConnectionInfo
        {
            ConnectionInfo()
            {
            }

            ConnectionInfo(typename SubscriberPort::MemberType_t* const portData,
                           const ProcessName_t& name,
                           const capro::ServiceDescription& service,
                           const NodeName_t& node)
                : subscriberInfo(portData, name, service, node)
                , state(ConnectionState::DEFAULT)
            {
            }

            ConnectionInfo(SubscriberInfo& subscriberInfo)
                : subscriberInfo(subscriberInfo)
                , state(ConnectionState::DEFAULT)
            {
            }

            SubscriberInfo subscriberInfo;
            PublisherInfo* publisherInfo{nullptr};
            ConnectionState state{ConnectionState::DEFAULT};

            bool isConnected()
            {
                return publisherInfo && state == ConnectionState::CONNECTED;
            }
        };

      public:
        PortData();

        /// @brief add a publisher port to be tracked by introspection
        /// there cannot be multiple publisher ports with the same capro id
        /// @param[in] port to be added
        /// @param[in] name of the port to be added
        /// @param[in] service capro service description of the port to be added
        /// @param[in] name of the node the port belongs to
        /// @return returns false if the port could not be added and true otherwise

        bool addPublisher(typename PublisherPort::MemberType_t* const port,
                          const ProcessName_t& name,
                          const capro::ServiceDescription& service,
                          const NodeName_t& node);

        /// @brief add a subscriber port to be tracked by introspection
        /// multiple subscribers with the same capro id are possible as long as the names are different
        /// @param[in] portData to be added
        /// @param[in] name name of the port to be added
        /// @param[in] service capro service description of the port to be added
        /// @param[in] name of the node the port belongs to
        /// @return returns false if the port could not be added and true otherwise
        bool addSubscriber(typename SubscriberPort::MemberType_t* const portData,
                           const ProcessName_t& name,
                           const capro::ServiceDescription& service,
                           const NodeName_t& node);

        /// @brief remove a publisher port from introspection
        /// @param[in] name name of the port to be added
        /// @param[in] service capro service description of the port to be added
        /// @return returns false if the port could not be removed (since it did not exist)
        ///              and true otherwise
        bool removePublisher(const ProcessName_t& name, const capro::ServiceDescription& service);

        /// @brief remove a subscriber port from introspection
        /// @param[in] name name of the port to be added
        /// @param[in] service capro service description of the port to be added
        /// @return returns false if the port could not be removed (since it did not exist)
        ///              and true otherwise
        bool removeSubscriber(const ProcessName_t& name, const capro::ServiceDescription& service);

        /// @brief update the state of any connection identified by the capro id of a given message
        ///        according to the message type (e.g. capro:SUB for a subscription request)
        /// @param[in] message capro message to be processed
        /// @return returns false there is no corresponding capro service and true otherwise
        bool updateConnectionState(const capro::CaproMessage& message);

        /// @brief prepare the topic to be send based on the internal connection state of all tracked ports
        /// @param[out] topic data structure to be prepared for sending
        void prepareTopic(PortIntrospectionTopic& topic);

        void prepareTopic(PortThroughputIntrospectionTopic& topic);

        void prepareTopic(SubscriberPortChangingIntrospectionFieldTopic& topic);

        /// @brief compute the next connection state based on the current connection state and a capro message type
        /// @param[in] currentState current connection state (e.g. CONNECTED)
        /// @param[in] messageType capro message type
        /// @return returns the new connection state
        PortIntrospection::ConnectionState getNextState(ConnectionState currentState,
                                                        capro::CaproMessageType messageType);

        /// @brief indicates whether the logical object state has changed (i.e. the data is new)
        /// @return returns true if the data is new(e.g. new connections were established), false otherwise
        bool isNew();

        /// @brief sets the internal flag indicating new data
        /// @param[in] value value to be set
        void setNew(bool value);

      private:
        using PublisherContainer = FixedSizeContainer<PublisherInfo, MAX_PUBLISHERS>;
        using ConnectionContainer = FixedSizeContainer<ConnectionInfo, MAX_SUBSCRIBERS>;

        /// @brief index publisher and connections by capro Ids
        std::map<capro::ServiceDescription, typename PublisherContainer::Index_t> m_publisherMap;

        /// @todo: replace inner map wih more appropiate structure if possible
        /// inner map maps from process names to indices in the ConnectionContainer
        std::map<capro::ServiceDescription, std::map<ProcessName_t, typename ConnectionContainer::Index_t>>
            m_connectionMap;

        /// @note we avoid allocating the objects on the heap but can still use a map
        /// to locate/remove them fast(er)
        /// max number needs to be a compile time constant
        PublisherContainer m_publisherContainer;
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

    /// @brief add a publisher port to be tracked by introspection
    /// there cannot be multiple publisher ports with the same capro id
    /// @param[in] port to be added
    /// @param[in] name of the port to be added
    /// @param[in] service capro service description of the port to be added
    /// @param[in] name of the node the port belongs to
    /// @return returns false if the port could not be added and true otherwise
    bool addPublisher(typename PublisherPort::MemberType_t* port,
                      const ProcessName_t& name,
                      const capro::ServiceDescription& service,
                      const NodeName_t& node);

    /// @brief add a subscriber port to be tracked by introspection
    /// multiple subscribers with the same capro id are possible as long as the names are different
    /// @param[in] port to be added
    /// @param[in] name name of the port to be added
    /// @param[in] service capro service description of the port to be added
    /// @param[in] name of the node the port belongs to
    /// @return returns false if the port could not be added and true otherwise
    bool addSubscriber(typename SubscriberPort::MemberType_t* port,
                       const ProcessName_t& name,
                       const capro::ServiceDescription& service,
                       const NodeName_t& node);


    /// @brief remove a publisher port from introspection
    /// @param[in] name name of the port to be added
    /// @param[in] service capro service description of the port to be added
    /// @return returns false if the port could not be removed (since it did not exist)
    ///              and true otherwise
    bool removePublisher(const ProcessName_t& name, const capro::ServiceDescription& service);

    /// @brief remove a subscriber port from introspection
    /// @param[in] name name of the port to be added
    /// @param[in] service capro service description of the port to be added
    /// @return returns false if the port could not be removed (since it did not exist)
    ///              and true otherwise
    bool removeSubscriber(const ProcessName_t& name, const capro::ServiceDescription& service);

    /// @brief report a capro message to introspection (since this could change the state of active connections)
    /// @param[in] message capro message to be processed
    void reportMessage(const capro::CaproMessage& message);

    /// @brief register publisher port used to send introspection
    /// @param[in] publisherPort publisher port to be registered
    /// @return true if registration was successful, false otherwise
    bool registerPublisherPort(PublisherPort&& publisherPortGeneric,
                               PublisherPort&& publisherPortThroughput,
                               PublisherPort&& publisherPortSubscriberPortsData);

    /// @brief set the time interval used to send new introspection data
    /// @param[in] sendIntervalMs time interval in ms
    void setSendInterval(unsigned int sendIntervalMs);


    /// @brief start the internal send thread
    void run();

    /// @brief stop the internal send thread
    void stop();

  protected:
    /// @brief sends the port data; this is used from the unittests
    void sendPortData();

    /// @brief sends the throughput data; this is used from the unittests
    void sendThroughputData();

    /// @brief sends the subscriberport changing data, this is used from the unittests
    void sendSubscriberPortsData();

  protected:
    cxx::optional<PublisherPort> m_publisherPort;
    cxx::optional<PublisherPort> m_publisherPortThroughput;
    cxx::optional<PublisherPort> m_publisherPortSubscriberPortsData;

  private:
    PortData m_portData;

    std::atomic<bool> m_runThread;
    std::thread m_thread;

    unsigned int m_sendIntervalCount{10};
    const std::chrono::milliseconds m_sendIntervalSleep{100};
};

/// @brief typedef for the templated port introspection class that is used by RouDi for the
/// actual port introspection functionality.
using PortIntrospectionType = PortIntrospection<PublisherPortUserType, SubscriberPortUserType>;

} // namespace roudi
} // namespace iox

#include "port_introspection.inl"

#endif // IOX_POSH_ROUDI_INTROSPECTION_PORT_INTROSPECTION_HPP
