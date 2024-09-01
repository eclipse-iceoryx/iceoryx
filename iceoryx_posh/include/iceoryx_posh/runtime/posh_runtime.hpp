// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_RUNTIME_POSH_RUNTIME_HPP
#define IOX_POSH_RUNTIME_POSH_RUNTIME_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/runtime/ipc_runtime_interface.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/popo/subscriber_options.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iox/atomic.hpp"
#include "iox/optional.hpp"
#include "iox/scope_guard.hpp"

namespace iox
{
namespace roudi_env
{
class RuntimeTestInterface;
} // namespace roudi_env

namespace runtime
{
/// @brief The runtime that is needed for each application to communicate with the RouDi daemon
class PoshRuntime
{
  public:
    virtual ~PoshRuntime() noexcept = default;

    PoshRuntime(const PoshRuntime&) = delete;
    PoshRuntime& operator=(const PoshRuntime&) = delete;
    PoshRuntime(PoshRuntime&&) noexcept = delete;
    PoshRuntime& operator=(PoshRuntime&&) noexcept = delete;

    /// @brief returns active runtime
    ///
    /// @return active runtime
    static PoshRuntime& getInstance() noexcept;

    /// @brief creates the runtime with given name
    ///
    /// @param[in] name used for registering the process with the RouDi daemon.
    ///            Must be a valid platform-independent file name, see
    ///            iox::isValidPathEntry
    ///
    /// @return active runtime
    static PoshRuntime& initRuntime(const RuntimeName_t& name) noexcept;

    /// @brief provides an object to extend the lifetime of the runtime
    /// @details While the PoshRuntime has static lifetime, it may not live long enough
    ///          when other static variables depend, possibly indirectly, on the PoshRuntime.
    ///          This is why its lifetime can be extended with this refcounting mechanism.
    ///          Those other static variables should store a lifetime participant object as a
    ///          static variable in the same translation unit before itself, to ensure that the
    ///          lifetime participant and thus the PoshRuntime lives long enough.
    ///
    /// @return an opaque RAII object
    static ScopeGuard getLifetimeParticipant() noexcept;

    /// @brief get the name that was used to register with RouDi
    /// @return name of the registered application
    RuntimeName_t getInstanceName() const noexcept;

    /// @brief initiates the shutdown of the runtime to unblock all potentially blocking publisher
    /// with the ConsumerTooSlowPolicy::WAIT_FOR_CONSUMER option set
    void shutdown() noexcept;

    /// @brief request the RouDi daemon to create a publisher port
    /// @param[in] serviceDescription service description for the new publisher port
    /// @param[in] publisherOptions like the history capacity of a publisher
    /// @param[in] portConfigInfo configuration information for the port
    /// (i.e. what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created publisher port user
    virtual PublisherPortUserType::MemberType_t*
    getMiddlewarePublisher(const capro::ServiceDescription& service,
                           const popo::PublisherOptions& publisherOptions = {},
                           const PortConfigInfo& portConfigInfo = {}) noexcept = 0;

    /// @brief request the RouDi daemon to create a subscriber port
    /// @param[in] serviceDescription service description for the new subscriber port
    /// @param[in] subscriberOptions like the queue capacity and history requested by a subscriber
    /// @param[in] portConfigInfo configuration information for the port
    /// (what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created subscriber port data
    virtual SubscriberPortUserType::MemberType_t*
    getMiddlewareSubscriber(const capro::ServiceDescription& service,
                            const popo::SubscriberOptions& subscriberOptions = {},
                            const PortConfigInfo& portConfigInfo = {}) noexcept = 0;

    /// @brief request the RouDi daemon to create a client port
    /// @param[in] serviceDescription service description for the new client port
    /// @param[in] clientOptions like the queue capacity and queue full policy by a client
    /// @param[in] portConfigInfo configuration information for the port
    /// (what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created client port data
    virtual popo::ClientPortData*
    getMiddlewareClient(const capro::ServiceDescription& service,
                        const popo::ClientOptions& clientOptions = {},
                        const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept = 0;

    /// @brief request the RouDi daemon to create a server port
    /// @param[in] serviceDescription service description for the new server port
    /// @param[in] serverOptions like the queue capacity and queue full policy by a server
    /// @param[in] portConfigInfo configuration information for the port
    /// (what type of port is requested, device where its payload memory is located on etc.)
    /// @return pointer to a created server port data
    virtual popo::ServerPortData*
    getMiddlewareServer(const capro::ServiceDescription& service,
                        const popo::ServerOptions& serverOptions = {},
                        const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept = 0;

    /// @brief request the RouDi daemon to create an interface port
    /// @param[in] interface interface to create
    /// @param[in] nodeName name of the node where the interface should belong to
    /// @return pointer to a created interface port data
    virtual popo::InterfacePortData* getMiddlewareInterface(const capro::Interfaces interface,
                                                            const NodeName_t& nodeName = {}) noexcept = 0;

    /// @brief request the RouDi daemon to create a condition variable
    /// @return pointer to a created condition variable data
    virtual popo::ConditionVariableData* getMiddlewareConditionVariable() noexcept = 0;

    /// @brief send a request to the RouDi daemon and get the response
    ///        currently each request is followed by a response
    /// @param[in] msg request message to send
    /// @param[out] response from the RouDi daemon
    /// @return true if sucessful request/response, false on error
    virtual bool sendRequestToRouDi(const IpcMessage& msg, IpcMessage& answer) noexcept = 0;

  protected:
    friend class roudi_env::RuntimeTestInterface;
    using factory_t = PoshRuntime& (*)(optional<const RuntimeName_t*>);

    // Protected constructor for derived classes
    PoshRuntime(optional<const RuntimeName_t*> name) noexcept;

    static PoshRuntime& defaultRuntimeFactory(optional<const RuntimeName_t*> name) noexcept;

    /// @brief gets current runtime factory. If the runtime factory is not yet initialized it is set to
    /// defaultRuntimeFactory.
    ///
    /// @return current runtime factory
    static factory_t& getRuntimeFactory() noexcept;

    /// @brief sets runtime factory, terminates if given factory is empty
    ///
    /// @param[in] factory function pointer to which the runtime factory should be set
    static void setRuntimeFactory(const factory_t& factory) noexcept;

    /// @brief creates the runtime or returns the already existing one -> Singleton
    ///
    /// @param[in] name optional containing the name used for registering with the RouDi daemon
    ///
    /// @return active runtime
    static PoshRuntime& getInstance(optional<const RuntimeName_t*> name) noexcept;

    /// @brief checks the given application name for certain constraints like length or if is empty
    const RuntimeName_t& verifyInstanceName(optional<const RuntimeName_t*> name) noexcept;

    const RuntimeName_t m_appName;
    concurrent::Atomic<bool> m_shutdownRequested{false};
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_POSH_RUNTIME_HPP
