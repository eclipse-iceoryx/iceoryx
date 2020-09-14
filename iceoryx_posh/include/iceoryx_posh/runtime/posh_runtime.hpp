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
#ifndef IOX_POSH_RUNTIME_POSH_RUNTIME_HPP
#define IOX_POSH_RUNTIME_POSH_RUNTIME_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/application_port.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/runtime/runnable_property.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iceoryx_utils/fixed_string/string100.hpp"

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
class Runnable;
class RunnableData;

constexpr char DEFAULT_RUNTIME_INSTANCE_NAME[] = "dummy";


/// @brief The runtime that is needed for each application to communicate with the RouDi daemon
class PoshRuntime
{
  public:
    /// @brief creates the runtime or return the already existing one -> Singleton
    /// @param[in] name name that is used for registering the process with the RouDi daemon
    static PoshRuntime& getInstance(const std::string& name = DEFAULT_RUNTIME_INSTANCE_NAME) noexcept;

    /// @brief get the name that was used to register with RouDi
    /// @return name of the reistered application
    std::string getInstanceName() const noexcept;

    /// @brief find all services that match the provided service description
    /// @param[in] serviceDescription service to search for
    /// @param[out] instanceContainer container that is filled with all matching instances
    /// @return cxx::expected<Error> Error, if any, encountered during the operation
    /// Error::kPOSH__SERVICE_DISCOVERY_INSTANCE_CONTAINER_OVERFLOW : Number of instances can't fit in instanceContainer
    /// Error::kMQ_INTERFACE__REG_UNABLE_TO_WRITE_TO_ROUDI_MQ : Find Service Request could not be sent to RouDi
    cxx::expected<Error> findService(const capro::ServiceDescription& serviceDescription,
                                     InstanceContainer& instanceContainer) noexcept;

    /// @brief offer the provided service, sends the offer from application to RouDi daemon
    /// @param[in] serviceDescription service to offer
    void offerService(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief stop offering the provided service
    /// @param[in] serviceDescription of the service that shall be no more offered
    void stopOfferService(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @deprecated #25
    /// @brief request the RouDi daemon to create a sender port
    /// @param[in] serviceDescription service description for the new sender port
    /// @param[in] runnableName name of the runnable where the sender should belong to
    /// @param[in] portConfigInfo configuration information for the port
    /// (i.e. what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created sender port data
    SenderPortType::MemberType_t* getMiddlewareSender(const capro::ServiceDescription& service,
                                                      const cxx::CString100& runnableName = cxx::CString100(""),
                                                      const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept;

    /// @deprecated #25
    /// @brief request the RouDi daemon to create a receiver port
    /// @param[in] serviceDescription service description for the new receiver port
    /// @param[in] runnableName name of the runnable where the receiver should belong to
    /// @param[in] portConfigInfo configuration information for the port
    /// (what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created receiver port data
    ReceiverPortType::MemberType_t*
    getMiddlewareReceiver(const capro::ServiceDescription& service,
                          const cxx::CString100& runnableName = cxx::CString100(""),
                          const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept;

    /// @brief request the RouDi daemon to create a publisher port
    /// @param[in] serviceDescription service description for the new publisher port
    /// @param[in] historyCapacity history capacity of a publisher
    /// @param[in] runnableName name of the runnable where the publisher should belong to
    /// @param[in] portConfigInfo configuration information for the port
    /// (i.e. what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created publisher port user
    PublisherPortUserType::MemberType_t*
    getMiddlewarePublisher(const capro::ServiceDescription& service,
                           const uint64_t& historyCapacity = 0U,
                           const cxx::CString100& runnableName = cxx::CString100(""),
                           const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept;

    /// @brief request the RouDi daemon to create a subscriber port
    /// @param[in] serviceDescription service description for the new subscriber port
    /// @param[in] historyRequest history requested by a subscriber
    /// @param[in] runnableName name of the runnable where the subscriber should belong to
    /// @param[in] portConfigInfo configuration information for the port
    /// (what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created subscriber port data
    SubscriberPortUserType::MemberType_t*
    getMiddlewareSubscriber(const capro::ServiceDescription& service,
                            const uint64_t& historyRequest = 0U,
                            const cxx::CString100& runnableName = cxx::CString100(""),
                            const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept;

    /// @brief request the RouDi daemon to create an interface port
    /// @param[in] interface interface to create
    /// @param[in] runnableName name of the runnable where the interface should belong to
    /// @return pointer to a created interface port data
    popo::InterfacePortData* getMiddlewareInterface(const capro::Interfaces interface,
                                                    const cxx::CString100& runnableName = cxx::CString100("")) noexcept;

    /// @brief request the RouDi daemon to create an application port
    /// @return pointer to a created application port data
    popo::ApplicationPortData* getMiddlewareApplication() noexcept;

    /// @brief request the RouDi daemon to create an condition variable
    /// @return pointer to a created condition variable data
    popo::ConditionVariableData* getMiddlewareConditionVariable() noexcept;

    /// @brief request the RouDi daemon to create a runnable
    /// @param[in] runnableProperty class which contains all properties which the runnable should have
    /// @return pointer to the data of the runnable
    RunnableData* createRunnable(const RunnableProperty& runnableProperty) noexcept;

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
    // Protected constructor for IPC setup
    PoshRuntime(const std::string& name, const bool doMapSharedMemoryIntoThread = true) noexcept;

    static std::function<PoshRuntime&(const std::string& name)> s_runtimeFactory; // = DefaultRuntimeFactory;
    static PoshRuntime& defaultRuntimeFactory(const std::string& name) noexcept;

  private:
    /// @deprecated #25
    cxx::expected<SenderPortType::MemberType_t*, MqMessageErrorType>
    requestSenderFromRoudi(const MqMessage& sendBuffer) noexcept;

    /// @deprecated #25
    ReceiverPortType::MemberType_t* requestReceiverFromRoudi(const MqMessage& sendBuffer) noexcept;

    cxx::expected<PublisherPortUserType::MemberType_t*, MqMessageErrorType>
    requestPublisherFromRoudi(const MqMessage& sendBuffer) noexcept;

    cxx::expected<SubscriberPortUserType::MemberType_t*, MqMessageErrorType>
    requestSubscriberFromRoudi(const MqMessage& sendBuffer) noexcept;

    cxx::expected<popo::ConditionVariableData*, MqMessageErrorType>
    requestConditionVariableFromRoudi(const MqMessage& sendBuffer) noexcept;

    /// @brief checks the given application name for certain constraints like length(100 chars) or leading slash
    /// @todo replace length check with fixedstring when its integrated
    const std::string& verifyInstanceName(const std::string& name) noexcept;

    const std::string m_appName;
    mutable std::mutex m_appMqRequestMutex;

    // Message queue interface for POSIX IPC from RouDi
    MqRuntimeInterface m_MqInterface;
    // Shared memory interface for POSIX IPC from RouDi
    SharedMemoryUser m_ShmInterface;
    popo::ApplicationPort m_applicationPort;

    void sendKeepAlive() noexcept;
    static_assert(PROCESS_KEEP_ALIVE_INTERVAL > DISCOVERY_INTERVAL, "Keep alive interval too small");

    /// @note the m_keepAliveTimer should always be the last member, so that it will be the first member to be detroyed
    iox::posix::Timer m_keepAliveTimer{PROCESS_KEEP_ALIVE_INTERVAL, [&]() { this->sendKeepAlive(); }};
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_POSH_RUNTIME_HPP
