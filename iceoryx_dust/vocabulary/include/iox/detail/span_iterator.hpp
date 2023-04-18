// Copyright (c) 2022 - 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_DUST_VOCABULARY_SPAN_ITERATOR_HPP
#define IOX_DUST_VOCABULARY_SPAN_ITERATOR_HPP

// Use 'assert's as 'iox::Expects' is not useable inside 'constexpr' functions
#include <cassert>
#include <iterator> // for reverse_iterator, distance, random_access_...

namespace iox
{

template <typename T>
class span_iterator final
{
  public:
    using iterator_category = std::random_access_iterator_tag;
    using value_type = std::remove_cv_t<T>;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    constexpr span_iterator(const span_iterator& other) = default;
    constexpr span_iterator(span_iterator&& other) noexcept = default;
    constexpr span_iterator& operator=(const span_iterator& other) = default;
    constexpr span_iterator& operator=(span_iterator&& other) noexcept = default;
    ~span_iterator() = default;

    constexpr span_iterator(pointer start, const pointer end) noexcept
        : span_iterator(start, end, start)
    {
    }

    constexpr span_iterator(const pointer begin, const pointer end, pointer current) noexcept
        : m_begin(begin)
        , m_end(end)
        , m_current(current)
    {
        assert(m_begin <= m_end);
        assert(m_begin <= m_current && m_current < m_end);
    }

    explicit constexpr operator span_iterator<const T>() const noexcept
    {
        return {m_begin, m_end, m_current};
    }

    constexpr reference operator*() const noexcept
    {
        assert(m_begin && m_end);
        assert(m_begin <= m_current && m_current < m_end);
        return *m_current;
    }

    constexpr pointer operator->() const noexcept
    {
        assert(m_begin && m_end);
        assert(m_begin <= m_current && m_current < m_end);
        return m_current;
    }
    constexpr span_iterator& operator++() noexcept
    {
        assert(m_begin && m_current && m_end);
        assert(m_current < m_end);

        ++m_current;
        return *this;
    }

    // Rule DCL21-CPP is deprecated
    // NOLINTNEXTLINE(cert-dcl21-cpp)
    constexpr span_iterator operator++(int) noexcept
    {
        span_iterator ret = *this;
        ++*this;
        return ret;
    }

    constexpr span_iterator& operator--() noexcept
    {
        assert(m_begin && m_end);
        assert(m_begin < m_current);
        --m_current;
        return *this;
    }

    // Rule DCL21-CPP is deprecated
    // NOLINTNEXTLINE(cert-dcl21-cpp)
    constexpr span_iterator operator--(int) noexcept
    {
        span_iterator ret = *this;
        --*this;
        return ret;
    }

    constexpr span_iterator& operator+=(const difference_type n) noexcept
    {
        if (n == 0)
        {
            return *this;
        }
        assert(m_begin && m_current && m_end);
        if (n > 0)
        {
            assert(m_end - m_current >= n);
        }
        else
        {
            assert(m_current - m_begin >= -n);
        }

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
        if (n == 0)
        {
            return *this;
        }
        assert(m_begin && m_current && m_end);
        if (n > 0)
        {
            assert(m_current - m_begin >= n);
        }
        else
        {
            assert(m_end - m_current >= -n);
        }
        m_current -= n;
        return *this;
    }

    constexpr span_iterator operator-(const difference_type n) const noexcept
    {
        span_iterator ret = *this;
        ret -= n;
        return ret;
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr difference_type operator-(const span_iterator<OtherType>& rhs) const noexcept
    {
        assert(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current - rhs.m_current;
    }

    constexpr reference operator[](const difference_type n) const noexcept
    {
        return *(*this + n);
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr bool operator==(const span_iterator<OtherType>& rhs) const noexcept
    {
        assert(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current == rhs.m_current;
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr bool operator!=(const span_iterator<OtherType>& rhs) const noexcept
    {
        return !(*this == rhs);
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr bool operator<(const span_iterator<OtherType>& rhs) const noexcept
    {
        assert(m_begin == rhs.m_begin && m_end == rhs.m_end);
        return m_current < rhs.m_current;
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr bool operator>(const span_iterator<OtherType>& rhs) const noexcept
    {
        return rhs < *this;
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr bool operator<=(const span_iterator<OtherType>& rhs) const noexcept
    {
        return !(rhs < *this);
    }

    template <class OtherType, std::enable_if_t<std::is_same<std::remove_cv_t<OtherType>, value_type>::value, int> = 0>
    constexpr bool operator>=(const span_iterator<OtherType>& rhs) const noexcept
    {
        return !(*this < rhs);
    }

    const pointer m_begin{nullptr};
    const pointer m_end{nullptr};
    pointer m_current{nullptr};

    template <typename Ptr>
    friend struct std::pointer_traits;
};
} // namespace iox

#endif // IOX_DUST_VOCABULARY_SPAN_ITERATOR_HPP
