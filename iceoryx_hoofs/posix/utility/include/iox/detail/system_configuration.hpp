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
#ifndef IOX_HOOFS_POSIX_UTILITY_SYSTEM_CONFIGURATION_HPP
#define IOX_HOOFS_POSIX_UTILITY_SYSTEM_CONFIGURATION_HPP

#include "iox/iceoryx_hoofs_deployment.hpp"

#include <cstdint>

namespace iox
{
namespace detail
{
/// @brief returns the page size of the system
uint64_t pageSize() noexcept;

/// @brief Returns info whether called on a 32-bit system
/// @return True if called on 32-bit, false if not 32-bit system
constexpr bool isCompiledOn32BitSystem() noexcept
{
    return INTPTR_MAX == INT32_MAX;
}
} // namespace detail
} // namespace iox

#endif // IOX_HOOFS_POSIX_UTILITY_SYSTEM_CONFIGURATION_HPP
