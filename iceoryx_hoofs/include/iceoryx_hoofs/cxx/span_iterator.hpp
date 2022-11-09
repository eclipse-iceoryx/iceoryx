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
#ifndef IOX_HOOFS_CXX_SPAN_ITERATOR_HPP
#define IOX_HOOFS_CXX_SPAN_ITERATOR_HPP

#include <iterator> // for reverse_iterator, distance, random_access_...

#include "iceoryx_hoofs/cxx/requires.hpp"

namespace iox
{
namespace cxx
{


inline constexpr bool ConstexprCheckTrue(bool condition)
{
    return condition || false;
}

template <typename T>
class span_iterator
{
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    constexpr span_iterator() = default;


    constexpr span_iterator(pointer start, const pointer end)
        : span_iterator(start, end, start)
    {
    }

    constexpr span_iterator(const pointer begin, const pointer end, pointer current)
        : m_begin(begin)
        , m_end(end)
        , m_current(current)
    {
    }

    constexpr span_iterator(const span_iterator& other) = default;

    constexpr operator span_iterator<const T>() const noexcept
    {
        return {m_begin, m_end, m_current};
    }

    constexpr reference operator*() const noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin && m_end);
        iox::cxx::ConstexprCheckTrue(m_begin <= m_current && m_current < m_end);
        return *m_current;
    }

    constexpr pointer operator->() const noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin && m_end);
        iox::cxx::ConstexprCheckTrue(m_begin <= m_current && m_current < m_end);
        return m_current;
    }
    constexpr span_iterator& operator++() noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin && m_current && m_end);
        iox::cxx::ConstexprCheckTrue(m_current < m_end);

        ++m_current;
        return *this;
    }

    constexpr span_iterator operator++(int) noexcept
    {
        span_iterator ret = *this;
        ++*this;
        return ret;
    }

    constexpr span_iterator& operator--() noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin && m_end);
        iox::cxx::ConstexprCheckTrue(m_begin < m_current);
        --m_current;
        return *this;
    }

    constexpr span_iterator operator--(int) noexcept
    {
        span_iterator ret = *this;
        --*this;
        return ret;
    }

    constexpr span_iterator& operator+=(const difference_type n) noexcept
    {
        if (n != 0)
            iox::cxx::ConstexprCheckTrue(m_begin && m_current && m_end);
        if (n > 0)
            iox::cxx::ConstexprCheckTrue(m_end - m_current >= n);
        if (n < 0)
            iox::cxx::ConstexprCheckTrue(m_current - m_begin >= -n);

        m_current += n;
        return *this;
    }

    constexpr span_iterator operator+(const difference_type n) const noexcept
    {
        span_iterator ret = *this;
        ret += n;
        return ret;
    }

    friend constexpr span_iterator operator+(const difference_type n, const span_iterator& rhs) noexcept
    {
        return rhs + n;
    }

    constexpr span_iterator& operator-=(const difference_type n) noexcept
    {
        if (n != 0)
            iox::cxx::ConstexprCheckTrue(m_begin && m_current && m_end);
        if (n > 0)
            iox::cxx::ConstexprCheckTrue(m_current - m_begin >= n);
        if (n < 0)
            iox::cxx::ConstexprCheckTrue(m_end - m_current >= -n);
        m_current -= n;
        return *this;
    }

    constexpr span_iterator operator-(const difference_type n) const noexcept
    {
        span_iterator ret = *this;
        ret -= n;
        return ret;
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr difference_type operator-(const span_iterator<Type2>& rhs) const noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current - rhs.m_current;
    }

    constexpr reference operator[](const difference_type n) const noexcept
    {
        return *(*this + n);
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr bool operator==(const span_iterator<Type2>& rhs) const noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current == rhs.m_current;
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr bool operator!=(const span_iterator<Type2>& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr bool operator<(const span_iterator<Type2>& rhs) const noexcept
    {
        iox::cxx::ConstexprCheckTrue(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current < rhs.m_current;
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr bool operator>(const span_iterator<Type2>& rhs) const noexcept
    {
        return rhs < *this;
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr bool operator<=(const span_iterator<Type2>& rhs) const noexcept
    {
        return !(rhs < *this);
    }

    template <class Type2, std::enable_if_t<std::is_same<std::remove_cv_t<Type2>, value_type>::value, int> = 0>
    constexpr bool operator>=(const span_iterator<Type2>& rhs) const noexcept
    {
        return !(*this < rhs);
    }

    const pointer m_begin = nullptr;
    const pointer m_end = nullptr;
    pointer m_current = nullptr;

    template <typename Ptr>
    friend struct std::pointer_traits;
};
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_SPAN_ITERATOR_HPP