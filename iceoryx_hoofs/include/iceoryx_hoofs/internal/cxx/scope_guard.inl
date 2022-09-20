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

#ifndef IOX_HOOFS_CXX_SCOPE_GUARD_INL
#define IOX_HOOFS_CXX_SCOPE_GUARD_INL

#include "iceoryx_hoofs/cxx/scope_guard.hpp"

namespace iox
{
namespace cxx
{
template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>::ScopeGuardWithVariableCapacity(
    const cxx::function<void(), CleanupCapacity>& cleanupFunction) noexcept
    : m_cleanupFunction(cleanupFunction)
{
}

template <uint64_t CleanupCapacity>
inline ScopeGuardWithVariableCapacity<CleanupCapacity>::ScopeGuardWithVariableCapacity(
    const function_ref<void()>& initFunction, const function<void(), CleanupCapacity>& cleanupFunction) noexcept
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
inline void ScopeGuardWithVariableCapacity<CleanupCapacity>::destroy() noexcept
{
    if (m_cleanupFunction)
    {
        m_cleanupFunction.value()();
    }
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_GENERIC_RAII_INL
