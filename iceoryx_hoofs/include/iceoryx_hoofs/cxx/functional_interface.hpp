// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_FUNCTIONAL_POLICY_HPP
#define IOX_HOOFS_CXX_FUNCTIONAL_POLICY_HPP

#include "iceoryx_hoofs/cxx/function_ref.hpp"
#include "iceoryx_hoofs/cxx/helplets.hpp"
#include <iostream>

namespace iox
{
namespace cxx
{
namespace internal
{
template <typename T, class = void>
struct HasValueMethod : std::false_type
{
};

template <typename T>
struct HasValueMethod<T, std::void_t<decltype(&T::value)>> : std::true_type
{
};

template <typename T, class = void>
struct HasGetErrorMethod : std::false_type
{
};

template <typename T>
struct HasGetErrorMethod<T, std::void_t<decltype(&T::get_error)>> : std::true_type
{
};

template <typename T>
struct Expect
{
    void expect(const char* const msg) const noexcept;
};

template <typename T>
struct ExpectWithValue
{
    using expect_value_t = typename T::value_t;
    expect_value_t& expect(const char* const msg) & noexcept;
    const expect_value_t& expect(const char* const msg) const& noexcept;
    expect_value_t&& expect(const char* const msg) && noexcept;
    const expect_value_t&& expect(const char* const msg) const&& noexcept;
};

template <typename T>
struct AndThenWithValue
{
    using and_then_callback_t = cxx::function_ref<void(typename T::value_t&)>;

    T& and_then(const and_then_callback_t& callable) & noexcept;
    const T& and_then(const and_then_callback_t& callable) const& noexcept;
    T&& and_then(const and_then_callback_t& callable) && noexcept;
    const T&& and_then(const and_then_callback_t& callable) const&& noexcept;
};

template <typename T>
struct AndThen
{
    using and_then_callback_t = cxx::function_ref<void()>;

    T& and_then(const and_then_callback_t& callable) & noexcept;
    const T& and_then(const and_then_callback_t& callable) const& noexcept;
    T&& and_then(const and_then_callback_t& callable) && noexcept;
    const T&& and_then(const and_then_callback_t& callable) const&& noexcept;
};

template <typename T>
struct OrElseWithValue
{
    using or_else_callback_t = cxx::function_ref<void(typename T::error_t&)>;

    T& or_else(const or_else_callback_t& callable) & noexcept;
    const T& or_else(const or_else_callback_t& callable) const& noexcept;
    T&& or_else(const or_else_callback_t& callable) && noexcept;
    const T&& or_else(const or_else_callback_t& callable) const&& noexcept;
};

template <typename T>
struct OrElse
{
    using or_else_callback_t = cxx::function_ref<void()>;

    T& or_else(const or_else_callback_t& callable) & noexcept;
    const T& or_else(const or_else_callback_t& callable) const& noexcept;
    T&& or_else(const or_else_callback_t& callable) && noexcept;
    const T&& or_else(const or_else_callback_t& callable) const&& noexcept;
};

template <typename T, bool HasValue, bool HasError>
struct FunctionalInterfaceImpl;

template <typename T>
struct FunctionalInterfaceImpl<T, false, false> : public Expect<T>, public AndThen<T>, public OrElse<T>
{
};

template <typename T>
struct FunctionalInterfaceImpl<T, true, false> : public ExpectWithValue<T>, public AndThenWithValue<T>, public OrElse<T>
{
};

template <typename T>
struct FunctionalInterfaceImpl<T, true, true>
    : public ExpectWithValue<T>, public AndThenWithValue<T>, public OrElseWithValue<T>
{
};

template <typename T>
struct FunctionalInterfaceImpl<T, false, true> : public Expect<T>, public AndThen<T>, public OrElseWithValue<T>
{
};
} // namespace internal

template <typename T>
using FunctionalInterface =
    internal::FunctionalInterfaceImpl<T, internal::HasValueMethod<T>::value, internal::HasGetErrorMethod<T>::value>;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/functional_interface.inl"

#endif
