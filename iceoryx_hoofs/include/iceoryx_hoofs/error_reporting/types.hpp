// Copyright (c) 2023 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_ERROR_REPORTING_TYPES_HPP
#define IOX_HOOFS_ERROR_REPORTING_TYPES_HPP

#include <cstdint>

namespace iox
{
namespace er
{

// These are lightweight regular read/write types that do not require encapsulation (no invariants
// can be broken).

struct ErrorCode
{
    using type = uint32_t;

    type value;

    static constexpr type ASSUMPTION_VIOLATION{0};
    static constexpr type PRECONDITION_VIOLATION{1};

    constexpr explicit ErrorCode(uint32_t value)
        : value(value)
    {
    }

    bool operator==(const ErrorCode& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const ErrorCode& rhs) const
    {
        return !(*this == rhs);
    }
};

struct ModuleId
{
    using type = uint32_t;

    type value;

    static constexpr type ANY{0};

    // User module ids should be larger or equal than this to avoid conflicts
    // with internal modules.
    // All lower values are reserved.
    static constexpr type USER_MODULE_BASE{0x100};

    constexpr explicit ModuleId(uint32_t value = ANY)
        : value(value)
    {
    }

    bool operator==(const ModuleId& rhs) const
    {
        return value == rhs.value;
    }

    bool operator!=(const ModuleId& rhs) const
    {
        return !(*this == rhs);
    }
};

} // namespace er
} // namespace iox

#endif
