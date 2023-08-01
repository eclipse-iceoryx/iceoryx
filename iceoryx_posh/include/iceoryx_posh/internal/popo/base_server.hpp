// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_BASE_SERVER_HPP
#define IOX_POSH_POPO_BASE_SERVER_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_user.hpp"
#include "iceoryx_posh/popo/server_options.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/expected.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

/// @brief The BaseServer class contains the common implementation for the different server
/// @param[in] PortT type of the underlying port, required for testing specializations.
/// @param[in] TriggerHandleT type of the underlying trigger handle, required for testing
/// @note Not intended for public usage! Use the 'Server' or 'UntypedServer' instead!
template <typename PortT = ServerPortUser, typename TriggerHandleT = TriggerHandle>
class BaseServer
{
  public:
    virtual ~BaseServer() noexcept;

    BaseServer(const BaseServer& other) = delete;
    BaseServer& operator=(const BaseServer&) = delete;
    BaseServer(BaseServer&& rhs) = delete;
    BaseServer& operator=(BaseServer&& rhs) = delete;

    ///
    /// @brief Get the UID of the server.
    /// @return The server's UID.
    ///
    uid_t getUid() const noexcept;

    ///
    /// @brief Get the service description of the server.
    /// @return A reference to the service description.
    ///
    const capro::ServiceDescription& getServiceDescription() const noexcept;

    ///
    /// @brief Offer the service to be connected to when not already offering, otherwise nothing.
    ///
    void offer() noexcept;

    ///
    /// @brief Stop offering the service when already offering, otherwise nothing.
    ///
    void stopOffer() noexcept;

    ///
    /// @brief Check if the server is offering.
    /// @return True if service is currently being offered.
    ///
    bool isOffered() const noexcept;

    ///
    /// @brief Check if the server has clients
    /// @return True if currently has subscribers to the service.
    ///
    bool hasClients() const noexcept;

    ///
    /// @brief Check if requests are available.
    /// @return True if requests are available.
    ///
    bool hasRequests() const noexcept;

    ///
    /// @brief Check if requests has been missed since the last call of this method.
    /// @return True if requests has been missed.
    /// @details Requests may be missed due to overflowing receive queue.
    ///
    bool hasMissedRequests() noexcept;

    /// @brief Releases any unread queued requests.
    void releaseQueuedRequests() noexcept;

    friend class NotificationAttorney;

  protected:
    using SelfType = BaseServer<PortT, TriggerHandleT>;
    using PortType = PortT;

    BaseServer(const capro::ServiceDescription& service, const ServerOptions& serverOptions) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Invalidates the internal triggerHandle.
    /// @param[in] uniqueTriggerId the id of the corresponding trigger
    void invalidateTrigger(const uint64_t uniqueTriggerId) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Attaches the triggerHandle to the internal
    /// trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] serverState the state which should be attached
    void enableState(TriggerHandleT&& triggerHandle, const ServerState serverState) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Returns method pointer to the event
    /// corresponding hasTriggered method callback
    /// @param[in] serverState the state to which the hasTriggeredCallback is required
    WaitSetIsConditionSatisfiedCallback
    getCallbackForIsStateConditionSatisfied(const ServerState serverState) const noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Resets the internal triggerHandle
    /// @param[in] serverState the state which should be detached
    void disableState(const ServerState serverState) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Attaches the triggerHandle to the internal
    /// trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] serverEvent the event which should be attached
    void enableEvent(TriggerHandleT&& triggerHandle, const ServerEvent serverEvent) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Resets the internal triggerHandle
    /// @param[in] serverEvent the event which should be detached
    void disableEvent(const ServerEvent serverEvent) noexcept;

    ///
    /// @brief port
    /// @return const accessor of the underlying port
    ///
    const PortT& port() const noexcept;

    ///
    /// @brief port
    /// @return accessor of the underlying port
    ///
    PortT& port() noexcept;

    PortT m_port;             // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
    TriggerHandleT m_trigger; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/base_server.inl"

#endif // IOX_POSH_POPO_BASE_SERVER_HPP
