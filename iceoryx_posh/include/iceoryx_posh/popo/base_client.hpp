// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/popo/ports/client_port_user.hpp"
#include "iceoryx_posh/popo/client_options.hpp"
#include "iceoryx_posh/popo/request.hpp"
#include "iceoryx_posh/internal/popo/sample_deleter.hpp"
#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_posh/popo/enum_trigger_type.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"
#include "iceoryx_posh/popo/listener.hpp"
#include "iceoryx_hoofs/cxx/unique_ptr.hpp"

namespace iox
{
namespace popo
{
using uid_t = UniquePortId;

enum class ClientEvent : EventEnumIdentifier
{
    DATA_RECEIVED
};

enum class ClientState : StateEnumIdentifier
{
    HAS_DATA
};
///
/// @brief The BaseClient class contains the common implementation for the different client specializations.
///
template <typename port_t = ClientPortUser>
class BaseClient
{
  protected:
    using SelfType = BaseClient<port_t>;
    using PortType = port_t;

    BaseClient(const BaseClient& other) = delete;
    BaseClient& operator=(const BaseClient&) = delete;
    BaseClient(BaseClient&& rhs) = delete;
    BaseClient& operator=(BaseClient&& rhs) = delete;
    virtual ~BaseClient();

    ///
    /// @brief uid Get the UID of the client.
    /// @return The client's UID.
    ///
    uid_t getUid() const noexcept;

    ///
    /// @brief getServiceDescription Get the service description of the client.
    /// @return The service description.
    ///
    capro::ServiceDescription getServiceDescription() const noexcept;

    ///
    /// @brief connect Establish connection with the available servers.
    ///
    void connect() noexcept;

    ///
    /// @brief disconnect Stop offering the service.
    ///
    void disconnect() noexcept;

    ///
    /// @brief getConnectionState
    /// @return Current connection state of client.
    ///
    ConnectionState getConnectionState() const noexcept;

    ///
    /// @brief returns true if there are responses in queue
    /// @return True if service is currently being offered.
    ///
    bool hasResponses() const noexcept;

    ///
    /// @brief Gets the responses from the response queue
    /// @return Pointer to the response
    ///
    cxx::expected<const ResponseHeader*, ChunkReceiveResult> takeResponses() noexcept;

    bool hasMissedResponses() noexcept;

    /// @brief Releases any unread queued data.
    void releaseQueuedResponses() noexcept;

    friend class NotificationAttorney;

  protected:
    BaseClient() = default; // Required for testing.
    BaseClient(const capro::ServiceDescription& service, const ClientOptions& clientOptions);

    ///
    /// @brief port
    /// @return const accessor of the underlying port
    ///
    const port_t& port() const noexcept;

    ///
    /// @brief port
    /// @return accessor of the underlying port
    ///
    port_t& port() noexcept;

    port_t m_port{nullptr};
    TriggerHandle m_trigger;

    void invalidateTrigger(const uint64_t trigger) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Attaches the triggerHandle to the internal trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] clientState the state which should be attached
    void enableState(iox::popo::TriggerHandle&& triggerHandle, const ClientState clientState) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Returns method pointer to the event corresponding
    /// hasTriggered method callback
    /// @param[in] clientState the state to which the hasTriggeredCallback is required
    WaitSetIsConditionSatisfiedCallback
    getCallbackForIsStateConditionSatisfied(const ClientState clientState) const noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Resets the internal triggerHandle
    /// @param[in] clientState the state which should be detached
    void disableState(const ClientState clientState) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Attaches the triggerHandle to the internal trigger.
    /// @param[in] triggerHandle rvalue reference to the triggerHandle. This class takes the ownership of that handle.
    /// @param[in] clientEvent the event which should be attached
    void enableEvent(iox::popo::TriggerHandle&& triggerHandle, const ClientEvent clientEvent) noexcept;

    /// @brief Only usable by the WaitSet, not for public use. Resets the internal triggerHandle
    /// @param[in] clientEvent the event which should be detached
    void disableEvent(const ClientEvent clientEvent) noexcept;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/base_client.inl"

#endif // IOX_POSH_POPO_BASE_CLIENT_HPP
