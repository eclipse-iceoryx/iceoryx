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
#ifndef IOX_POSH_RUNTIME_POSH_RUNTIME_HPP
#define IOX_POSH_RUNTIME_POSH_RUNTIME_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/application_port.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/runtime/node_property.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/internal/concurrent/periodic_task.hpp"

#include <atomic>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace iox
{
namespace roudi
{
class RuntimeTestInterface;
} // namespace roudi

namespace runtime
{
class Node;
class NodeData;

/// @brief The runtime that is needed for each application to communicate with the RouDi daemon
class PoshRuntime
{
  public:
    /// @brief returns active runtime
    ///
    /// @return active runtime
    static PoshRuntime& getInstance() noexcept;

    /// @brief creates the runtime with given name
    ///
    /// @param[in] name used for registering the process with the RouDi daemon
    ///
    /// @return active runtime
    static PoshRuntime& initRuntime(const ProcessName_t& name) noexcept;

    /// @brief get the name that was used to register with RouDi
    /// @return name of the registered application
    ProcessName_t getInstanceName() const noexcept;

    /// @brief find all services that match the provided service description
    /// @param[in] serviceDescription service to search for
    /// @return cxx::expected<InstanceContainer,Error>
    /// InstanceContainer: on success, container that is filled with all matching instances
    /// Error: if any, encountered during the operation
    /// Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW : Number of instances can't fit in instanceContainer
    /// Error::kMQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ : Find Service Request could not be sent to RouDi
    cxx::expected<InstanceContainer, Error> findService(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief offer the provided service, sends the offer from application to RouDi daemon
    /// @param[in] serviceDescription service to offer
    /// @return bool, if service is offered returns true else false
    bool offerService(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief stop offering the provided service
    /// @param[in] serviceDescription of the service that shall be no more offered
    void stopOfferService(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief request the RouDi daemon to create a publisher port
    /// @param[in] serviceDescription service description for the new publisher port
    /// @param[in] publisherOptions like the history capacity of a publisher
    /// @param[in] nodeName name of the node where the publisher should belong to
    /// @param[in] portConfigInfo configuration information for the port
    /// (i.e. what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created publisher port user
    PublisherPortUserType::MemberType_t*
    getMiddlewarePublisher(const capro::ServiceDescription& service,
                           const popo::PublisherOptions& publisherOptions = popo::PublisherOptions(),
                           const NodeName_t& nodeName = "",
                           const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept;

    /// @brief request the RouDi daemon to create a subscriber port
    /// @param[in] serviceDescription service description for the new subscriber port
    /// @param[in] subscriberOptions like the queue capacity and history requested by a subscriber
    /// @param[in] nodeName name of the node where the subscriber should belong to
    /// @param[in] portConfigInfo configuration information for the port
    /// (what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created subscriber port data
    SubscriberPortUserType::MemberType_t*
    getMiddlewareSubscriber(const capro::ServiceDescription& service,
                            const popo::SubscriberOptions& subscriberOptions = popo::SubscriberOptions(),
                            const NodeName_t& nodeName = "",
                            const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept;

    /// @brief request the RouDi daemon to create an interface port
    /// @param[in] interface interface to create
    /// @param[in] nodeName name of the node where the interface should belong to
    /// @return pointer to a created interface port data
    popo::InterfacePortData* getMiddlewareInterface(const capro::Interfaces interface,
                                                    const NodeName_t& nodeName = "") noexcept;

    /// @brief request the RouDi daemon to create an application port
    /// @return pointer to a created application port data
    popo::ApplicationPortData* getMiddlewareApplication() noexcept;

    /// @brief request the RouDi daemon to create an condition variable
    /// @return pointer to a created condition variable data
    popo::ConditionVariableData* getMiddlewareConditionVariable() noexcept;

    /// @brief request the RouDi daemon to create a node
    /// @param[in] nodeProperty class which contains all properties which the node should have
    /// @return pointer to the data of the node
    NodeData* createNode(const NodeProperty& nodeProperty) noexcept;

    /// @brief requests the serviceRegistryChangeCounter from the shared memory
    /// @return pointer to the serviceRegistryChangeCounter
    const std::atomic<uint64_t>* getServiceRegistryChangeCounter() noexcept;

    /// @brief send a request to the RouDi daemon and get the response
    ///        currently each request is followed by a response
    /// @param[in] msg request message to send
    /// @param[out] response from the RouDi daemon
    /// @return true if sucessful request/response, false on error
    bool sendRequestToRouDi(const MqMessage& msg, MqMessage& answer) noexcept;

  public:
    PoshRuntime(const PoshRuntime&) = delete;
    PoshRuntime& operator=(const PoshRuntime&) = delete;
    PoshRuntime(PoshRuntime&&) = delete;
    PoshRuntime& operator=(PoshRuntime&&) = delete;
    virtual ~PoshRuntime() noexcept;

    friend class roudi::RuntimeTestInterface;

  protected:
    using factory_t = PoshRuntime& (*)(cxx::optional<const ProcessName_t*>);

    // Protected constructor for IPC setup
    PoshRuntime(cxx::optional<const ProcessName_t*> name, const bool doMapSharedMemoryIntoThread = true) noexcept;

    static PoshRuntime& defaultRuntimeFactory(cxx::optional<const ProcessName_t*> name) noexcept;

    static ProcessName_t& defaultRuntimeInstanceName() noexcept;

    /// @brief gets current runtime factory. If the runtime factory is not yet initialized it is set to
    /// defaultRuntimeFactory.
    ///
    /// @return current runtime factory
    static factory_t& getRuntimeFactory() noexcept;

    /// @brief sets runtime factory, terminates if given factory is empty
    ///
    /// @param[in] factory std::function to which the runtime factory should be set
    static void setRuntimeFactory(const factory_t& factory) noexcept;

    /// @brief creates the runtime or returns the already existing one -> Singleton
    ///
    /// @param[in] name optional containing the name used for registering with the RouDi daemon
    ///
    /// @return active runtime
    static PoshRuntime& getInstance(cxx::optional<const ProcessName_t*> name) noexcept;

  private:
    cxx::expected<PublisherPortUserType::MemberType_t*, MqMessageErrorType>
    requestPublisherFromRoudi(const MqMessage& sendBuffer) noexcept;

    cxx::expected<SubscriberPortUserType::MemberType_t*, MqMessageErrorType>
    requestSubscriberFromRoudi(const MqMessage& sendBuffer) noexcept;

    cxx::expected<popo::ConditionVariableData*, MqMessageErrorType>
    requestConditionVariableFromRoudi(const MqMessage& sendBuffer) noexcept;

    /// @brief checks the given application name for certain constraints like length or if is empty
    const ProcessName_t& verifyInstanceName(cxx::optional<const ProcessName_t*> name) noexcept;

    const ProcessName_t m_appName;
    mutable std::mutex m_appMqRequestMutex;

    // Message queue interface for POSIX IPC from RouDi
    MqRuntimeInterface m_MqInterface;
    // Shared memory interface for POSIX IPC from RouDi
    SharedMemoryUser m_ShmInterface;
    popo::ApplicationPort m_applicationPort;

    void sendKeepAlive() noexcept;
    static_assert(PROCESS_KEEP_ALIVE_INTERVAL > roudi::DISCOVERY_INTERVAL, "Keep alive interval too small");

    /// @note the m_keepAliveTask should always be the last member, so that it will be the first member to be destroyed
    concurrent::PeriodicTask<cxx::MethodCallback<void>> m_keepAliveTask{
        "KeepAlive", PROCESS_KEEP_ALIVE_INTERVAL, *this, &PoshRuntime::sendKeepAlive};
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_POSH_RUNTIME_HPP
