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

#include "iceoryx_utils/internal/concurrent/fifo.hpp"

namespace iox
{
namespace concurrent
{
template <class ValueType, uint32_t Capacity>
inline bool FiFo<ValueType, Capacity>::push(const ValueType& f_param_r)
{
    if (is_full())
    {
        return false;
    }
    else
    {
        m_data[m_write_pos.load(std::memory_order_relaxed) % Capacity] = f_param_r;

        // m_write_pos must be increased after writing the new value otherwise
        // it is possible that the value is read by pop while it is written
        m_write_pos.fetch_add(1, std::memory_order_acq_rel);
        return true;
    }
}

template <class ValueType, uint32_t Capacity>
inline bool FiFo<ValueType, Capacity>::is_full() const
{
    return m_write_pos.load(std::memory_order_relaxed) == m_read_pos.load(std::memory_order_relaxed) + Capacity;
}

template <class ValueType, uint32_t Capacity>
inline bool FiFo<ValueType, Capacity>::empty() const
{
    return m_read_pos.load(std::memory_order_relaxed) == m_write_pos.load(std::memory_order_relaxed);
}

template <class ValueType, uint32_t Capacity>
inline cxx::optional<ValueType> FiFo<ValueType, Capacity>::pop()
{
    if (empty())
    {
        return cxx::nullopt_t();
    }
    else
    {
        ValueType out = m_data[m_read_pos.load(std::memory_order_acquire) % Capacity];

        // m_read_pos must be increased after reading the pop'ed value otherwise
        // it is possible that the pop'ed value is overwritten by push while it is read
        m_read_pos.fetch_add(1, std::memory_order_relaxed);
        return out;
    }
}
} // namespace concurrent
} // namespace iox
