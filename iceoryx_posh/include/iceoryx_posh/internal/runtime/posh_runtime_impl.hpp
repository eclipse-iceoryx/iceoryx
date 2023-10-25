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
#ifndef IOX_POSH_RUNTIME_POSH_RUNTIME_IMPL_HPP
#define IOX_POSH_RUNTIME_POSH_RUNTIME_IMPL_HPP

#include "iceoryx_hoofs/internal/concurrent/periodic_task.hpp"
#include "iceoryx_hoofs/internal/posix_wrapper/mutex.hpp"
#include "iceoryx_posh/internal/runtime/heartbeat.hpp"
#include "iceoryx_posh/internal/runtime/shared_memory_user.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/function.hpp"
#include "iox/optional.hpp"

namespace iox
{
namespace runtime
{
enum class RuntimeLocation
{
    SEPARATE_PROCESS_FROM_ROUDI,
    SAME_PROCESS_LIKE_ROUDI,
};

/// @brief The runtime that is needed for each application to communicate with the RouDi daemon
class PoshRuntimeImpl : public PoshRuntime
{
  public:
    PoshRuntimeImpl(const PoshRuntimeImpl&) = delete;
    PoshRuntimeImpl& operator=(const PoshRuntimeImpl&) = delete;
    PoshRuntimeImpl(PoshRuntimeImpl&&) = delete;
    PoshRuntimeImpl& operator=(PoshRuntimeImpl&&) = delete;
    virtual ~PoshRuntimeImpl() noexcept;

    /// @copydoc PoshRuntime::getMiddlewarePublisher
    PublisherPortUserType::MemberType_t*
    getMiddlewarePublisher(const capro::ServiceDescription& service,
                           const popo::PublisherOptions& publisherOptions = popo::PublisherOptions(),
                           const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept override;

    /// @copydoc PoshRuntime::getMiddlewareSubscriber
    SubscriberPortUserType::MemberType_t*
    getMiddlewareSubscriber(const capro::ServiceDescription& service,
                            const popo::SubscriberOptions& subscriberOptions = popo::SubscriberOptions(),
                            const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept override;

    /// @copydoc PoshRuntime::getMiddlewareClient
    popo::ClientPortUser::MemberType_t*
    getMiddlewareClient(const capro::ServiceDescription& service,
                        const popo::ClientOptions& clientOptions = {},
                        const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept override;

    /// @copydoc PoshRuntime::getMiddlewareServer
    popo::ServerPortUser::MemberType_t*
    getMiddlewareServer(const capro::ServiceDescription& service,
                        const popo::ServerOptions& ServerOptions = {},
                        const PortConfigInfo& portConfigInfo = PortConfigInfo()) noexcept override;

    /// @copydoc PoshRuntime::getMiddlewareInterface
    popo::InterfacePortData* getMiddlewareInterface(const capro::Interfaces interface,
                                                    const NodeName_t& nodeName = {""}) noexcept override;

    /// @copydoc PoshRuntime::getMiddlewareConditionVariable
    popo::ConditionVariableData* getMiddlewareConditionVariable() noexcept override;

    /// @copydoc PoshRuntime::createNode
    NodeData* createNode(const NodeProperty& nodeProperty) noexcept override;

    /// @copydoc PoshRuntime::sendRequestToRouDi
    bool sendRequestToRouDi(const IpcMessage& msg, IpcMessage& answer) noexcept override;

  protected:
    friend class PoshRuntime;
    friend class roudi_env::RuntimeTestInterface;

    // Protected constructor for IPC setup
    PoshRuntimeImpl(optional<const RuntimeName_t*> name,
                    const RuntimeLocation location = RuntimeLocation::SEPARATE_PROCESS_FROM_ROUDI) noexcept;

  private:
    expected<PublisherPortUserType::MemberType_t*, IpcMessageErrorType>
    requestPublisherFromRoudi(const IpcMessage& sendBuffer) noexcept;

    expected<SubscriberPortUserType::MemberType_t*, IpcMessageErrorType>
    requestSubscriberFromRoudi(const IpcMessage& sendBuffer) noexcept;

    expected<popo::ClientPortUser::MemberType_t*, IpcMessageErrorType>
    requestClientFromRoudi(const IpcMessage& sendBuffer) noexcept;

    expected<popo::ServerPortUser::MemberType_t*, IpcMessageErrorType>
    requestServerFromRoudi(const IpcMessage& sendBuffer) noexcept;

    expected<popo::ConditionVariableData*, IpcMessageErrorType>
    requestConditionVariableFromRoudi(const IpcMessage& sendBuffer) noexcept;

    mutable optional<posix::mutex> m_appIpcRequestMutex;

    IpcRuntimeInterface m_ipcChannelInterface;
    optional<SharedMemoryUser> m_ShmInterface;

    optional<Heartbeat*> m_heartbeat;
    void sendKeepAliveAndHandleShutdownPreparation() noexcept;

    // the m_keepAliveTask should always be the last member, so that it will be the first member to be destroyed
    optional<concurrent::PeriodicTask<function<void()>>> m_keepAliveTask;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_POSH_RUNTIME_IMPL_HPP
