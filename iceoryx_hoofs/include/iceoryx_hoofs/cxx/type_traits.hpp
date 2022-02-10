// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_CXX_TYPE_TRAITS_HPP
#define IOX_HOOFS_CXX_TYPE_TRAITS_HPP

#include <type_traits>

namespace iox
{
namespace cxx
{
///
/// @brief Conditionally add const to type T if C has the const qualifier
/// @tparam T is the type to conditionally add the const qualifier
/// @tparam Condition is the type which determines if the const qualifier needs to be added to T
///
template <typename T, typename C>
struct add_const_conditionally
{
    using type = T;
};
template <typename T, typename C>
struct add_const_conditionally<T, const C>
{
    using type = const T;
};
///
/// @brief Helper type for add_const_conditionally which adds const to type T if C has the const qualifier
///
template <typename T, typename C>
using add_const_conditionally_t = typename add_const_conditionally<T, C>::type;

///
/// @brief Helper value to bind a static_assert to a type
/// @code
/// static_assert(always_false_v<Foo>, "Not implemented for the given type!");
/// @endcode
///
template <typename>
constexpr bool always_false_v = false;

// windows defines __cplusplus as 199711L
#if __cplusplus < 201703L && !defined(_WIN32)
template <typename C, typename... Cargs>
using invoke_result = std::result_of<C(Cargs...)>;
#elif __cplusplus >= 201703L || defined(_WIN32)
template <typename C, typename... Cargs>
using invoke_result = std::invoke_result<C, Cargs...>;
#endif

///
/// @brief Verifies whether the passed Callable type is in fact invocable with the given arguments
///
template <typename Callable, typename... ArgTypes>
struct is_invocable
{
    // This variant is chosen when Callable(ArgTypes) successfully resolves to a valid type, i.e. is invocable.
    /// @note result_of is deprecated, switch to invoke_result in C++17
    template <typename C, typename... As>
    static constexpr std::true_type test(typename cxx::invoke_result<C, As...>::type*) noexcept
    {
        return {};
    }

    // This is chosen if Callable(ArgTypes) does not resolve to a valid type.
    template <typename C, typename... As>
    static constexpr std::false_type test(...) noexcept
    {
        return {};
    }

    // Test with nullptr as this can stand in for a pointer to any type.
    static constexpr bool value = decltype(test<Callable, ArgTypes...>(nullptr))::value;
};

///
/// @brief Verifies whether the passed Callable type is in fact invocable with the given arguments
///        and the result of the invocation is convertible to ReturnType.
///
/// @note This is an implementation of std::is_invokable_r (C++17).
///
template <typename ReturnType, typename Callable, typename... ArgTypes>
struct is_invocable_r
{
    template <typename C, typename... As>
    static constexpr std::true_type test(
        std::enable_if_t<std::is_convertible<typename cxx::invoke_result<C, As...>::type, ReturnType>::value>*) noexcept
    {
        return {};
    }

    template <typename C, typename... As>
    static constexpr std::false_type test(...) noexcept
    {
        return {};
    }

    // Test with nullptr as this can stand in for a pointer to any type.
    static constexpr bool value = decltype(test<Callable, ArgTypes...>(nullptr))::value;
};

///
/// @brief Check whether T is a function pointer with arbitrary signature
///
template <typename T>
struct is_function_pointer : std::false_type
{
};
template <typename ReturnType, typename... ArgTypes>
struct is_function_pointer<ReturnType (*)(ArgTypes...)> : std::true_type
{
};

/// @brief Maps a sequence of any types to the type void
template <typename...>
using void_t = void;
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_TYPE_TRAITS_HPP
