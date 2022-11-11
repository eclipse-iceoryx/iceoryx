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
#ifndef IOX_HOOFS_CXX_SPAN_INL
#define IOX_HOOFS_CXX_SPAN_INL

#include "iceoryx_hoofs/cxx/span.hpp"

namespace iox
{
template <typename T, uint64_t Extent>
inline constexpr span<T, Extent>::span() noexcept
    : span_storage_t(0)
    , m_data(nullptr)
{
    static_assert(Extent == dynamic_extent || Extent == 0, "Invalid Extent");
}

template <typename T, uint64_t Extent>
template <typename It>
inline constexpr span<T, Extent>::span(It first, uint64_t count) noexcept
    : span_storage_t(count)
    , m_data(internal::to_address(first))
{
    iox::ConstexprCheckTrue(Extent == dynamic_extent || Extent == count);
}

template <typename T, uint64_t Extent>
template <typename It, typename End, typename>
inline constexpr span<T, Extent>::span(It begin, End end) noexcept
    : span(begin, static_cast<uint64_t>(end - begin))
{
    // check for non negative result
    iox::ConstexprCheckTrue(begin <= end, "");
}

template <typename T, uint64_t Extent>
template <uint64_t N, typename>
inline constexpr span<T, Extent>::span(
    T (&array)[N]) noexcept // NOLINT(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    : span(iox::data(array), N)
{
}

template <typename T, uint64_t Extent>
template <typename U, uint64_t N, typename>
inline constexpr span<T, Extent>::span(std::array<U, N>& array) noexcept
    : span(iox::data(array), N)
{
}

template <typename T, uint64_t Extent>
template <typename U, uint64_t N, typename>
inline constexpr span<T, Extent>::span(const std::array<U, N>& array) noexcept
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
template <uint64_t Count>
inline constexpr span<T, Count> span<T, Extent>::first() const noexcept
{
    static_assert(Count <= Extent, "Count must not exceed Extent");
    iox::ConstexprCheckTrue(Extent != dynamic_extent || Count <= size());
    return {data(), Count};
}

template <typename T, uint64_t Extent>
inline constexpr span<T, dynamic_extent> span<T, Extent>::first(uint64_t count) const noexcept
{
    iox::ConstexprCheckTrue(count <= size());
    return {data(), count};
}

template <typename T, uint64_t Extent>
template <uint64_t Count>
inline constexpr span<T, Count> span<T, Extent>::last() const noexcept
{
    static_assert(Count <= Extent, "Count must not exceed Extent");
    iox::ConstexprCheckTrue(Extent != dynamic_extent || Count <= size());
    return {data() + (size() - Count), Count};
}

template <typename T, uint64_t Extent>
inline constexpr span<T, dynamic_extent> span<T, Extent>::last(uint64_t count) const noexcept
{
    iox::ConstexprCheckTrue(count <= size());
    return {data() + (size() - count), count};
}

template <typename T, uint64_t Extent>
template <uint64_t Offset, uint64_t Count>
inline constexpr span<T,
                      (Count != dynamic_extent ? Count : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent))>
span<T, Extent>::subspan() const noexcept
{
    static_assert(Offset <= Extent, "Offset must not exceed Extent");
    static_assert(Count == dynamic_extent || Count <= Extent - Offset, "Count must not exceed Extent - Offset");
    iox::ConstexprCheckTrue(Extent != dynamic_extent || Offset <= size());
    iox::ConstexprCheckTrue(Extent != dynamic_extent || Count == dynamic_extent || Count <= size() - Offset);
    return {data() + Offset, Count != dynamic_extent ? Count : size() - Offset};
}

template <typename T, uint64_t Extent>
inline constexpr span<T, dynamic_extent> span<T, Extent>::subspan(uint64_t offset, uint64_t count) const noexcept
{
    iox::ConstexprCheckTrue(offset <= size());
    iox::ConstexprCheckTrue(count == dynamic_extent || count <= size() - offset);
    return {data() + offset, count != dynamic_extent ? count : size() - offset};
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
    static_assert(Extent == dynamic_extent || Extent > 0, "Extent must not be 0");
    iox::ConstexprCheckTrue(Extent != dynamic_extent || !empty());
    return *data();
}

template <typename T, uint64_t Extent>
inline constexpr T& span<T, Extent>::back() const noexcept
{
    static_assert(Extent == dynamic_extent || Extent > 0, "Extent must not be 0");
    iox::ConstexprCheckTrue(Extent != dynamic_extent || !empty());
    return *(data() + size() - 1);
}

template <typename T, uint64_t Extent>
inline constexpr T& span<T, Extent>::operator[](uint64_t idx) const noexcept
{
    iox::ConstexprCheckTrue(idx < size());
    return *(data() + idx);
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

template <class T, uint64_t Extent>
constexpr uint64_t span<T, Extent>::extent;

// object representation
template <typename T, uint64_t X>
span<const uint8_t, (X == dynamic_extent ? dynamic_extent : sizeof(T) * X)> as_bytes(span<T, X> s) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<const uint8_t*>(s.data()), s.size_bytes()};
}

template <typename T, uint64_t X, typename = std::enable_if_t<!std::is_const<T>::value>>
span<uint8_t, (X == dynamic_extent ? dynamic_extent : sizeof(T) * X)> as_writable_bytes(span<T, X> s) noexcept
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    return {reinterpret_cast<uint8_t*>(s.data()), s.size_bytes()};
}

// helpers for creation of a span.
template <int&... ArgBarrier, typename It>
constexpr auto make_span(It it, uint64_t size) noexcept
{
    using RemoveRef_t = std::remove_reference_t<iox::iter_reference_t<It>>;
    return span<RemoveRef_t>(it, size);
}

template <int&... ArgBarrier,
          typename It,
          typename End,
          typename = std::enable_if_t<!iox::is_convertible_v<End, uint64_t>>>
constexpr auto make_span(It it, End end) noexcept
{
    using RemoveRef_t = std::remove_reference_t<iox::iter_reference_t<It>>;
    return span<RemoveRef_t>(it, end);
}

template <int&... ArgBarrier, typename Container>
constexpr auto make_span(Container&& container) noexcept
{
    using RemovePtr_t = std::remove_pointer_t<decltype(data(std::declval<Container>()))>;
    using Extent = internal::extent_t<Container>;
    return span<RemovePtr_t, Extent::value>(std::forward<Container>(container));
}

template <uint64_t N, int&... ArgBarrier, typename It>
constexpr auto make_span(It it, uint64_t size) noexcept
{
    using RemoveRef_t = std::remove_reference_t<iox::iter_reference_t<It>>;
    return span<RemoveRef_t, N>(it, size);
}

template <uint64_t N,
          int&... ArgBarrier,
          typename It,
          typename End,
          typename = std::enable_if_t<!iox::is_convertible_v<End, uint64_t>>>
constexpr auto make_span(It it, End end) noexcept
{
    using RemoveRef_t = std::remove_reference_t<iox::iter_reference_t<It>>;
    return span<RemoveRef_t, N>(it, end);
}

template <uint64_t N, int&... ArgBarrier, typename Container>
constexpr auto make_span(Container&& container) noexcept
{
    using RemovePtr_t = std::remove_pointer_t<decltype(data(std::declval<Container>()))>;
    return span<RemovePtr_t, N>(data(container), size(container));
}

} // namespace iox

#endif // IOX_HOOFS_CXX_SPAN_INL