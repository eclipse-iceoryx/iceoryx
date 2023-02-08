// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_POOL_DATA_HPP
#define IOX_POSH_ROUDI_PORT_POOL_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/client_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/interface_port.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/server_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/runtime/node_data.hpp"
#include "iox/optional.hpp"
#include "iox/vector.hpp"

namespace iox
{
namespace roudi
{
/// @brief workaround container until we have a fixed list with the needed functionality
template <typename T, uint64_t Capacity>
class FixedPositionContainer
{
  public:
    static constexpr uint64_t FIRST_ELEMENT = std::numeric_limits<uint64_t>::max();

    bool hasFreeSpace() noexcept;

    template <typename... Targs>
    T* insert(Targs&&... args) noexcept;

    void erase(const T* const element) noexcept;

    vector<T*, Capacity> content() noexcept;

  private:
    vector<optional<T>, Capacity> m_data;
};

struct PortPoolData
{
    FixedPositionContainer<popo::InterfacePortData, MAX_INTERFACE_NUMBER> m_interfacePortMembers;
    FixedPositionContainer<runtime::NodeData, MAX_NODE_NUMBER> m_nodeMembers;
    FixedPositionContainer<popo::ConditionVariableData, MAX_NUMBER_OF_CONDITION_VARIABLES> m_conditionVariableMembers;

    FixedPositionContainer<iox::popo::PublisherPortData, MAX_PUBLISHERS> m_publisherPortMembers;
    FixedPositionContainer<iox::popo::SubscriberPortData, MAX_SUBSCRIBERS> m_subscriberPortMembers;

    FixedPositionContainer<iox::popo::ServerPortData, MAX_SERVERS> m_serverPortMembers;
    FixedPositionContainer<iox::popo::ClientPortData, MAX_CLIENTS> m_clientPortMembers;
};

} // namespace roudi
} // namespace iox

#include "iceoryx_posh/internal/roudi/port_pool_data.inl"

#endif // IOX_POSH_ROUDI_PORT_POOL_DATA_BASE_HPP
