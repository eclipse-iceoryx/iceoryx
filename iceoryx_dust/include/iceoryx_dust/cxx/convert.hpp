// Copyright (c) 2019, 2021 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2022 by NXP. All rights reserved.
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
#ifndef IOX_DUST_CXX_CONVERT_HPP
#define IOX_DUST_CXX_CONVERT_HPP

#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"
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
namespace cxx
{
/// @brief Collection of static methods for conversion from and to string.
/// @code
///     std::string number      = cxx::convert::toString(123);
///     std::string someClass   = cxx::convert::toString(someToStringConvertableObject);
///
///     int i;
///     unsigned int a;
///     if ( cxx::convert::fromString("123", i) ) {}  // will succeed
///     if ( cxx::convert::fromString("-123", a) ) {} // will fail since -123 is not unsigned
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
    template <typename Destination>
    static bool fromString(const char* v, Destination& dest) noexcept;

    /// @brief Sets dest from a given string. If the conversion fails false is
    ///         returned and the value of dest is undefined.
    /// @param[in] v string which contains the value of dest
    /// @param[in] dest destination to which the value should be written
    /// @return false = if the conversion fails otherwise true
    template <uint64_t Capacity>
    static bool fromString(const char* v, string<Capacity>& dest) noexcept;

    /// @brief checks if a given string v is a number
    /// @param[in] v string which contains the number
    /// @param[in] type is the expected contained type in v
    /// @return true if the given string is a number, otherwise false
    static bool stringIsNumber(const char* v, const NumberType type) noexcept;

  private:
    static bool stringIsNumberWithErrorMessage(const char* v, const NumberType type) noexcept;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_dust/internal/cxx/convert.inl"

#endif // IOX_DUST_CXX_CONVERT_HPP
