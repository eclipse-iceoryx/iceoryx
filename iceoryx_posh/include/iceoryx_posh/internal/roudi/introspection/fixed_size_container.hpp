// Copyright (c) 2019, 2021 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_INTROSPECTION_FIXED_SIZE_CONTAINER_HPP
#define IOX_POSH_ROUDI_INTROSPECTION_FIXED_SIZE_CONTAINER_HPP

#include "iox/vector.hpp"

#include <cstdint>


namespace iox
{
namespace roudi
{
/// @details allows allocating a predefined capacity of T objects on the stack
/// and using there pointers outside
/// (pointers stay valid until object is removed from theFixedSizeContainer)
/// therefore we avoid heap allocation but we can still use pointers to objects
/// for efficient update/passing until we exhaust the fixed size container

/// @attention no bounds checking during access for efficiency (as in STL containers)
/// access to indices < 0 or >= capacity is undefined behaviour (most likely a
/// segmentation fault)
template <typename T, uint32_t capacity = 1U>
class FixedSizeContainer
{
  public:
    using Index_t = int32_t;
    using Capacity_t = decltype(capacity);
    static constexpr int32_t NOT_AN_ELEMENT = -1;

    FixedSizeContainer() noexcept
        : m_values(capacity)
    {
    }

    /// @note returns index or -1 if element could not be added
    /// a successful add returns an arbitrary index which can be non consecutive
    /// for consecutive adds
    Index_t add(const T& element) noexcept
    {
        auto nextElement = nextFree();

        if (nextElement >= 0)
        {
            m_freeIndex = nextElement;
            m_values[static_cast<Capacity_t>(m_freeIndex)].value = element;
            m_values[static_cast<Capacity_t>(m_freeIndex)].isValid = true;
            ++m_size;
        }

        return nextElement;
    }

    void remove(Index_t index) noexcept
    {
        if (m_values[static_cast<Capacity_t>(index)].isValid)
        {
            m_values[static_cast<Capacity_t>(index)].isValid = false;
            --m_size;
        }
    }

    /// @note access can change the underlying object, without modifying valid flag
    /// if the index is invalid than the behavior is undefined
    T& operator[](Index_t index) noexcept
    {
        return m_values[static_cast<Capacity_t>(index)].value;
    }

    T* get(Index_t index) noexcept
    {
        return (m_values[static_cast<Capacity_t>(index)].isValid) ? &m_values[static_cast<uint32_t>(index)].value
                                                                  : nullptr;
    }

    size_t size() noexcept
    {
        return m_size;
    }

  private:
    Index_t nextFree() noexcept
    {
        if (m_size >= capacity)
            return NOT_AN_ELEMENT; // container is full

        for (; m_values[static_cast<Capacity_t>(m_freeIndex)].isValid;
             m_freeIndex = (m_freeIndex + 1) % static_cast<Index_t>(capacity))
            ;

        return m_freeIndex;
    }

    void setValid(Index_t index, bool value = true) noexcept
    {
        m_values[static_cast<Capacity_t>(index)].isValid = value;
    }

    void setInvalid(Index_t index) noexcept
    {
        setValid(index, false);
    }

    Index_t m_freeIndex{0};
    size_t m_size{0U};

    struct entry_t
    {
        T value = T();
        bool isValid = false;
    };

    iox::vector<entry_t, capacity> m_values;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_INTROSPECTION_FIXED_SIZE_CONTAINER_HPP
