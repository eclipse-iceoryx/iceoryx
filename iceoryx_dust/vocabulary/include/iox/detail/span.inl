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
#ifndef IOX_DUST_VOCABULARY_SPAN_INL
#define IOX_DUST_VOCABULARY_SPAN_INL

#include "iox/span.hpp"

namespace iox
{
template <typename Container>
inline constexpr auto data(Container& container) -> decltype(container.data())
{
    return container.data();
}

template <typename Container>
inline constexpr auto data(const Container& container) -> decltype(container.data())
{
    return container.data();
}

template <typename T, uint64_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
inline constexpr T* data(T (&array)[N]) noexcept
{
    return array;
}

template <typename T, uint64_t N, template <typename, uint64_t> class Buffer>
inline constexpr T* data(UninitializedArray<T, N, Buffer>& uninit_array) noexcept
{
    return uninit_array.begin();
}

template <typename T, uint64_t Extent>
template <typename It>
inline constexpr span<T, Extent>::span(It first, uint64_t count) noexcept
    : span_storage_t(count)
    , m_data(detail::to_address(first))
{
    assert(Extent == DYNAMIC_EXTENT || Extent == count);
}

template <typename T, uint64_t Extent>
template <typename It, typename End, typename>
inline constexpr span<T, Extent>::span(It begin, End end) noexcept
    : span(begin, static_cast<uint64_t>(end - begin))
{
    assert(begin == nullptr || end == nullptr || begin <= end);
}

template <typename T, uint64_t Extent>
template <uint64_t N, typename>
inline constexpr span<T, Extent>::span(
    T (&array)[N]) noexcept // NOLINT(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    : span(iox::data(array), N)
{
}

template <typename T, uint64_t Extent>
template <typename U, uint64_t N, template <typename, uint64_t> class Buffer, typename>
inline constexpr span<T, Extent>::span(iox::UninitializedArray<U, N, Buffer>& array) noexcept
    : span(iox::data(array), N)
{
}

template <typename T, uint64_t Extent>
template <typename U, uint64_t N, template <typename, uint64_t> class Buffer, typename>
inline constexpr span<T, Extent>::span(const iox::UninitializedArray<U, N, Buffer>& array) noexcept
    : span(iox::data(array), N)
{
}

template <typename T, uint64_t Extent>
template <typename Container, typename>
inline constexpr span<T, Extent>::span(Container& container) noexcept
    : span(iox::data(container), iox::size(container))
{
}

template <typename T, uint64_t Extent>
template <typename Container, typename>
inline constexpr span<T, Extent>::span(const Container& container) noexcept
    : span(iox::data(container), iox::size(container))
{
}

template <typename T, uint64_t Extent>
template <typename U, uint64_t OtherExtent, typename>
inline constexpr span<T, Extent>::span(const span<U, OtherExtent>& other)
    : span(other.data(), other.size())
{
}

template <typename T, uint64_t Extent>
inline constexpr span<T, Extent>::span(span&& other) noexcept
    : span_storage_t(std::move(other))
{
    *this = std::move(other);
}

template <typename T, uint64_t Extent>
inline constexpr span<T, Extent>& span<T, Extent>::operator=(span&& other) noexcept
{
    if (this != &other)
    {
        span_storage_t::operator=(std::move(other));
        m_data = other.m_data;
        other.m_data = nullptr;
    }
    return *this;
}

template <typename T, uint64_t Extent>
template <uint64_t Count>
inline constexpr span<T, Count> span<T, Extent>::first() const noexcept
{
    static_assert(Count <= Extent, "Count must not exceed Extent");
    assert(Extent != DYNAMIC_EXTENT || Count <= size());
    return {data(), Count};
}

template <typename T, uint64_t Extent>
inline constexpr span<T, DYNAMIC_EXTENT> span<T, Extent>::first(uint64_t count) const noexcept
{
    assert(count <= size());
    return {data(), count};
}

template <typename T, uint64_t Extent>
template <uint64_t Count>
inline constexpr span<T, Count> span<T, Extent>::last() const noexcept
{
    static_assert(Count <= Extent, "Count must not exceed Extent");
    assert(Extent != DYNAMIC_EXTENT || Count <= size());
    return {data() + (size() - Count), Count};
}

template <typename T, uint64_t Extent>
inline constexpr span<T, DYNAMIC_EXTENT> span<T, Extent>::last(uint64_t count) const noexcept
{
    assert(count <= size());
    return {data() + (size() - count), count};
}

template <typename T, uint64_t Extent>
template <uint64_t Offset, uint64_t Count>
inline constexpr span<T,
                      (Count != DYNAMIC_EXTENT ? Count : (Extent != DYNAMIC_EXTENT ? Extent - Offset : DYNAMIC_EXTENT))>
span<T, Extent>::subspan() const noexcept
{
    static_assert(Offset <= Extent, "Offset must not exceed Extent");
    static_assert(Count == DYNAMIC_EXTENT || Count <= Extent - Offset, "Count must not exceed Extent - Offset");
    assert(Extent != DYNAMIC_EXTENT || Offset <= size());
    assert(Extent != DYNAMIC_EXTENT || Count == DYNAMIC_EXTENT || Count <= size() - Offset);
    return {data() + Offset, Count != DYNAMIC_EXTENT ? Count : size() - Offset};
}

template <typename T, uint64_t Extent>
inline constexpr span<T, DYNAMIC_EXTENT> span<T, Extent>::subspan(uint64_t offset, uint64_t count) const noexcept
{
    assert(offset <= size());
    assert(count == DYNAMIC_EXTENT || count <= size() - offset);
    return {data() + offset, count != DYNAMIC_EXTENT ? count : size() - offset};
}

template <typename T, uint64_t Extent>
inline constexpr uint64_t span<T, Extent>::size() const noexcept
{
    return span_storage_t::size();
}

template <typename T, uint64_t Extent>
inline constexpr uint64_t span<T, Extent>::size_bytes() const noexcept
{
    return size() * sizeof(T);
}

template <typename T, uint64_t Extent>
inline constexpr bool span<T, Extent>::empty() const noexcept
{
    return size() == 0;
}

template <typename T, uint64_t Extent>
inline constexpr T& span<T, Extent>::front() const noexcept
{
    static_assert(Extent == DYNAMIC_EXTENT || Extent > 0, "Extent must not be 0");
    assert(Extent != DYNAMIC_EXTENT || !empty());
    return *data();
}

template <typename T, uint64_t Extent>
inline constexpr T& span<T, Extent>::back() const noexcept
{
    static_assert(Extent == DYNAMIC_EXTENT || Extent > 0, "Extent must not be 0");
    assert(Extent != DYNAMIC_EXTENT || !empty());
    return *(data() + size() - 1);
}

template <typename T, uint64_t Extent>
inline constexpr T& span<T, Extent>::operator[](uint64_t index) const noexcept
{
    assert(index < size());
    return *(data() + index);
}

template <typename T, uint64_t Extent>
inline constexpr T* span<T, Extent>::data() const noexcept
{
    return m_data;
}

template <typename T, uint64_t Extent>
inline constexpr iox::span_iterator<T> span<T, Extent>::begin() const noexcept
{
    return iterator(m_data, m_data + size());
}

template <typename T, uint64_t Extent>
inline constexpr iox::span_iterator<T> span<T, Extent>::end() const noexcept
{
    return iterator(m_data, m_data + size(), m_data + size());
}

template <typename T, uint64_t Extent>
inline constexpr std::reverse_iterator<iox::span_iterator<T>> span<T, Extent>::rbegin() const noexcept
{
    return reverse_iterator{end()};
}

template <typename T, uint64_t Extent>
inline constexpr std::reverse_iterator<iox::span_iterator<T>> span<T, Extent>::rend() const noexcept
{
    return reverse_iterator{begin()};
}

template <typename T, uint64_t Extent>
constexpr uint64_t span<T, Extent>::extent;

// object representation
template <typename T, uint64_t X>
inline span<const uint8_t, (X == DYNAMIC_EXTENT ? DYNAMIC_EXTENT : sizeof(T) * X)> as_bytes(span<T, X> s) noexcept
{
    static_assert(X == DYNAMIC_EXTENT || sizeof(T) < std::numeric_limits<uint64_t>::max() / X,
                  "Potential overflow when calculating the size of as_bytes");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uint8_t*>(s.data()), s.size_bytes()};
}

template <typename T, uint64_t X, typename>
inline span<uint8_t, (X == DYNAMIC_EXTENT ? DYNAMIC_EXTENT : sizeof(T) * X)> as_writable_bytes(span<T, X> s) noexcept
{
    static_assert(X == DYNAMIC_EXTENT || sizeof(T) < std::numeric_limits<uint64_t>::max() / X,
                  "Potential overflow when calculating the size of as_writable_bytes");
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uint8_t*>(s.data()), s.size_bytes()};
}
} // namespace iox

#endif // IOX_DUST_VOCABULARY_SPAN_INL
