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
#include "iox/fixed_position_container.hpp"
#include "iox/optional.hpp"
#include "iox/vector.hpp"

namespace iox
{
namespace roudi
{
struct PortPoolData
{
    PortPoolData(const roudi::UniqueRouDiId uniqueRouDiId) noexcept
        : m_uniqueRouDiId(uniqueRouDiId)
    {
    }

    using InterfaceContainer = FixedPositionContainer<popo::InterfacePortData, MAX_INTERFACE_NUMBER>;
    InterfaceContainer m_interfacePortMembers;

    using CondVarContainer = FixedPositionContainer<popo::ConditionVariableData, MAX_NUMBER_OF_CONDITION_VARIABLES>;
    CondVarContainer m_conditionVariableMembers;

    using PublisherContainer = FixedPositionContainer<iox::popo::PublisherPortData, MAX_PUBLISHERS>;
    PublisherContainer m_publisherPortMembers;

    using SubscriberContainer = FixedPositionContainer<iox::popo::SubscriberPortData, MAX_SUBSCRIBERS>;
    SubscriberContainer m_subscriberPortMembers;

    using ServerContainer = FixedPositionContainer<iox::popo::ServerPortData, MAX_SERVERS>;
    ServerContainer m_serverPortMembers;

    using ClientContainer = FixedPositionContainer<iox::popo::ClientPortData, MAX_CLIENTS>;
    ClientContainer m_clientPortMembers;

    const roudi::UniqueRouDiId m_uniqueRouDiId;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_PORT_POOL_DATA_BASE_HPP
