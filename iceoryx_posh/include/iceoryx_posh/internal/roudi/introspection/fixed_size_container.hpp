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

#pragma once

#include "iceoryx_utils/cxx/vector.hpp"

#include <cstdint>

// Use: allows allocating a predefined capacity of T objects on the stack
// and using there pointers outside
//(pointers stay valid until object is removed from theFixedSizeContainer)
// therefore we avoid heap allocation but we can still use pointers to objects
// for efficient update/passing until we exhaust the fixed size container

// note: no bounds checking during access for efficiency (as in STL containers)
// access to indices < 0 or >= capacity is undefined behaviour (most likely a
// segmentation fault)

template <typename T, uint32_t capacity = 1>
class FixedSizeContainer
{
  public:
    using Index_t = int32_t;
    static constexpr int32_t NOT_AN_ELEMENT = -1;

    FixedSizeContainer()
        : m_values(capacity)
    {
    }

    // returns index or -1 if element could not be added
    // a successful add returns an arbitrary index which can be non consecutive
    // for consecutive adds
    Index_t add(const T& element)
    {
        auto nextElement = nextFree();

        if (nextElement >= 0)
        {
            m_freeIndex = nextElement;
            m_values[m_freeIndex].value = element;
            m_values[m_freeIndex].isValid = true;
            ++m_size;
        }

        return nextElement;
    }

    void remove(Index_t index)
    {
        if (m_values[index].isValid)
        {
            m_values[index].isValid = false;
            --m_size;
        }
    }

    // note: access can change the underlying object, without modifying valid
    // flag
    // if the index is invalid than the behavior is undefined
    T& operator[](Index_t index)
    {
        return m_values[index].value;
    }

    T* get(Index_t index)
    {
        return (m_values[index].isValid) ? &m_values[index].value : nullptr;
    }

    size_t size()
    {
        return m_size;
    }

  private:
    Index_t nextFree()
    {
        if (m_size >= capacity)
            return NOT_AN_ELEMENT; // container is full

        for (; m_values[m_freeIndex].isValid; m_freeIndex = (m_freeIndex + 1) % capacity)
            ;

        return m_freeIndex;
    }

    void setValid(Index_t index, bool value = true)
    {
        m_values[index].isValid = value;
    }

    void setInvalid(Index_t index)
    {
        setValid(index, false);
    }

    Index_t m_freeIndex{0};
    size_t m_size{0};

    struct entry_t
    {
        T value = T();
        bool isValid = false;
    };

    iox::cxx::vector<entry_t, capacity> m_values;
};
