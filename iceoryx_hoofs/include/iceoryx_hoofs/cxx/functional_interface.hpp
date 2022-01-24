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
struct HasValueMethod<T, cxx::void_t<decltype(std::declval<T>().value())>> : std::true_type
{
};

template <typename T, class = void>
struct HasGetErrorMethod : std::false_type
{
};

template <typename T>
struct HasGetErrorMethod<T, cxx::void_t<decltype(std::declval<T>().get_error())>> : std::true_type
{
};

template <typename T>
struct Expect
{
    /// @brief Expects that the object is valid, otherwise the method prints the
    ///        provided message and induces a fatal error
    /// @param[in] msg Message which will be printed when the object is invalid
    void expect(const char* const msg) const noexcept;
};

template <typename T, typename ValueType>
struct ExpectWithValue
{
    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return a reference to the contained value
    ValueType& expect(const char* const msg) & noexcept;

    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return a const reference the contained value
    const ValueType& expect(const char* const msg) const& noexcept;

    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return rvalue reference to the contained value
    ValueType&& expect(const char* const msg) && noexcept;

    /// @brief Expects that the object is valid and returns the contained value, otherwise
    //         the method prints the provided message and induces a fatal error
    /// @param[in] msg Message which will be printed when the object is invalid
    /// @return const rvalue reference to the contained value
    const ValueType&& expect(const char* const msg) const&& noexcept;
};

template <typename T, typename ValueType>
struct ValueOr
{
    /// @brief When the object contains a value a copy will be returned otherwise a
    ///        a copy of alternative will be returned
    /// @param[in]  alternative the return value which will be used when the object does
    ///             not contain a value
    /// @return A copy of the contained value if possible otherwise a copy of alternative
    ValueType value_or(const ValueType& alternative) const noexcept;
};

template <typename T, typename ValueType>
struct AndThenWithValue
{
    using and_then_callback_t = cxx::function_ref<void(ValueType&)>;
    using const_and_then_callback_t = cxx::function_ref<void(const ValueType&)>;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when valid
    /// @return reference to *this
    T& and_then(const and_then_callback_t& callable) & noexcept;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value const reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when valid
    /// @return const reference to *this
    const T& and_then(const const_and_then_callback_t& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when valid
    /// @return rvalue reference to *this
    T&& and_then(const and_then_callback_t& callable) && noexcept;

    /// @brief Calls the provided callable when the object is valid and provides the underlying
    ///        value const reference as argument to the callable. If the object is not valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when valid
    /// @return const rvalue reference to *this
    const T&& and_then(const const_and_then_callback_t& callable) const&& noexcept;
};

template <typename T>
struct AndThen
{
    using and_then_callback_t = cxx::function_ref<void()>;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable Callable to be called when valid
    /// @return reference to *this
    T& and_then(const and_then_callback_t& callable) & noexcept;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable Callable to be called when valid
    /// @return const reference to *this
    const T& and_then(const and_then_callback_t& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable Callable to be called when valid
    /// @return rvalue reference to *this
    T&& and_then(const and_then_callback_t& callable) && noexcept;

    /// @brief Calls the provided callable when the object is valid. If the object is not
    ///        valid, nothing happens.
    /// @param[in] callable Callable to be called when valid
    /// @return const rvalue reference to *this
    const T&& and_then(const and_then_callback_t& callable) const&& noexcept;
};

template <typename T, typename ErrorType>
struct OrElseWithValue
{
    using or_else_callback_t = cxx::function_ref<void(ErrorType&)>;
    using const_or_else_callback_t = cxx::function_ref<void(const ErrorType&)>;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return reference to *this
    T& or_else(const or_else_callback_t& callable) & noexcept;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error const reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return const reference to *this
    const T& or_else(const const_or_else_callback_t& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return rvalue reference to *this
    T&& or_else(const or_else_callback_t& callable) && noexcept;

    /// @brief Calls the provided callable when the object is invalid and provide the underlying
    ///        error const reference as argument to the callable. If the object is valid, nothing
    ///        happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return const rvalue reference to *this
    const T&& or_else(const const_or_else_callback_t& callable) const&& noexcept;
};

template <typename T>
struct OrElse
{
    using or_else_callback_t = cxx::function_ref<void()>;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return reference to *this
    T& or_else(const or_else_callback_t& callable) & noexcept;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return const reference to *this
    const T& or_else(const or_else_callback_t& callable) const& noexcept;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return rvalue reference to *this
    T&& or_else(const or_else_callback_t& callable) && noexcept;

    /// @brief Calls the provided callable when the object is invalid. If the object is valid,
    ///        nothing happens.
    /// @param[in] callable Callable to be called when invalid
    /// @return const rvalue reference to *this
    const T&& or_else(const or_else_callback_t& callable) const&& noexcept;
};

template <typename T, typename ValueType, typename ErrorType>
struct FunctionalInterfaceImpl : public ExpectWithValue<T, ValueType>,
                                 public ValueOr<T, ValueType>,
                                 public AndThenWithValue<T, ValueType>,
                                 public OrElseWithValue<T, ErrorType>
{
};

template <typename T>
struct FunctionalInterfaceImpl<T, void, void> : public Expect<T>, public AndThen<T>, public OrElse<T>
{
};

template <typename T, typename ValueType>
struct FunctionalInterfaceImpl<T, ValueType, void> : public ExpectWithValue<T, ValueType>,
                                                     public ValueOr<T, ValueType>,
                                                     public AndThenWithValue<T, ValueType>,
                                                     public OrElse<T>
{
};

template <typename T, typename ErrorType>
struct FunctionalInterfaceImpl<T, void, ErrorType>
    : public Expect<T>, public AndThen<T>, public OrElseWithValue<T, ErrorType>
{
};
} // namespace internal

/// @brief Provides a functional interface for types which have a bool conversion
///        operator. This provides the methods
///          * and_then
///          * or_else
///          * map
///          * expect
///        When the class has a value method the method
///          * value_or
///        is added and and_then provides a reference in the callback to the underlying value.
///        When the class has a get_error method the or_else method has a parameter to access
///        a reference to the underlying error.
///
/// @note When inheriting from this type one does not have to write additional unit tests.
///       Instead add a factory for your class to `test_cxx_functional_interface_types.hpp`,
///       add the type to the FunctionalInterfaceImplementations and all typed tests will be
///       generated.
template <typename T, typename ValueType, typename ErrorType>
using FunctionalInterface = internal::FunctionalInterfaceImpl<T, ValueType, ErrorType>;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/functional_interface.inl"

#endif
