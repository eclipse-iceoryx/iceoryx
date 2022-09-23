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
#ifndef IOX_HOOFS_CXX_VECTOR_INL
#define IOX_HOOFS_CXX_VECTOR_INL

#include "iceoryx_hoofs/cxx/vector.hpp"

#include <iostream>

namespace iox
{
namespace cxx
{
// NOLINTJUSTIFICATION See header and todo, using UninitializedArray will solve the issue
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(const uint64_t count, const T& value) noexcept
{
    if (count > Capacity)
    {
        std::cerr << "Attempting to initialize a vector of capacity " << Capacity << " with " << count
                  << " elements. This exceeds the capacity and only " << Capacity << " elements will be created!"
                  << std::endl;
    }

    for (uint64_t i = 0U; i < count && i < Capacity; ++i)
    {
        emplace_back(value);
    }
}

// NOLINTJUSTIFICATION Not all elements in the array shall be initialized
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(const uint64_t count) noexcept
{
    if (count > Capacity)
    {
        std::cerr << "Attempting to initialize a vector of capacity " << Capacity << " with " << count
                  << " elements. This exceeds the capacity and only " << Capacity << " elements will be created!"
                  << std::endl;
    }

    m_size = std::min(count, Capacity);
    for (uint64_t i = 0U; i < m_size; ++i)
    {
        new (&at(i)) T();
    }
}

// NOLINTJUSTIFICATION Not all elements in the array shall be initialized
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(const vector& rhs) noexcept
{
    *this = rhs;
}

// NOLINTJUSTIFICATION Not all elements in the array shall be initialized
// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::vector(vector&& rhs) noexcept
{
    *this = std::move(rhs);
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>::~vector() noexcept
{
    clear();
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>& vector<T, Capacity>::operator=(const vector& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0U;
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
        clearFrom(i);

        m_size = rhs.m_size;
    }
    return *this;
}

template <typename T, uint64_t Capacity>
inline vector<T, Capacity>& vector<T, Capacity>::operator=(vector&& rhs) noexcept
{
    if (this != &rhs)
    {
        uint64_t i = 0U;
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
        clearFrom(i);

        m_size = rhs.m_size;
        rhs.clear();
    }
    return *this;
}

template <typename T, uint64_t Capacity>
inline bool vector<T, Capacity>::empty() const noexcept
{
    return m_size == 0U;
}

template <typename T, uint64_t Capacity>
inline uint64_t vector<T, Capacity>::size() const noexcept
{
    return m_size;
}

template <typename T, uint64_t Capacity>
inline uint64_t vector<T, Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <typename T, uint64_t Capacity>
inline void vector<T, Capacity>::clear() noexcept
{
    clearFrom(0);
}

template <typename T, uint64_t Capacity>
template <typename... Targs>
inline bool vector<T, Capacity>::emplace_back(Targs&&... args) noexcept
{
    if (m_size < Capacity)
    {
        new (&at(m_size++)) T(std::forward<Targs>(args)...);
        return true;
    }
    return false;
}

template <typename T, uint64_t Capacity>
template <typename... Targs>
inline bool vector<T, Capacity>::emplace(const uint64_t position, Targs&&... args) noexcept
{
    if (m_size >= Capacity || position >= Capacity || position > m_size)
    {
        return false;
    }

    if (position == m_size)
    {
        return emplace_back(std::forward<Targs>(args)...);
    }
    emplace_back(std::move(at_unchecked(m_size - 1U)));
    for (uint64_t i = m_size - 1U; i > position; --i)
    {
        at_unchecked(i) = std::move(at_unchecked(i - 1U));
    }

    at(position).~T();
    new (&at(position)) T(std::forward<Targs>(args)...);
    return true;
}

template <typename T, uint64_t Capacity>
inline bool vector<T, Capacity>::push_back(const T& value) noexcept
{
    return emplace_back(value);
}

template <typename T, uint64_t Capacity>
inline bool vector<T, Capacity>::push_back(T&& value) noexcept
{
    return emplace_back(std::forward<T>(value));
}

template <typename T, uint64_t Capacity>
inline bool vector<T, Capacity>::pop_back() noexcept
{
    if (m_size > 0U)
    {
        at_unchecked(--m_size).~T();
        return true;
    }
    return false;
}

template <typename T, uint64_t Capacity>
template <typename... Targs>
inline bool vector<T, Capacity>::resize(const uint64_t count, const Targs&... args) noexcept
{
    if (count > Capacity)
    {
        return false;
    }

    if (count < m_size)
    {
        clearFrom(count);
    }
    else if (count > m_size)
    {
        while (count != m_size)
        {
            emplace_back(args...);
        }
    }
    return true;
}

template <typename T, uint64_t Capacity>
inline T* vector<T, Capacity>::data() noexcept
{
    // AXIVION Next Line AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<T*>(const_cast<const vector<T, Capacity>*>(this)->data());
}

template <typename T, uint64_t Capacity>
inline const T* vector<T, Capacity>::data() const noexcept
{
    return &at_unchecked(0);
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::at(const uint64_t index) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<T&>(const_cast<const vector<T, Capacity>*>(this)->at(index));
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::at(const uint64_t index) const noexcept
{
    cxx::Expects((index < m_size) && "Out of bounds access");
    return at_unchecked(index);
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::operator[](const uint64_t index) noexcept
{
    return at(index);
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::operator[](const uint64_t index) const noexcept
{
    return at(index);
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::front() noexcept
{
    cxx::Expects(!empty() && "Attempting to access the front of an empty vector");
    return at(0);
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::front() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<vector<T, Capacity>*>(this)->front();
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::back() noexcept
{
    cxx::Expects(!empty() && "Attempting to access the back of an empty vector");
    return at(size() - 1U);
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::back() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<vector<T, Capacity>*>(this)->back();
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::iterator vector<T, Capacity>::begin() noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<iterator>(const_cast<const vector<T, Capacity>*>(this)->begin());
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::const_iterator vector<T, Capacity>::begin() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type-safety ensured by template parameter
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const_iterator>(&at_unchecked(0));
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::iterator vector<T, Capacity>::end() noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<iterator>(const_cast<const vector<T, Capacity>*>(this)->end());
}

template <typename T, uint64_t Capacity>
inline typename vector<T, Capacity>::const_iterator vector<T, Capacity>::end() const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type-safety ensured by template parameter
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return reinterpret_cast<const_iterator>(&(at_unchecked(0)) + m_size);
}

template <typename T, uint64_t Capacity>
inline bool vector<T, Capacity>::erase(iterator position) noexcept
{
    if (begin() <= position && position < end())
    {
        uint64_t index = static_cast<uint64_t>(position - begin()) % (sizeof(element_t) * Capacity);
        size_t n = index;
        for (; n + 1U < size(); ++n)
        {
            at(n) = std::move(at(n + 1U));
        }
        at(n).~T();
        m_size--;
        return true;
    }
    return false;
}

template <typename T, uint64_t Capacity>
inline T& vector<T, Capacity>::at_unchecked(const uint64_t index) noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.3 : const cast to avoid code duplication
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<T&>(const_cast<const vector<T, Capacity>*>(this)->at_unchecked(index));
}

template <typename T, uint64_t Capacity>
inline const T& vector<T, Capacity>::at_unchecked(const uint64_t index) const noexcept
{
    // AXIVION Next Construct AutosarC++19_03-A5.2.4 : Type-safety ensured by template parameter
    // NOLINTJUSTIFICATION User accessible method at() performs bounds check
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast,cppcoreguidelines-pro-bounds-pointer-arithmetic)
    return reinterpret_cast<const T*>(m_data)[index];
}

template <typename T, uint64_t Capacity>
inline void vector<T, Capacity>::clearFrom(const uint64_t startPosition) noexcept
{
    while (m_size > startPosition)
    {
        at_unchecked(--m_size).~T();
    }
}

} // namespace cxx
} // namespace iox

template <typename T, uint64_t CapacityLeft, uint64_t CapacityRight>
inline bool operator==(const iox::cxx::vector<T, CapacityLeft>& lhs,
                       const iox::cxx::vector<T, CapacityRight>& rhs) noexcept
{
    uint64_t vectorSize = lhs.size();
    if (vectorSize != rhs.size())
    {
        return false;
    }

    for (uint64_t i = 0U; i < vectorSize; ++i)
    {
        if (lhs[i] != rhs[i])
        {
            return false;
        }
    }
    return true;
}

template <typename T, uint64_t CapacityLeft, uint64_t CapacityRight>
inline bool operator!=(const iox::cxx::vector<T, CapacityLeft>& lhs,
                       const iox::cxx::vector<T, CapacityRight>& rhs) noexcept
{
    return !(lhs == rhs);
}


#endif // IOX_HOOFS_CXX_VECTOR_INL
