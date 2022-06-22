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
#ifndef IOX_HOOFS_CXX_GENERIC_RAII_HPP
#define IOX_HOOFS_CXX_GENERIC_RAII_HPP

#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/cxx/function_ref.hpp"

namespace iox
{
namespace cxx
{
/// @brief The GenericRAII class is a simple helper class to apply the C++ RAII
///             idiom quickly. You set 2 functions, one which is called in the
///             constructor and another function is called in the destructor
///             which can be useful when handling resources.
/// @code
/// // This example leads to a console output of:
/// // hello world
/// // I am doing stuff
/// // goodbye
/// void someFunc() {
///     auto raii{[](){ std::cout << "hello world\n"; },
///                 [](){ std::cout << "goodbye\n"; }};
///     std::cout << "I am doing stuff\n";
///     // raii goes out of scope here and the cleanupFunction is called in the
///     // destructor
/// }
/// @endcode
template <uint64_t CleanupCapacity = DEFAULT_FUNCTION_CAPACITY>
class GenericRAIIWithVariableCapacity
{
  public:
    /// @brief constructor which creates GenericRAII that calls only the cleanupFunction on destruction
    /// @param[in] cleanupFunction callable which will be called in the destructor
    explicit GenericRAIIWithVariableCapacity(const function<void(), CleanupCapacity>& cleanupFunction) noexcept;

    /// @brief constructor which calls initFunction and stores the cleanupFunction which will be
    ///           called in the destructor
    /// @param[in] initFunction callable which will be called in the constructor
    /// @param[in] cleanupFunction callable which will be called in the destructor
    GenericRAIIWithVariableCapacity(const function_ref<void()>& initFunction,
                                    const function<void()>& cleanupFunction) noexcept;

    /// @brief calls m_cleanupFunction callable if it was set in the constructor
    ~GenericRAIIWithVariableCapacity() noexcept;

    GenericRAIIWithVariableCapacity(const GenericRAIIWithVariableCapacity&) = delete;
    GenericRAIIWithVariableCapacity& operator=(const GenericRAIIWithVariableCapacity&) = delete;

    /// @brief move constructor which moves a generic raii object without calling the cleanupFunction
    GenericRAIIWithVariableCapacity(GenericRAIIWithVariableCapacity&& rhs) noexcept;

    /// @brief move assignment which moves a generic raii object without calling the cleanupFunction
    GenericRAIIWithVariableCapacity& operator=(GenericRAIIWithVariableCapacity&& rhs) noexcept;

  private:
    void destroy() noexcept;

  private:
    function<void(), CleanupCapacity> m_cleanupFunction;
};

// This alias can be removed with C++17 and class template argument deduction
using GenericRAII = GenericRAIIWithVariableCapacity<>;

} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/generic_raii.inl"

#endif // IOX_HOOFS_CXX_GENERIC_RAII_HPP
