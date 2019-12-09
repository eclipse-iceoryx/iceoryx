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

#include "iceoryx_utils/cxx/vector.hpp"

#include <iostream>

namespace iox
{
namespace cxx
{
template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(const uint64_t count, const T& value)
{
    if (count > Capacity)
    {
        std::cerr << "Attemting to initialize a vector with more elements than its capacity!" << std::endl;
    }
    for (uint64_t i = 0; i < count && i < Capacity; ++i)
    {
        emplace_back(value);
    }
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(const uint64_t count)
    : vector(count, T())
{
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(const vector& rhs)
{
    *this = rhs;
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(vector&& rhs)
{
    *this = std::move(rhs);
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::~vector()
{
    clear();
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>& vector<T, Capacity>::operator=(const vector& rhs)
{
    if (this != &rhs)
    {
        uint64_t i = 0;
        // copy using copy assignment
        for (; i < std::min(rhs.size(), size()); ++i)
        {
            at(i) = rhs.at(i);
        }

        // copy using copy ctor
        for (; i < rhs.size(); ++i)
        {
            emplace_back(rhs.at(i));
        }

        // delete remaining elements
        for (; i < size(); ++i)
        {
            at(i).~T();
        }

        m_size = rhs.m_size;
    }
    return *this;
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>& vector<T, Capacity>::operator=(vector&& rhs)
{
    if (this != &rhs)
    {
        uint64_t i = 0;
        // move using move assignment
        for (; i < std::min(rhs.size(), size()); ++i)
        {
            at(i) = std::move(rhs.at(i));
        }

        // move using move ctor
        for (; i < rhs.size(); ++i)
        {
            emplace_back(std::move(rhs.at(i)));
        }

        // delete remaining elements
        for (; i < size(); ++i)
        {
            at(i).~T();
        }

        m_size = rhs.m_size;
        rhs.clear();
    }
    return *this;
}

template <typename T, uint64_t Capacity>
inline bool vector<T, Capacity>::empty() const
{
    return m_size == 0;
}

template <typename T, uint64_t Capacity>
inline uint64_t vector<T, Capacity>::size() const
{
    return m_size;
}

template <typename T, uint64_t Capacity>
inline uint64_t vector<T, Capacity>::capacity() const
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
inline void vector<T, Capacity>::clear()
{
    for (uint64_t i = 0; i < m_size; ++i)
    {
        at(i).~T();
    }
    m_size = 0;
}

template <typename T, uint64_t Capacity>
template <typename... Targs>
inline bool vector<T, Capacity>::emplace_back(Targs&&... args)
{
    if (m_size < Capacity)
    {
        new (&at(m_size++)) T(std::forward<Targs>(args)...);
        return true;
    }
    return false;
}

template <typename T, uint64_t Capacity>
bool vector<T, Capacity>::push_back(const T& value)
{
    return emplace_back(value);
}

template <typename T, uint64_t Capacity>
bool vector<T, Capacity>::push_back(T&& value)
{
    return emplace_back(std::forward<T>(value));
}

template <typename T, uint64_t Capacity>
void vector<T, Capacity>::pop_back()
{
    if (m_size > 0)
    {
        back().~T();
        m_size--;
    }
}

template <typename T, uint64_t Capacity>
inline T* vector<T, Capacity>::data() noexcept
{
    return reinterpret_cast<T*>(m_data);
}

template <typename T, uint64_t Capacity>
inline const T* vector<T, Capacity>::data() const noexcept
{
    return reinterpret_cast<const T*>(m_data);
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::at(const uint64_t index)
{
    return const_cast<T&>(const_cast<const vector<T, Capacity>*>(this)->at(index));
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::at(const uint64_t index) const
{
    if (index + 1 > m_size)
    {
        fprintf(stderr, "out of bounds access, current size is %lu but given index is %lu\n", m_size, index);
        std::terminate();
    }
    return reinterpret_cast<const T*>(m_data)[index];
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::operator[](const uint64_t index)
{
    return at(index);
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::operator[](const uint64_t index) const
{
    return at(index);
}

template <typename T, uint64_t Capacity>
T& vector<T, Capacity>::front() noexcept
{
    if (empty())
    {
        std::cerr << "Attempting to access the front of an empty vector!" << std::endl;
        std::terminate();
    }
    return at(0);
}

template <typename T, uint64_t Capacity>
const T& vector<T, Capacity>::front() const noexcept
{
    if (empty())
    {
        std::cerr << "Attempting to access the front of an empty vector!" << std::endl;
        std::terminate();
    }
    return at(0);
}

template <typename T, uint64_t Capacity>
T& vector<T, Capacity>::back() noexcept
{
    if (empty())
    {
        std::cerr << "Attempting to access the back of an empty vector!" << std::endl;
        std::terminate();
    }
    return at(size() - 1);
}

template <typename T, uint64_t Capacity>
const T& vector<T, Capacity>::back() const noexcept
{
    if (empty())
    {
        std::cerr << "Attempting to access the back of an empty vector!" << std::endl;
        std::terminate();
    }
    return at(size() - 1);
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::iterator vector<T, Capacity>::begin()
{
    return reinterpret_cast<iterator>(&(m_data[0]));
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::const_iterator vector<T, Capacity>::begin() const
{
    return reinterpret_cast<const_iterator>(&(m_data[0]));
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::iterator vector<T, Capacity>::end()
{
    return reinterpret_cast<iterator>((&(m_data[0]) + m_size)[0]);
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::const_iterator vector<T, Capacity>::end() const
{
    return reinterpret_cast<const_iterator>((&(m_data[0]) + m_size)[0]);
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::iterator vector<T, Capacity>::erase(iterator position)
{
    if (begin() <= position && position < end())
    {
        uint64_t index = static_cast<uint64_t>(position - begin()) % (sizeof(element_t) * Capacity);
        size_t n = index;
        for (; n + 1 < size(); ++n)
        {
            at(n) = std::move(at(n + 1));
        }
        at(n).~T();
        m_size--;
    }
    return nullptr;
}

} // namespace cxx
} // namespace iox
