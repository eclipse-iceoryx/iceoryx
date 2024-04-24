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

#ifndef IOX_HOOFS_VOCABULARY_SPAN_HPP
#define IOX_HOOFS_VOCABULARY_SPAN_HPP

#include "iox/assertions.hpp"
#include "iox/detail/span_iterator.hpp"
#include "iox/detail/uninitialized_array_type_traits.hpp"
#include "iox/size.hpp"
#include "iox/type_traits.hpp"
#include "iox/uninitialized_array.hpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>

namespace iox
{
// constants
constexpr uint64_t DYNAMIC_EXTENT = std::numeric_limits<uint64_t>::max();

namespace detail
{
template <uint64_t Offset, uint64_t Count, uint64_t Extent>
static constexpr uint64_t subspan_capacity()
{
    if (Count != DYNAMIC_EXTENT)
    {
        return Count;
    }

    if (Extent != DYNAMIC_EXTENT)
    {
        return Extent - Offset;
    }

    return DYNAMIC_EXTENT;
}
} // namespace detail

// Specialization/ implementation of C++17's std::size() and std::data()

/// @brief Returns a pointer to the block of memory containing the elements of the range.
/// @tparam Container Type of the container
/// @param container A container or view with a data() member function
/// @return Returns container.data()
template <typename Container>
constexpr auto data(Container& container) -> decltype(container.data());

/// @brief Returns a pointer to the block of memory containing the elements of the range.
/// @tparam Container Type of the container
/// @param container A container or view with a data() member function
/// @return Returns container.data()
template <typename Container>
constexpr auto data(const Container& container) -> decltype(container.data());

/// @brief Returns a pointer to the block of memory containing the elements of the range.
/// @tparam T Type of the array
/// @tparam N Size of the array
/// @param array An array of arbitrary type
/// @return Returns array
template <typename T, uint64_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
constexpr T* data(T (&array)[N]) noexcept;

/// @brief Returns a pointer to the block of memory containing the elements of the range.
/// @tparam T Type of the iox::UninitializedArray
/// @tparam N Size of the iox::UninitializedArray
/// @param array An iox::UninitializedArray of arbitrary type
/// @return Returns iox::UninitializedArray
template <typename T, uint64_t N, template <typename, uint64_t> class Buffer>
constexpr T* data(UninitializedArray<T, N, Buffer>& uninit_array) noexcept;

template <typename T, uint64_t Extent = DYNAMIC_EXTENT>
class span;

namespace detail
{
template <uint64_t I>
using span_size_constant = std::integral_constant<uint64_t, I>;

template <typename T>
struct extent_impl : span_size_constant<DYNAMIC_EXTENT>
{
};

template <typename T, uint64_t N>
// NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
struct extent_impl<T[N]> : span_size_constant<N>
{
};

template <typename T, uint64_t N>
struct extent_impl<iox::UninitializedArray<T, N>> : span_size_constant<N>
{
};

template <typename T, uint64_t N>
struct extent_impl<iox::span<T, N>> : span_size_constant<N>
{
};

template <typename T>
using extent_t = extent_impl<remove_cvref_t<T>>;

template <typename T>
struct is_span : std::false_type
{
};

template <typename T, uint64_t Extent>
struct is_span<span<T, Extent>> : std::true_type
{
};

template <typename T>
using is_not_span_t = iox::negation<is_span<std::decay_t<T>>>;

template <typename Container, typename T>
using container_has_convertible_data_t =
    is_convertible_t<std::remove_pointer_t<decltype(iox::data(std::declval<Container>()))>, T>;

template <typename Container>
using is_integral_sized_container_t = std::is_integral<decltype(iox::size(std::declval<Container>()))>;

template <typename From, uint64_t FromExtent, typename To, uint64_t ToExtent>
using enable_if_conversion_allowed_t =
    std::enable_if_t<(ToExtent == DYNAMIC_EXTENT || ToExtent == FromExtent) && is_convertible_t<From, To>::value>;

// SFINAE Verify it the passed array can be converted to a span<T>.
template <typename Array, typename T, uint64_t Extent>
using enable_if_compatible_array_t =
    std::enable_if_t<(Extent == DYNAMIC_EXTENT || Extent == detail::extent_t<Array>::value)
                     && container_has_convertible_data_t<Array, T>::value>;

// SFINAE Verify it the passed container can be converted to a span<T>.
template <typename Container, typename T>
using is_compatible_container_t = conjunction<is_not_span_t<Container>,
                                              iox::is_not_iox_array_t<Container>,
                                              iox::is_not_c_array_t<Container>,
                                              container_has_convertible_data_t<Container, T>,
                                              is_integral_sized_container_t<Container>>;

template <typename Container, typename T>
using enable_if_compatible_container_t = std::enable_if_t<is_compatible_container_t<Container, T>::value>;

template <typename Container, typename T, uint64_t Extent>
using enable_if_compatible_dynamic_container_t =
    std::enable_if_t<is_compatible_container_t<Container, T>::value && Extent == DYNAMIC_EXTENT>;

template <uint64_t Extent>
class span_storage
{
  public:
    constexpr explicit span_storage(uint64_t) noexcept
    {
    }
    constexpr uint64_t size() const noexcept
    {
        return Extent;
    }
};

template <>
struct span_storage<DYNAMIC_EXTENT>
{
    constexpr explicit span_storage(uint64_t size) noexcept
        : m_size(size)
    {
    }
    constexpr uint64_t size() const noexcept
    {
        return m_size;
    }

    span_storage(const span_storage& other) noexcept = default;
    span_storage& operator=(const span_storage& other) noexcept = default;
    span_storage(span_storage&& other) noexcept = default;
    span_storage& operator=(span_storage&& other) noexcept = default;
    ~span_storage() noexcept = default;

  private:
    uint64_t m_size{0};
};

template <typename T>
constexpr T* to_address(T* p) noexcept
{
    static_assert(!std::is_function<T>::value, "Error: T must not be a function type.");
    return p;
}

template <typename Ptr>
constexpr auto to_address(const Ptr& p) noexcept -> decltype(std::pointer_traits<Ptr>::to_address(p))
{
    return std::pointer_traits<Ptr>::to_address(p);
}

template <typename Ptr, typename... None>
constexpr auto to_address(const Ptr& p, None...) noexcept
{
    return to_address(p.operator->());
}

} // namespace detail

/// @brief  C++11 compatible span implementation. The class template span describes an object that can refer to a
///         contiguous sequence of objects with the first element of the sequence at position zero. A span can either
///         have a static extent, in which case the number of elements in the sequence is known at compile-time and
///         encoded in the type, or a dynamic extent. If a span has dynamic extent, a typical implementation holds two
///         members: a pointer to T and a size. A span with static extent may have only one member: a pointer to T.
///         http://eel.is/c++draft/views contains the latest C++20 draft
/// @tparam T - element type; must be a complete object type that is not an abstract class type
/// @tparam Extent - the number of elements in the sequence, or 'iox::DYNAMIC_EXTENT' if dynamic
template <typename T, uint64_t Extent>
class span final : public detail::span_storage<Extent>
{
  private:
    using span_storage_t = detail::span_storage<Extent>;

  public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using uint64_type = uint64_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator = span_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    using const_iterator = const T*;
    using const_reference = const T&;
    using size_type = decltype(Extent);
    using index_type = size_type;
    static constexpr uint64_t extent = Extent;

    // constructors, copy, assignment, and destructor

    constexpr span() noexcept = delete;

    /// @brief Constructs a span that is a view over the range [first, first + count);
    /// @tparam It Type of the iterator
    /// @param first iterator of where to start creating the span
    /// @param count number of elements counting from first
    template <typename It>
    constexpr span(It first, uint64_t count) noexcept;

    /// @brief Constructs a span that is a view over the range [first, last);
    /// @tparam It Type of the start/end iterators
    /// @param begin iterator of where to start creating the span
    /// @param end iterator of where to stop
    template <typename It, typename = std::enable_if_t<!std::is_convertible<It, uint64_t>::value>>
    constexpr span(It begin, It end) noexcept;

    /// @brief Constructs a span that is a view over the array arr; the resulting span has size() == N and data() ==
    /// std::data(arr)
    /// @tparam N implicit size of the array
    /// @param array used to construct the span
    // NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)
    template <uint64_t N, typename = detail::enable_if_compatible_array_t<T (&)[N], T, Extent>>
    constexpr explicit span(T (&array)[N]) noexcept;
    // NOLINTEND(cppcoreguidelines-avoid-c-arrays, hicpp-avoid-c-arrays)

    /// @brief Constructs a span that is a view over the uninitialized array; the resulting span has size() == N and
    /// data() == std::data(arr)
    /// @tparam U implicit element type stored in the uninitialized array
    /// @tparam N implicit capacity of the uninitialized array
    /// @param array uninitialized array used to create the span
    template <typename U,
              uint64_t N,
              template <typename, uint64_t>
              class Buffer,
              typename = detail::enable_if_compatible_array_t<iox::UninitializedArray<U, N, Buffer>&, T, Extent>>
    constexpr explicit span(iox::UninitializedArray<U, N, Buffer>& array) noexcept;

    /// @brief Constructs a span that is a view over a const uninitialized array; the resulting span has size() == N and
    /// data() == std::data(arr)
    /// @tparam U implicit element type stored in the uninitialized array
    /// @tparam N implicit capacity of the uninitialized array
    /// @param array const uninitialized array used to create the span
    template <typename U,
              uint64_t N,
              template <typename, uint64_t>
              class Buffer,
              typename = detail::enable_if_compatible_array_t<const iox::UninitializedArray<U, N, Buffer>&, T, Extent>>
    constexpr explicit span(const iox::UninitializedArray<U, N, Buffer>& array) noexcept;

    /// @brief Convert from container (with std::size() and std::data() to iox::span)
    /// @tparam Container Type of passed container
    /// @param container container which is used to create the span
    template <typename Container, typename = detail::enable_if_compatible_dynamic_container_t<Container&, T, Extent>>
    constexpr explicit span(Container& container) noexcept;

    /// @brief Convert from container (with std::size() and std::data() to iox::span)
    /// @tparam Container Type of passed container
    /// @param container container which is used to create the span
    template <typename Container,
              typename = detail::enable_if_compatible_dynamic_container_t<const Container&, T, Extent>>
    constexpr explicit span(const Container& container) noexcept;

    constexpr span(const span& other) noexcept = default;

    /// @brief Conversions between spans of compatible types
    /// @tparam U implicit type which is wrapped by the span
    /// @tparam OtherExtent the implicit value of the extent of other
    /// @param other the passed span
    template <typename U,
              uint64_t OtherExtent,
              typename = detail::enable_if_conversion_allowed_t<U, OtherExtent, T, Extent>>
    constexpr explicit span(const span<U, OtherExtent>& other);

    constexpr span& operator=(const span&) noexcept = default;

    constexpr span(span&& other) noexcept;
    constexpr span& operator=(span&& other) noexcept;

    ~span() noexcept = default;

    // subviews

    /// @brief obtains a subspan consisting of the first N elements of the sequence
    /// @tparam Count number of elements in the returned span
    /// @return the subspan
    template <uint64_t Count>
    constexpr span<T, Count> first() const noexcept;

    /// @brief obtains a subspan consisting of the first N elements of the sequence
    /// @param count number of elements in the returned span
    /// @return the subspan
    constexpr span<T, DYNAMIC_EXTENT> first(uint64_t count) const noexcept;

    /// @brief obtains a subspan consisting of the last N elements of the sequence
    /// @tparam Count number of elements in the returned span
    /// @return the subspan
    template <uint64_t Count>
    constexpr span<T, Count> last() const noexcept;


    /// @brief obtains a subspan consisting of the last N elements of the sequence
    /// @param count number of elements in the returned span
    /// @return the subspan
    constexpr span<T, DYNAMIC_EXTENT> last(uint64_t count) const noexcept;

    /// @brief obtains a subspan
    /// @tparam Offset in the returned span
    /// @tparam Count number of elements in the returned span
    /// @return the subspan
    template <uint64_t Offset, uint64_t Count = DYNAMIC_EXTENT>
    constexpr span<T, detail::subspan_capacity<Offset, Count, Extent>()> subspan() const noexcept;

    /// @brief obtains a subspan with dynamic extend
    /// @param offset in the returned span
    /// @param count number of elements in the returned span
    /// @return the subspan
    constexpr span<T, DYNAMIC_EXTENT> subspan(uint64_t offset, uint64_t count = DYNAMIC_EXTENT) const noexcept;

    // observers

    /// @brief returns the number of elements in the sequence
    /// @return number of elements
    constexpr uint64_t size() const noexcept;

    /// @brief returns the size of the sequence in bytes
    /// @return size in bytes
    constexpr uint64_t size_bytes() const noexcept;

    /// @brief checks if the sequence is empty
    /// @return true if empty, otherwise false
    constexpr bool empty() const noexcept;

    // element access

    /// @brief access the first element
    /// @return reference to first element
    constexpr T& front() const noexcept;

    /// @brief access the last element
    /// @return reference to first element
    constexpr T& back() const noexcept;

    /// @brief accesses an element of the sequence
    /// @param index which should be accessed
    /// @return reference to respective element
    constexpr T& operator[](uint64_t index) const noexcept;

    /// @brief returns a pointer to the beginning of the sequence of elements
    /// @return pointer of type T to the first element
    constexpr T* data() const noexcept;

    // iterator support

    /// @brief returns an iterator to the beginning
    /// @return iterator to the beginning
    constexpr iterator begin() const noexcept;

    /// @brief returns an iterator to the end
    /// @return iterator to the end
    constexpr iterator end() const noexcept;

    /// @brief returns a reverse iterator to the beginning
    /// @return reverse_iterator to the beginning
    constexpr reverse_iterator rbegin() const noexcept;

    /// @brief returns a reverse iterator to the end
    /// @return reverse_iterator to the end
    constexpr reverse_iterator rend() const noexcept;

  private:
    T* m_data;
};

template <typename T, uint64_t X>
span<const uint8_t, (X == DYNAMIC_EXTENT ? DYNAMIC_EXTENT : sizeof(T) * X)> as_bytes(span<T, X> s) noexcept;

template <typename T, uint64_t X, typename = std::enable_if_t<!std::is_const<T>::value>>
span<uint8_t, (X == DYNAMIC_EXTENT ? DYNAMIC_EXTENT : sizeof(T) * X)> as_writable_bytes(span<T, X> s) noexcept;

} // namespace iox

#include "iox/detail/span.inl"

#endif // IOX_HOOFS_VOCABULARY_SPAN_HPP
