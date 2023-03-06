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

#ifndef IOX_POSH_POPO_BASE_CLIENT_HPP
#define IOX_POSH_POPO_BASE_CLIENT_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iox/expected.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

/// @brief The BaseClient class contains the common implementation for the different clients
/// @param[in] PortT type of the underlying port, required for testing
/// @param[in] TriggerHandleT type of the underlying trigger handle, required for testing
/// @note Not intended for public usage! Use the 'Client' or 'UntypedClient' instead!
template <typename PortT = ClientPortUser, typename TriggerHandleT = TriggerHandle>
class BaseClient
{
  public:
    virtual ~BaseClient() noexcept;

    BaseClient(const BaseClient& other) = delete;
    BaseClient& operator=(const BaseClient&) = delete;
    BaseClient(BaseClient&& rhs) = delete;
    BaseClient& operator=(BaseClient&& rhs) = delete;

    ///
    /// @brief Get the unique ID of the client.
    /// @return The client's unique ID.
    ///
    uid_t getUid() const noexcept;

    ///
    /// @brief Get the service description of the client.
    /// @return A reference to the service description.
    ///
    const capro::ServiceDescription& getServiceDescription() const noexcept;

    ///
    /// @brief Initiate connection to server when not already connected, otherwise nothing.
    ///
    void connect() noexcept;

    ///
    /// @brief Get current connection state.
    /// @return The current connection state.
    ///
    ConnectionState getConnectionState() const noexcept;

    ///
    /// @brief Disconnects when already connected, otherwise nothing.
    ///
    void disconnect() noexcept;

    ///
    /// @brief Check if response are available.
    /// @return True if responses are available.
    ///
    bool hasResponses() const noexcept;

    ///
    /// @brief Check if response has been missed since the last call of this method.
    /// @return True if response has been missed.
    /// @details Response may be missed due to overflowing receive queue.
    ///
    bool hasMissedResponses() noexcept;

    /// @brief Releases any unread queued response.
    void releaseQueuedResponses() noexcept;

    friend class NotificationAttorney;

  protected:
    using SelfType = BaseClient<PortT, TriggerHandleT>;
    using PortType = PortT;

    BaseClient(const capro::ServiceDescription& service, const ClientOptions& clientOptions) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Invalidates the internal triggerHandle.
    /// @param[in] uniqueTriggerId the id of the corresponding trigger
    void invalidateTrigger(const uint64_t uniqueTriggerId) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Attaches the triggerHandle to the internal
    /// trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] clientState the state which should be attached
    void enableState(TriggerHandleT&& triggerHandle, const ClientState clientState) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Returns method pointer to the event
    /// corresponding hasTriggered method callback
    /// @param[in] clientState the state to which the hasTriggeredCallback is required
    WaitSetIsConditionSatisfiedCallback
    getCallbackForIsStateConditionSatisfied(const ClientState clientState) const noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Resets the internal triggerHandle
    /// @param[in] clientState the state which should be detached
    void disableState(const ClientState clientState) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Attaches the triggerHandle to the internal
    /// trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] clientEvent the event which should be attached
    void enableEvent(TriggerHandleT&& triggerHandle, const ClientEvent clientEvent) noexcept;

    /// @brief Only usable by the WaitSet/Listener, not for public use. Resets the internal triggerHandle
    /// @param[in] clientEvent the event which should be detached
    void disableEvent(const ClientEvent clientEvent) noexcept;

    ///
    /// @brief const accessor of the underlying port
    ///
    const PortT& port() const noexcept;

    ///
    /// @brief accessor of the underlying port
    ///
    PortT& port() noexcept;

  protected:
    PortT m_port;             // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
    TriggerHandleT m_trigger; // NOLINT(cppcoreguidelines-non-private-member-variables-in-classes)
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/base_client.inl"

#endif // IOX_POSH_POPO_BASE_CLIENT_HPP
