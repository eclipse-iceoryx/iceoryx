// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_RUNTIME_PORT_CONFIG_INFO_HPP
#define IOX_POSH_RUNTIME_PORT_CONFIG_INFO_HPP

#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iox/detail/serialization.hpp"

#include <cstdint>

namespace iox
{
namespace runtime
{
/// @brief Stores information necessary to create the right type of
/// port on RouDi side. Different types of ports are required
/// if e.g. different types of shared memory are used (e.g. on GPU).
struct PortConfigInfo
{
    static constexpr uint32_t DEFAULT_PORT_TYPE{0U};
    static constexpr uint32_t DEFAULT_DEVICE_ID{0U};
    static constexpr uint32_t DEFAULT_MEMORY_TYPE{0U};

    // these are intentionally not defined as enum classes for flexibility and extendibility
    // with specific user defined codes used by custom ports
    // values will be resolved at lower level, (i.e. in the port creation factory)

    uint32_t portType{DEFAULT_PORT_TYPE};
    iox::mepoo::MemoryInfo memoryInfo;

    PortConfigInfo(const PortConfigInfo&) noexcept = default;
    PortConfigInfo(PortConfigInfo&&) noexcept = default;
    PortConfigInfo& operator=(const PortConfigInfo&) noexcept = default;
    PortConfigInfo& operator=(PortConfigInfo&&) noexcept = default;

    /// @brief creates a PortConfigInfo object
    /// @param[in] portType specifies the type of port to be created
    /// @param[in] deviceId specifies the device the port operates on (CPU, GPUx etc.)
    /// @param[in] memoryType encodes additional information about the memory used by the port
    PortConfigInfo(uint32_t portType = DEFAULT_PORT_TYPE,
                   uint32_t deviceId = DEFAULT_DEVICE_ID,
                   uint32_t memoryType = DEFAULT_MEMORY_TYPE) noexcept;

    /// @brief creates a PortConfigInfo object from its serialization
    /// @param[in] serialization specifies the serialization from which the port is created
    PortConfigInfo(const Serialization& serialization) noexcept;

    /// @brief creates a serilaization of the PortConfigInfo
    operator Serialization() const noexcept;

    /// @brief comparison operator
    /// @param[in] rhs the right hand side of the comparison
    bool operator==(const PortConfigInfo& rhs) const noexcept;
};

} // namespace runtime
} // namespace iox

#endif // IOX_POSH_RUNTIME_PORT_CONFIG_INFO_HPP
