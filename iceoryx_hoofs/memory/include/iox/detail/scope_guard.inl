// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MEMORY_SCOPE_GUARD_INL
#define IOX_HOOFS_MEMORY_SCOPE_GUARD_INL

#include "iox/scope_guard.hpp"

namespace iox
{
template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>::ScopeGuardWithVariableCapacity(
    const CleanupFunction& cleanupFunction) noexcept
    : m_cleanupFunction(cleanupFunction)
{
}

template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>::ScopeGuardWithVariableCapacity(
    const InitFunction initFunction, const CleanupFunction& cleanupFunction) noexcept
    : ScopeGuardWithVariableCapacity(cleanupFunction)
{
    initFunction();
}

template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>::~ScopeGuardWithVariableCapacity() noexcept
{
    destroy();
}

template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>::ScopeGuardWithVariableCapacity(
    ScopeGuardWithVariableCapacity&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>& ScopeGuardWithVariableCapacity<CleanupCapacity>::operator=(
    ScopeGuardWithVariableCapacity<CleanupCapacity>&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();
        m_cleanupFunction = rhs.m_cleanupFunction;
        rhs.m_cleanupFunction.reset();
    }
    return *this;
}

template <uint64_t CleanupCapacity>
inline typename ScopeGuardWithVariableCapacity<CleanupCapacity>::CleanupFunction
ScopeGuardWithVariableCapacity<CleanupCapacity>::release(
    // NOLINTNEXTLINE(cppcoreguidelines-rvalue-reference-param-not-moved) false positive
    ScopeGuardWithVariableCapacity<CleanupCapacity>&& scopeGuard) noexcept
{
    IOX_ENFORCE(scopeGuard.m_cleanupFunction.has_value(),
                "Cleanup function must always have a value if the 'ScopeGuard' is not a moved from object.");
    CleanupFunction cleanupFunction = std::move(scopeGuard.m_cleanupFunction.value());
    scopeGuard.m_cleanupFunction.reset();
    return cleanupFunction;
}

template <uint64_t CleanupCapacity>
inline void ScopeGuardWithVariableCapacity<CleanupCapacity>::destroy() noexcept
{
    if (m_cleanupFunction)
    {
        m_cleanupFunction.value()();
    }
}

} // namespace iox

#endif // IOX_HOOFS_MEMORY_SCOPE_GUARD_INL
