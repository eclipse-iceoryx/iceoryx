// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
// Copyright (c) 2023 by Dennis Liu. All rights reserved.
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

#ifndef IOX_HOOFS_UTILITY_CONVERT_HPP
#define IOX_HOOFS_UTILITY_CONVERT_HPP

#include "iox/posix_call.hpp"
#include "iox/string.hpp"

#include <climits>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>

namespace iox
{
/// @brief Collection of static methods for conversion from and to string.
/// @code
///     std::string number      = iox::convert::toString(123);
///     std::string someClass   = iox::convert::toString(someToStringConvertableObject);
///
///     int i;
///     unsigned int a;
///     if ( iox::convert::from_string("123", i) ) {}  // will succeed
///     if ( iox::convert::from_string("-123", a) ) {} // will fail since -123 is not unsigned
/// @endcode
/// @todo iox-#260 Refactor 'convert' so that one can use 'into' to directly to convert numbers to strings:
/// 'ClassExpectingAnIoxString(iox::into<iox::string<100>>(42)'
class convert
{
  public:
    enum class NumberType
    {
        INTEGER,
        UNSIGNED_INTEGER,
        FLOAT
    };

    static constexpr int32_t STRTOULL_BASE = 10;

    /// @brief Converts every type which is either a pod (plain old data) type or is convertable
    ///         to a string (this means that the operator std::string() is defined)
    /// @param Source type of the value which should be converted to a string
    /// @param[in] t value which should be converted to a string
    /// @return string representation of t
    template <typename Source>
    static typename std::enable_if<!std::is_convertible<Source, std::string>::value, std::string>::type
    toString(const Source& t) noexcept;

    /// @brief Converts every type which is either a pod (plain old data) type or is convertable
    ///         to a string (this means that the operator std::string() is defined)
    /// @param Source type of the value which should be converted to a string
    /// @param[in] t value which should be converted to a string
    /// @return string representation of t
    template <typename Source>
    static typename std::enable_if<std::is_convertible<Source, std::string>::value, std::string>::type
    toString(const Source& t) noexcept;

    /// @brief Sets dest from a given string. If the conversion fails false is
    ///         returned and the value of dest is undefined.
    /// @param[in] v string which contains the value of dest
    /// @param[in] dest destination to which the value should be written
    /// @return false = if the conversion fails otherwise true
    template <typename IoxString, typename std::enable_if_t<is_iox_string<IoxString>::value, int> = 0>
    static iox::optional<IoxString> from_string(const char* v) noexcept;

    /// @brief  for those not Destination not string
    /// @tparam Destination
    /// @param v
    /// @return
    template <typename Destination, typename std::enable_if_t<!is_iox_string<Destination>::value, int> = 0>
    static iox::optional<Destination> from_string(const char* v) noexcept;

  private:
    /// @todo iox-#2055
    /// @brief
    /// @tparam ValueType
    /// @tparam CallType
    /// @param call
    /// @param errno_cache
    /// @param end_ptr
    /// @param v
    /// @return
    template <typename TargetType, typename CallType>
    static iox::optional<TargetType>
    evaluate_return_value(CallType& call, decltype(errno) errno_cache, const char* end_ptr, const char* v) noexcept;

    /// @todo iox-#2055
    /// @brief Check the edge cases. If
    /// @tparam Destination
    /// @param errno_cache
    /// @param end_ptr
    /// @param v
    /// @param check_value
    /// @return
    template <typename TargetType, typename RequireCheckValType>
    static bool check_edge_case(decltype(errno) errno_cache,
                                const char* end_ptr,
                                const char* v,
                                const RequireCheckValType& require_check_val) noexcept;

    /// @todo iox-#2055
    /// @brief
    /// @param v
    /// @return
    static bool start_with_neg_sign(const char* v) noexcept;

    /// @todo iox-#2055
    /// @brief
    /// @tparam RequireCheckValType
    /// @param end_ptr
    /// @param v
    /// @param require_check_val
    /// @return
    template <typename RequireCheckValType>
    static bool
    is_valid_input(const char* end_ptr, const char* v, const RequireCheckValType& require_check_val) noexcept;


    /// @todo iox-#2055
    /// @brief
    /// @param errno_cache
    /// @return
    static bool is_valid_errno(decltype(errno) errno_cache) noexcept;


    /// @todo iox-#2055
    /// @brief
    /// @tparam TargetType
    /// @tparam RequireCheckValType
    /// @param require_check_val
    /// @return
    template <typename TargetType, typename RequireCheckValType>
    static bool is_within_range(const RequireCheckValType& require_check_val) noexcept;


    /// @todo iox-#2055 helper, I believe there's a more elegant way to implement this.
  private:
    template <typename T>
    struct is_iox_string : std::false_type
    {
    };

    template <uint64_t Capacity>
    struct is_iox_string<iox::string<Capacity>> : std::true_type
    {
    };

    template <typename T>
    struct GetCapacity;

    template <uint64_t Capacity>
    struct GetCapacity<iox::string<Capacity>>
    {
        static constexpr uint64_t value = Capacity;
    };
};

} // namespace iox

#include "iox/detail/convert.inl"

#endif // IOX_HOOFS_UTILITY_CONVERT_HPP
