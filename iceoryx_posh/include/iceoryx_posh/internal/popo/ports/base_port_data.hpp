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
#ifndef IOX_POSH_POPO_PORTS_BASE_PORT_DATA_HPP
#define IOX_POSH_POPO_PORTS_BASE_PORT_DATA_HPP

#include "iceoryx_posh/capro/service_description.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <atomic>

namespace iox
{
namespace runtime
{
class RunnableData;
}

namespace popo
{
/// @brief Defines different base port data
struct BasePortData
{
    /// @brief Constructor for base port data members
    BasePortData() = default;

    /// @brief Constructor
    /// @param[in] serviceDescription creates the service service description
    /// @param[in] portType Type of port to be created
    /// @param[in] processName Name of the process
    /// @param[in] runnable The runnable where this port is attached to
    BasePortData(const capro::ServiceDescription& serviceDescription, const ProcessName_t& processName) noexcept;

    BasePortData(const BasePortData&) = delete;
    BasePortData& operator=(const BasePortData&) = delete;
    BasePortData(BasePortData&&) = delete;
    BasePortData& operator=(BasePortData&&) = delete;
    ~BasePortData() = default;

    capro::ServiceDescription m_serviceDescription;
    ProcessName_t m_processName;

    UniquePortId m_uniqueId;
    std::atomic_bool m_toBeDestroyed{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_PORTS_BASE_PORT_DATA_HPP
