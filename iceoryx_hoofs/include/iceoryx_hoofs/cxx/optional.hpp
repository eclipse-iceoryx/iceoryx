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
#ifndef IOX_HOOFS_CXX_OPTIONAL_HPP
#define IOX_HOOFS_CXX_OPTIONAL_HPP

#include "iceoryx_hoofs/cxx/functional_interface.hpp"
#include "iceoryx_hoofs/cxx/requires.hpp"
#include "iceoryx_hoofs/iceoryx_hoofs_types.hpp"

#include <new> // needed for placement new in the construct_value member function
#include <utility>

namespace iox
{
namespace cxx
{
/// @brief Helper struct which is used to signal an empty optional.
///         It is equivalent to no value.
struct nullopt_t
{
};
constexpr nullopt_t nullopt = nullopt_t();

/// @brief helper struct which is used to call the in-place-construction constructor
struct in_place_t
{
};
constexpr in_place_t in_place{};

/// @brief Optional implementation from the C++17 standard with C++11. The
///         interface is analog to the C++17 standard and it can be used in
///         factory functions which can fail.
///
/// @code
///     #include "iceoryx_hoofs/cxx/optional.hpp"
///
///     cxx::optional<void*> SomeFactory() {
///         void *memory = malloc(1234);
///         if ( memory == nullptr )
///             return cxx::nullopt_t();
///         else
///             return cxx::make_optional<void*>(memory);
///     }
///
///     int main() {
///         auto var = SomeFactory();
///         // never forget the has_value call before working with an optional
///         if ( var.has_value() ) {
///             // do stuff with var
///         }
///     }
/// @endcode
template <typename T>
class optional : public FunctionalInterface<optional<T>, T, void>
{
  public:
    using type = T;

    /// @brief Creates an optional which has no value. If you access such an
    ///         optional via .value() or the arrow operator the application
    ///         terminates.
    optional() noexcept;

    /// @brief Creates an optional which has no value. If you access such an
    ///         optional via .value() or the arrow operator the application
    ///         terminates.
    // NOLINTNEXTLINE(hicpp-explicit-conversions) for justification see doxygen
    optional(const nullopt_t& noValue) noexcept;

    /// @brief Creates an optional by forwarding value to the constructor of
    ///         T. This optional has a value.
    /// @param[in] value rvalue of type T which will be moved into the optional
    // NOLINTNEXTLINE(hicpp-explicit-conversions) for justification see doxygen
    optional(T&& value) noexcept;

    /// @brief Creates an optional by using the copy constructor of T.
    /// @param[in] value lvalue of type T which will be copy constructed into the optional
    // NOLINTNEXTLINE(hicpp-explicit-conversions) for justification see doxygen
    optional(const T& value) noexcept;

    /// @brief Creates an optional and an object inside the optional on construction by perfectly forwarding args to the
    /// constructor of T. Could be used e.g. when T is not copyable/movable.
    /// @tparam Targs is the template parameter pack for the perfectly forwarded arguments
    /// @param[in] in_place_t compile time variable to distinguish between constructors with certain behavior
    template <typename... Targs>
    // in_place_t is a compile time variable to call the in-place-construction
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    explicit optional(in_place_t, Targs&&... args) noexcept;

    /// @brief The destructor will call the destructor of T if a value is set.
    ~optional() noexcept;

    /// @brief Constructs a value with the copy constructor if rhs has a value.
    ///         Otherwise it contains no value.
    /// @param[in] rhs source of the copy
    optional(const optional& rhs) noexcept;

    /// @brief Constructs a value with the move constructor if rhs has a value.
    ///         Otherwise it contains no value.
    /// @param[in] rhs source of the move
    optional(optional&& rhs) noexcept;

    /// @brief Copies an optional. If the optional has a value then the copy
    ///         assignment of that value is called. If the optional has no
    ///         value a new value is constructed with the copy constructor.
    /// @param[in] rhs source of the copy
    /// @return reference to the current optional
    optional& operator=(const optional& rhs) noexcept;

    /// @brief Moves an optional. If the optional has a value then the move
    ///         assignment of that value is called. If the optional has no
    ///         value a new value is constructed with the move constructor.
    /// @param[in] rhs source of the move
    /// @return reference to the current optional
    optional& operator=(optional&& rhs) noexcept;

    /// @brief If the optionals have values it compares these values by using
    ///         their comparison operator.
    /// @param[in] rhs value to which this optional should be compared to
    /// @return true if the contained values are equal, otherwise false
    constexpr bool operator==(const optional<T>& rhs) const noexcept;

    /// @brief Comparison with nullopt_t for easier unset optional comparison
    /// @return true if the optional is unset, otherwise false
    constexpr bool operator==(const nullopt_t& rhs) const noexcept;

    /// @brief If the optionals have values it compares these values by using
    ///         their comparison operator.
    /// @param[in] rhs value to which this optional should be compared to
    /// @return true if the contained values are not equal, otherwise false
    constexpr bool operator!=(const optional<T>& rhs) const noexcept;

    /// @brief comparison with nullopt_t for easier unset optional comparison
    /// @return true if the optional is set, otherwise false
    constexpr bool operator!=(const nullopt_t& rhs) const noexcept;

    /// @brief Direct assignment of the underlying value. If the optional has no
    ///         value then a new T is constructed by forwarding the assignment to
    ///         T's constructor.
    ///        If the optional has a value the assignment operator of T is called.
    /// @param[in] value value to assign to the underlying optional value
    /// @return reference to the current optional
    template <typename U = T>
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature) return type is optional&
    typename std::enable_if<!std::is_same<U, optional<T>&>::value, optional>::type& operator=(U&& value) noexcept;

    /// @brief Returns a pointer to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return pointer of type const T to the underlying type
    const T* operator->() const noexcept;

    /// @brief Returns a reference to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return reference of type const T to the underlying type
    const T& operator*() const noexcept;

    /// @brief Returns a pointer to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return pointer of type T to the underlying type
    T* operator->() noexcept;

    /// @brief Returns a reference to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return reference of type T to the underlying type
    T& operator*() noexcept;

    /// @brief Will return true if the optional contains a value, otherwise false.
    /// @return true if optional contains a value, otherwise false
    constexpr explicit operator bool() const noexcept;

    /// @brief Will return true if the optional contains a value, otherwise false.
    /// @return true if optional contains a value, otherwise false
    constexpr bool has_value() const noexcept;

    /// @brief A new element is constructed by forwarding the arguments to the
    ///         constructor of T.
    ///         If the optional has a value then the destructor of T is called.
    /// @param[in] perfectly forwards args to the constructor of T to perform a placement
    ///             new
    /// @return reference to the underlying type
    template <typename... Targs>
    T& emplace(Targs&&... args) noexcept;

    /// @brief Calls the destructor of T if the optional has a value.
    ///         If the optional has no value, nothing happens. After that call
    ///         the optional has no more value.
    void reset() noexcept;

    /// @brief Returns a reference to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return reference to the underlying type
    T& value() & noexcept;

    /// @brief Returns a const reference to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return const reference to the underlying type
    const T& value() const& noexcept;

    /// @brief Returns a rvalue reference to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return rvalue reference to the underlying type
    T&& value() && noexcept;

    /// @brief Returns a const rvalue reference to the underlying value. If the optional has no
    ///         value the application terminates. You need to verify that the
    ///         optional has a value by calling has_value() before using it.
    /// @return const rvalue reference to the underlying type
    const T&& value() const&& noexcept;

  private:
    // the hasValue member should be first member in the memory layout to reveal casting
    // bugs early.
    //
    // See the following problem:
    //   void initHandle(void * ptr) {
    //     Handle * handle = static_cast<Handle>(ptr);
    //   }
    //   void doStuff(cxx::optional<Handle> & handle) {
    //     // uses optional<Handle> instead of handle, if the bool is the second parameter such
    //     // mistakes can be introduced quickly in low level c abstraction classes
    //     initHandle(&handle);
    //   }
    bool m_hasValue{false};
    // safe access is guaranteed since the array is wrapped inside the optional
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    alignas(T) byte_t m_data[sizeof(T)];

  private:
    template <typename... Targs>
    void construct_value(Targs&&... args) noexcept;
    void destruct_value() noexcept;
};

/// @brief Creates an optional which contains a value by forwarding the arguments
///         to the constructor of T.
/// @param[in] args arguments which will be perfectly forwarded to the constructor
///             of T
/// @return optional which contains T constructed with args
template <typename OptionalBaseType, typename... Targs>
optional<OptionalBaseType> make_optional(Targs&&... args) noexcept;
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/optional.inl"

#endif // IOX_HOOFS_CXX_OPTIONAL_HPP
