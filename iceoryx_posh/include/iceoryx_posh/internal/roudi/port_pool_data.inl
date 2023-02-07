// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_PORT_POOL_DATA_INL
#define IOX_POSH_ROUDI_PORT_POOL_DATA_INL

#include "iceoryx_posh/internal/roudi/port_pool_data.hpp"

namespace iox
{
namespace roudi
{
template <typename T, uint64_t Capacity>
bool FixedPositionContainer<T, Capacity>::hasFreeSpace() noexcept
{
    if (m_data.capacity() > m_data.size())
    {
        return true;
    }

    for (auto& e : m_data)
    {
        if (!e.has_value())
        {
            return true;
        }
    }

    return false;
}

template <typename T, uint64_t Capacity>
template <typename... Targs>
T* FixedPositionContainer<T, Capacity>::insert(Targs&&... args) noexcept
{
    for (auto& e : m_data)
    {
        if (!e.has_value())
        {
            e.emplace(std::forward<Targs>(args)...);
            return &e.value();
        }
    }

    m_data.emplace_back();
    m_data.back().emplace(std::forward<Targs>(args)...);
    return &m_data.back().value();
}

template <typename T, uint64_t Capacity>
void FixedPositionContainer<T, Capacity>::erase(const T* const element) noexcept
{
    for (auto& e : m_data)
    {
        if (e.has_value() && &e.value() == element)
        {
            e.reset();
            return;
        }
    }
}

template <typename T, uint64_t Capacity>
vector<T*, Capacity> FixedPositionContainer<T, Capacity>::content() noexcept
{
    vector<T*, Capacity> returnValue;
    for (auto& e : m_data)
    {
        if (e.has_value())
        {
            returnValue.emplace_back(&e.value());
        }
    }
    return returnValue;
}

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_PORT_POOL_DATA_BASE_INL
