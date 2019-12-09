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

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/application_port.hpp"
#include "iceoryx_posh/internal/popo/interface_port.hpp"
#include "iceoryx_posh/internal/popo/receiver_port.hpp"
#include "iceoryx_posh/internal/popo/sender_port.hpp"
#include "iceoryx_posh/internal/runtime/message_queue_interface.hpp"
#include "iceoryx_posh/internal/runtime/runnable_property.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
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

using IdString = iox::capro::ServiceDescription::IdString;
using InstanceContainer = std::vector<IdString>;

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
    void findService(const capro::ServiceDescription& serviceDescription,
                     InstanceContainer& instanceContainer) noexcept;

    /// @brief offer the provided service, sends the offer from application to RouDi daemon
    /// @param[in] serviceDescription service to offer
    void offerService(const capro::ServiceDescription& serviceDescription) noexcept;

    /// @brief stop offering the provided service
    /// @param[in] serviceDescription of the service that shall be no more offered
    void stopOfferService(const capro::ServiceDescription& f_serviceDescription) noexcept;

    /// @brief request the RouDi daemon to create a sender port
    /// @param[in] serviceDescription service description for the new sender port
    /// @param[in] interface interface to which the sender port shall belong
    /// @param[in] runnableName name of the runnable where the sender should belong to
    /// @return poiner to a created sender port data
    SenderPortType::MemberType_t* getMiddlewareSender(const capro::ServiceDescription& service,
                                                      const Interfaces interface = Interfaces::INTERNAL,
                                                      const cxx::CString100& runnableName = "") noexcept;

    /// @brief request the RouDi daemon to create a receiver port
    /// @param[in] serviceDescription service description for the new receiver port
    /// @param[in] interface interface to which the receiver port shall belong
    /// @param[in] runnableName name of the runnable where the receiver should belong to
    /// @return poiner to a created receiver port data
    ReceiverPortType::MemberType_t* getMiddlewareReceiver(const capro::ServiceDescription& service,
                                                          const Interfaces interface = Interfaces::INTERNAL,
                                                          const cxx::CString100& runnableName = "") noexcept;

    /// @brief request the RouDi daemon to create an interface port
    /// @param[in] interface interface to create
    /// @param[in] runnableName name of the runnable where the interface should belong to
    /// @return poiner to a created interface port data
    popo::InterfacePortData* getMiddlewareInterface(const Interfaces interface,
                                                    const cxx::CString100& runnableName = "") noexcept;

    /// @brief request the RouDi daemon to create an application port
    /// @param[in] interface to which the application port shall belong
    /// @return poiner to a created application port data
    popo::ApplicationPortData* getMiddlewareApplication(const Interfaces interface) noexcept;

    /// @brief request the RouDi daemon to create a runnable
    /// @param[in] runnableProperty class which contains all properties which the runnable should have
    /// @return pointer to the data of the runnable
    RunnableData* createRunnable(const RunnableProperty& runnableProperty) noexcept;

    /// @brief request the RouDi daemon to remove a runnable
    /// @param[in] runnable runnable which should be removed
    /// @return true if the remove was successful, otherwise false
    void removeRunnable(const Runnable& runnable) noexcept;

    /// @brief requests the serviceRegistryChangeCounter from the shared memory
    /// @return pointer to the serviceRegistryChangeCounter
    const std::atomic<uint64_t>* getServiceRegistryChangeCounter() noexcept;

    /// @brief send a request to the RouDi daemon and get the response
    ///        currently each request is followed by a response
    /// @param[in] msg request message to send
    /// @param[out] response from the RouDi daemon
    /// @return true if sucessful request/response, false on error
    bool sendRequestToRouDi(const MqMessage& msg, MqMessage& answer) noexcept;

    /// @brief sends a message to the RouDi daemon
    /// @param[in] msg request message to send
    /// @return true if sucessful send message, false on error
    bool sendMessageToRouDi(const MqMessage& msg) noexcept;

  public:
    PoshRuntime(const PoshRuntime&) = delete;
    PoshRuntime& operator=(const PoshRuntime&) = delete;
    PoshRuntime(PoshRuntime&&) = delete;
    PoshRuntime& operator=(PoshRuntime&&) = delete;

    friend class roudi::RuntimeTestInterface;

  protected:
    // Protected constructor for IPC setup
    PoshRuntime(const std::string& name, const bool doMapSharedMemoryIntoThread = true) noexcept;

    static std::function<PoshRuntime&(const std::string& name)> s_runtimeFactory; // = DefaultRuntimeFactory;
    static PoshRuntime& defaultRuntimeFactory(const std::string& name) noexcept;

  private:
    SenderPortType::MemberType_t* requestSenderFromRoudi(const MqMessage& sendBuffer) noexcept;
    ReceiverPortType::MemberType_t* requestReceiverFromRoudi(const MqMessage& sendBuffer) noexcept;

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
