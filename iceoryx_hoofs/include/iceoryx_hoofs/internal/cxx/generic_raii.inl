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

#ifndef IOX_HOOFS_CXX_GENERIC_RAII_INL
#define IOX_HOOFS_CXX_GENERIC_RAII_INL

#include "iceoryx_hoofs/cxx/generic_raii.hpp"

namespace iox
{
namespace cxx
{
template <uint64_t Capacity>
inline GenericRAIIWithVariableCapacity<Capacity>::GenericRAIIWithVariableCapacity(
    const cxx::function<void(), Capacity>& cleanupFunction) noexcept
    : GenericRAIIWithVariableCapacity(function_ref<void()>(), cleanupFunction)
{
}

template <uint64_t Capacity>
inline GenericRAIIWithVariableCapacity<Capacity>::GenericRAIIWithVariableCapacity(
    const function_ref<void()>& initFunction, const function<void()>& cleanupFunction) noexcept
    : m_cleanupFunction(cleanupFunction)
{
    if (initFunction)
    {
        initFunction();
    }
}

template <uint64_t Capacity>
inline GenericRAIIWithVariableCapacity<Capacity>::~GenericRAIIWithVariableCapacity() noexcept
{
    destroy();
}

template <uint64_t Capacity>
inline GenericRAIIWithVariableCapacity<Capacity>::GenericRAIIWithVariableCapacity(
    GenericRAIIWithVariableCapacity&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <uint64_t Capacity>
inline GenericRAIIWithVariableCapacity<Capacity>&
GenericRAIIWithVariableCapacity<Capacity>::operator=(GenericRAIIWithVariableCapacity<Capacity>&& rhs) noexcept
{
    if (this != &rhs)
    {
        destroy();
        m_cleanupFunction = rhs.m_cleanupFunction;
        rhs.m_cleanupFunction = function<void()>();
    }
    return *this;
}

template <uint64_t Capacity>
inline void GenericRAIIWithVariableCapacity<Capacity>::destroy() noexcept
{
    if (m_cleanupFunction)
    {
        m_cleanupFunction();
        m_cleanupFunction = function<void()>();
    }
}

} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_GENERIC_RAII_INL
