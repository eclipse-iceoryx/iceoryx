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
#ifndef IOX_POSH_MEPOO_MEMORY_INFO_HPP
#define IOX_POSH_MEPOO_MEMORY_INFO_HPP

#include <cstdint>

namespace iox
{
namespace mepoo
{
/// @brief Stores properties of the memory to be used when we distinguish between
/// different types of memory on e.g. different devices with different characteristics.
struct MemoryInfo
{
    static constexpr uint32_t DEFAULT_DEVICE_ID{0U};
    static constexpr uint32_t DEFAULT_MEMORY_TYPE{0U};

    // These are intentionally not defined as enum classes for flexibility and extendibility.
    // Currently only the defaults are used.
    // This will change when we support different devices (CPU, GPUs, ...)
    // and other properties that influence how memory is accessed.

    uint32_t deviceId{DEFAULT_DEVICE_ID};
    uint32_t memoryType{DEFAULT_MEMORY_TYPE};

    MemoryInfo(const MemoryInfo&) noexcept = default;
    MemoryInfo(MemoryInfo&&) noexcept = default;
    MemoryInfo& operator=(const MemoryInfo&) noexcept = default;
    MemoryInfo& operator=(MemoryInfo&&) noexcept = default;

    /// @brief creates a MemoryInfo object
    /// @param[in] deviceId specifies the device where the memory is located
    /// @param[in] memoryType encodes additional information about the memory
    explicit MemoryInfo(uint32_t deviceId = DEFAULT_DEVICE_ID, uint32_t memoryType = DEFAULT_MEMORY_TYPE) noexcept;

    /// @brief comparison operator
    /// @param[in] rhs the right hand side of the comparison
    bool operator==(const MemoryInfo& rhs) const noexcept;
};
} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEMORY_INFO_HPP
