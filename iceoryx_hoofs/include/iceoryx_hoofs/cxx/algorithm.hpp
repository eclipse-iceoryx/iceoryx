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
#ifndef IOX_HOOFS_CXX_ALGORITHM_HPP
#define IOX_HOOFS_CXX_ALGORITHM_HPP

#include "iceoryx_hoofs/cxx/attributes.hpp"

#include <cstdint>
#include <type_traits>

namespace iox
{
namespace algorithm
{
/// @brief Returns the maximum gained with operator<() of an arbitrary amount
///          of variables of the same type. Helper function which is required as generic
///          recursive template endpoint.
/// @param T type which implements operator<()
/// @param[in] left value which should be compared
/// @return returns the given argument left
template <typename T>
constexpr T maxVal(const T& left) noexcept;

/// @brief Returns the maximum gained with operator<() of an arbitrary amount
///          of variables of the same type. Helper function which takes two arguments and returns the
///          greater one.
/// @param T type which implements operator<()
/// @param[in] left value which should be compared
/// @param[in] right value which should be compared
/// @return returns the maximum value of the set {left, right}
template <typename T>
constexpr T maxVal(const T& left, const T& right) noexcept;

/// @brief Returns the maximum gained with operator<() of an arbitrary amount
///          of variables of the same type.
/// @param T type which implements operator<()
/// @param[in] left value which should be compared
/// @param[in] right value which should be compared
/// @param[in] args... an arbitrary amount of values
/// @return returns the maximum value of the set {left, right, args...}
template <typename T, typename... Targs>
constexpr T maxVal(const T& left, const T& right, const Targs&... args) noexcept;

/// @brief Returns the minimum gained with operator<() of an arbitrary amount
///          of variables of the same type. Helper function which is required as generic
///          recursive template endpoint.
/// @param T type which implements operator<()
/// @param[in] left value which should be compared
/// @return returns the given argument left
template <typename T>
constexpr T minVal(const T& left) noexcept;

/// @brief Returns the minimum gained with operator<() of an arbitrary amount
///          of variables of the same type. Helper function which takes two arguments and returns the
///          smaller one.
/// @param T type which implements operator<()
/// @param[in] left value which should be compared
/// @param[in] right value which should be compared
/// @return returns the minimum of the set {left, right}
template <typename T>
constexpr T minVal(const T& left, const T& right) noexcept;

/// @brief Returns the minimum gained with operator<() of an arbitrary amount
///          of variables of the same type.
/// @param T type which implements operator<()
/// @param[in] left value which should be compared
/// @param[in] right value which should be compared
/// @param[in] args... an arbitrary amount of values
/// @return returns the minimum of the set {left, right, args...}
template <typename T, typename... Targs>
constexpr T minVal(const T& left, const T& right, const Targs&... args) noexcept;

/// @brief Returns true if T is equal to CompareType, otherwise false
/// @param T type to compare to
/// @param CompareType the type to which T is compared
/// @return true if the types T and CompareType are equal, otherwise false
template <typename T, typename CompareType>
constexpr bool doesContainType() noexcept;

/// @brief Returns true if T is contained the provided type list
/// @param T type to compare to
/// @param CompareType, Next, Remainder the type list in which T should be contained
/// @return true if the T is contained in the type list, otherwise false
template <typename T, typename CompareType, typename Next, typename... Remainder>
constexpr bool doesContainType() noexcept;

/// @brief Finalizes the recursion of doesContainValue
/// @return always false
template <typename T>
inline constexpr bool doesContainValue(const T) noexcept;

/// @brief Returns true if value of T is found in the ValueList, otherwise false
/// @tparam T type of the value to check
/// @tparam ValueList is a list of values to check for a specific value
/// @param[in] value to look for in the ValueList
/// @param[in] firstValueListEntry is the first variadic argument of ValueList
/// @param[in] remainingValueListEntries are the remaining variadic arguments of ValueList
/// @return true if value is contained in the ValueList, otherwise false
/// @note be aware that value is tested for exact equality with the entries of ValueList and regular floating-point
/// comparison rules apply
template <typename T, typename... ValueList>
inline constexpr bool
doesContainValue(const T value, const T firstValueListEntry, const ValueList... remainingValueListEntries) noexcept;
} // namespace algorithm
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/algorithm.inl"

#endif // IOX_HOOFS_CXX_ALGORITHM_HPP
