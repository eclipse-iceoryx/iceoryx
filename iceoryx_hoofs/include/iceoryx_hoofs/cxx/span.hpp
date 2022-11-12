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
#ifndef IOX_HOOFS_CXX_SPAN_HPP
#define IOX_HOOFS_CXX_SPAN_HPP

#include "iceoryx_hoofs/cxx/requires.hpp"
#include "iceoryx_hoofs/cxx/span_iterator.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iox/uninitialized_array.hpp"

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <utility>

using namespace std;

namespace iox
{
// constants
constexpr uint64_t dynamic_extent = std::numeric_limits<uint64_t>::max();

template <class From, class To>
constexpr bool is_convertible_v = std::is_convertible<From, To>::value;

// Implementation of C++17 std::size() and std::data()
template <class C>
constexpr auto size(const C& c) -> decltype(c.size())
{
    return c.size();
}

template <class T, std::uint64_t N>
constexpr std::uint64_t size(const T (&)[N]) noexcept
{
    return N;
}

template <class C>
constexpr auto data(C& c) -> decltype(c.data())
{
    return c.data();
}

template <class C>
constexpr auto data(const C& c) -> decltype(c.data())
{
    return c.data();
}

template <class T, std::uint64_t N>
constexpr T* data(T (&array)[N]) noexcept
{
    return array;
}

template <class E>
constexpr const E* data(std::initializer_list<E> il) noexcept
{
    return il.begin();
}

template <typename T, uint64_t Extent = dynamic_extent>
class span;

namespace internal
{
template <uint64_t I>
using size_constant = std::integral_constant<uint64_t, I>;

template <typename T>
struct extent_impl : size_constant<dynamic_extent>
{
};

template <typename T, uint64_t N>
struct extent_impl<T[N]> : size_constant<N>
{
};

template <typename T, uint64_t N>
struct extent_impl<iox::UninitializedArray<T, N>> : size_constant<N>
{
};

template <typename T, uint64_t N>
struct extent_impl<iox::span<T, N>> : size_constant<N>
{
};

template <typename T>
using extent_t = extent_impl<remove_cvref_t<T>>;

template <typename T>
struct is_span : std::false_type
{
};

template <typename T, uint64_t Extent_t>
struct is_span<span<T, Extent_t>> : std::true_type
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
    std::enable_if_t<(ToExtent == dynamic_extent || ToExtent == FromExtent) && is_convertible_t<From, To>::value>;

// SFINAE Verify it the passed array can be converted to a span<T>.
template <typename Array, typename T, uint64_t Extent>
using enable_if_compatible_array_t =
    std::enable_if_t<(Extent == dynamic_extent || Extent == internal::extent_t<Array>::value)
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
    std::enable_if_t<is_compatible_container_t<Container, T>::value && Extent == dynamic_extent>;

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
struct span_storage<dynamic_extent>
{
    constexpr explicit span_storage(uint64_t size) noexcept
        : size_(size)
    {
    }
    constexpr uint64_t size() const noexcept
    {
        return size_;
    }

  private:
    uint64_t size_;
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

} // namespace internal

/// @brief  C++11 compatible span implementation. The class template span describes an object that can refer to a
///         contiguous sequence of objects with the first element of the sequence at position zero. A span can either
///         have a static extent, in which case the number of elements in the sequence is known at compile-time and
///         encoded in the type, or a dynamic extent. If a span has dynamic extent, a typical implementation holds two
///         members: a pointer to T and a size. A span with static extent may have only one member: a pointer to T.
///         http://eel.is/c++draft/views contains the latest C++20 draft
/// @tparam T - element type; must be a complete object type that is not an abstract class type
/// @tparam Extent - the number of elements in the sequence, or std::dynamic_extent if dynamic
template <typename T, uint64_t Extent>
class span : public internal::span_storage<Extent>
{
  private:
    using span_storage_t = internal::span_storage<Extent>;

  public:
    using element_type = T;
    using value_type = std::remove_cv_t<T>;
    using uint64_type = uint64_t;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    using iterator = iox::span_iterator<T>;
    using reverse_iterator = std::reverse_iterator<iterator>;
    static constexpr uint64_t extent = Extent;

    // constructors, copy, assignment, and destructor

    /// @brief Constructs an empty span whose data() == nullptr and size() == 0.
    constexpr span() noexcept;

    /// @brief Constructs a span that is a view over the range [first, first + count);
    /// @tparam It
    /// @param first
    /// @param count
    template <typename It>
    constexpr span(It first, uint64_t count) noexcept;

    /// @brief Constructs a span that is a view over the range [first, last);
    /// @tparam It
    /// @tparam End
    /// @tparam
    /// @param begin
    /// @param end
    template <typename It, typename End, typename = std::enable_if_t<!std::is_convertible<End, uint64_t>::value>>
    constexpr span(It begin, End end) noexcept;

    /// @brief Constructs a span that is a view over the array arr; the resulting span has size() == N and data() ==
    /// std::data(arr)
    /// @tparam
    /// @tparam N
    /// @param array
    template <uint64_t N, typename = internal::enable_if_compatible_array_t<T (&)[N], T, Extent>>
    constexpr explicit span(T (&array)[N]) noexcept;

    /// @brief Constructs a span that is a view over the array arr; the resulting span has size() == N and data() ==
    /// std::data(arr)
    /// @tparam U
    /// @tparam
    /// @tparam N
    /// @param array
    template <typename U,
              uint64_t N,
              typename = internal::enable_if_compatible_array_t<iox::UninitializedArray<U, N>&, T, Extent>>
    constexpr explicit span(iox::UninitializedArray<U, N>& array) noexcept;

    /// @brief Constructs a span that is a view over the array arr; the resulting span has size() == N and data() ==
    /// std::data(arr)
    /// @tparam U
    /// @tparam
    /// @tparam N
    /// @param array
    template <typename U,
              uint64_t N,
              typename = internal::enable_if_compatible_array_t<const iox::UninitializedArray<U, N>&, T, Extent>>
    constexpr explicit span(const iox::UninitializedArray<U, N>& array) noexcept;

    /// @brief Convert from container (with std::size() and std::data() to iox::span)
    /// @tparam Container
    ///
    /// @param container
    template <typename Container, typename = internal::enable_if_compatible_dynamic_container_t<Container&, T, Extent>>
    constexpr explicit span(Container& container) noexcept;

    /// @brief Convert from container (with std::size() and std::data() to iox::span)
    /// @tparam Container
    /// @tparam
    /// @param container
    template <typename Container,
              typename = internal::enable_if_compatible_dynamic_container_t<const Container&, T, Extent>>
    constexpr explicit span(const Container& container) noexcept;

    constexpr span(const span& other) noexcept = default;

    /// @brief Conversions between spans of compatible types
    /// @tparam U
    /// @tparam
    /// @tparam OtherExtent
    /// @param other
    template <typename U,
              uint64_t OtherExtent,
              typename = internal::enable_if_conversion_allowed_t<U, OtherExtent, T, Extent>>
    constexpr explicit span(const span<U, OtherExtent>& other);

    /// @brief Defaulted copy constructor copies the size and data pointer
    /// @param other
    /// @return
    constexpr span& operator=(const span& other) noexcept = default;

    /// @brief
    ~span() noexcept = default;

    // subviews

    /// @brief obtains a subspan consisting of the first N elements of the sequence
    /// @tparam Count
    /// @return
    template <uint64_t Count>
    constexpr span<T, Count> first() const noexcept;

    /// @brief obtains a subspan consisting of the first N elements of the sequence
    /// @param count
    /// @return
    constexpr span<T, dynamic_extent> first(uint64_t count) const noexcept;

    /// @brief obtains a subspan consisting of the last N elements of the sequence
    /// @tparam Count
    /// @return
    template <uint64_t Count>
    constexpr span<T, Count> last() const noexcept;


    /// @brief obtains a subspan consisting of the last N elements of the sequence
    /// @param count
    /// @return
    constexpr span<T, dynamic_extent> last(uint64_t count) const noexcept;

    /// @brief
    /// @tparam Offset
    /// @tparam Count
    /// @return
    template <uint64_t Offset, uint64_t Count = dynamic_extent>
    constexpr span<T, (Count != dynamic_extent ? Count : (Extent != dynamic_extent ? Extent - Offset : dynamic_extent))>
    subspan() const noexcept;

    /// @brief obtains a subspan
    /// @param offset
    /// @param count
    /// @return
    constexpr span<T, dynamic_extent> subspan(uint64_t offset, uint64_t count = dynamic_extent) const noexcept;

    // observers

    /// @brief returns the number of elements in the sequence
    /// @return
    constexpr uint64_t size() const noexcept;

    /// @brief returns the size of the sequence in bytes
    /// @return
    constexpr uint64_t size_bytes() const noexcept;

    /// @brief checks if the sequence is empty
    /// @return
    constexpr bool empty() const noexcept;

    // element access

    /// @brief access the first element
    /// @return
    constexpr T& front() const noexcept;

    /// @brief access the last element
    /// @return
    constexpr T& back() const noexcept;

    /// @brief accesses an element of the sequence
    /// @param idx
    /// @return
    constexpr T& operator[](uint64_t idx) const noexcept;
    /// @brief returns a pointer to the beginning of the sequence of elements
    /// @return
    constexpr T* data() const noexcept;

    // iterator support

    /// @brief returns an iterator to the beginning
    /// @return
    constexpr iterator begin() const noexcept;

    /// @brief returns an iterator to the end
    /// @return
    constexpr iterator end() const noexcept;

    /// @brief returns a reverse iterator to the beginning
    /// @return
    constexpr reverse_iterator rbegin() const noexcept;

    /// @brief returns a reverse iterator to the end
    /// @return
    constexpr reverse_iterator rend() const noexcept;

  private:
    T* m_data;
};

} // namespace iox

#include "iceoryx_hoofs/internal/cxx/span.inl"

#endif // IOX_HOOFS_CXX_SPAN_HPP
