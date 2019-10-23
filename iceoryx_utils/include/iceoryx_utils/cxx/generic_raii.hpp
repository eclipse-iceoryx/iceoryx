// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#pragma once

#include <functional>

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
class GenericRAII
{
  public:
    /// @brief constructor which calls initFunction and stores the cleanupFunction which will be
    ///           called in the destructor
    /// @param[in] initFunction callable which will be called in the constructor
    /// @param[in] cleanupFunction callable which will be called in the destructor
    GenericRAII(const std::function<void()> initFunction, const std::function<void()> cleanupFunction) noexcept;

    /// @brief calls m_cleanupFunction callable if it was set in the constructor
    ~GenericRAII() noexcept;

    GenericRAII(const GenericRAII&) = delete;
    GenericRAII& operator=(const GenericRAII&) = delete;

    /// @brief move constructor which moves a generic raii object without calling the cleanupFunction
    GenericRAII(GenericRAII&& rhs) noexcept;

    /// @brief move assignment which moves a generic raii object without calling the cleanupFunction
    GenericRAII& operator=(GenericRAII&& rhs) noexcept;

  private:
    std::function<void()> m_cleanupFunction;
};

} // namespace cxx
} // namespace iox
