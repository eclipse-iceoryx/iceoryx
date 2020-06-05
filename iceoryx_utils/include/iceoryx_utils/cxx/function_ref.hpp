// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_UTILS_CXX_FUNCTION_REF_HPP
#define IOX_UTILS_CXX_FUNCTION_REF_HPP

#include <cstddef>
#include <iostream>
#include <memory>
#include <type_traits>

namespace iox
{
namespace cxx
{
/// @brief cxx::function_ref is a non-owning reference to a callable.
///
///        It has these features:
///         * No heap usage
///         * No exceptions
///         * Stateful lambda support
///         * C++11/14 support
/// @code
///         // Usage as function parameter
///         void fuu(cxx::function_ref<void()> callback)
///         {
///             callback();
///         }
///         // Call the lambda
///         fuu([]{ doSomething(); });
///
///         // Usage with l-values
///         // Pitfall: Ensure that lifetime of callable suits the point in time of calling callback()
///         auto callable = [&]{ doSomething(); };
///         cxx::function_ref<void()> callback(callable);
///         // Call the callback
///         callback();
/// @endcode

template <typename SignatureType>
class function_ref;

template <class ReturnType, class... ArgTypes>
class function_ref<ReturnType(ArgTypes...)>
{
    using SignatureType = ReturnType(ArgTypes...);
    template <typename T>
    using EnableIfNotFunctionRef = typename std::enable_if<!std::is_same<std::decay<T>, function_ref>::value>::type;

  public:
    /// @brief Creates an empty function_ref
    function_ref() noexcept;

    /// @brief D'tor
    ~function_ref() noexcept = default;

    /// @brief Copy c'tor
    function_ref(const function_ref&) noexcept = default;

    /// @brief Copy assignment operator
    function_ref& operator=(const function_ref&) noexcept = default;

    /// @brief Create a function_ref
    template <typename CallableType, typename = EnableIfNotFunctionRef<CallableType>>
    function_ref(CallableType&& callable) noexcept;

    /// @brief Moves a function_ref
    function_ref(function_ref&& rhs) noexcept;

    /// @brief Move assignment operator
    function_ref& operator=(function_ref&& rhs) noexcept;

    /// @brief Calls the provided callable
    ReturnType operator()(ArgTypes... args) const noexcept;

    /// @brief Checks whether a valid target is contained
    explicit operator bool() const noexcept;

    void swap(function_ref& rhs) noexcept;

  private:
    /// @brief Raw pointer of the callable
    void* m_target{nullptr};

    /// @brief Function pointer to the callable
    ReturnType (*m_functionPointer)(void*, ArgTypes...){nullptr};
};

} // namespace cxx
} // namespace iox

#include "iceoryx_utils/internal/cxx/function_ref.inl"

#endif
