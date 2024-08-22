// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_POPO_PORTS_BASE_PORT_DATA_HPP
#define IOX_POSH_POPO_PORTS_BASE_PORT_DATA_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/unique_port_id.hpp"
#include "iox/atomic.hpp"
#include "iox/relative_pointer.hpp"

namespace iox
{
namespace popo
{
/// @brief Defines different base port data
struct BasePortData
{
    /// @brief Constructor
    /// @param[in] serviceDescription creates the service service description
    /// @param[in] runtimeName Name of the application's runtime
    /// @param[in] nodeName Name of the node
    /// @param[in] uniqueRouDiId to tie the port to
    BasePortData(const capro::ServiceDescription& serviceDescription,
                 const RuntimeName_t& runtimeName,
                 const roudi::UniqueRouDiId uniqueRoudiId) noexcept;

    BasePortData(const BasePortData&) = delete;
    BasePortData& operator=(const BasePortData&) = delete;
    BasePortData(BasePortData&&) = delete;
    BasePortData& operator=(BasePortData&&) = delete;
    ~BasePortData() noexcept = default;

    capro::ServiceDescription m_serviceDescription;
    RuntimeName_t m_runtimeName;
    UniquePortId m_uniqueId;
    concurrent::Atomic<bool> m_toBeDestroyed{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_BASE_PORT_DATA_HPP
