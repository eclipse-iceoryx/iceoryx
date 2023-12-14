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
    /// @brief Evaluates the return value from a POSIX call or similar function call.
    /// This function checks if the call resulted in an error and validates the edge cases
    /// of the conversion from string to a numeric value. It returns an optional containing
    /// the converted value if successful, or nullopt if there's an error or edge case failure.
    /// @tparam TargetType The type to which the string is being converted.
    /// @tparam CallType The type of the call object.
    /// @param call Reference to the call object.
    /// @param errno_cache Cached errno value for error checking.
    /// @param end_ptr Pointer to the character after the last character used in the conversion.
    /// @param v Pointer to the input string.
    /// @return Optional containing the converted value or nullopt.
    template <typename TargetType, typename CallType>
    static iox::optional<TargetType>
    evaluate_return_value(CallType& call, decltype(errno) errno_cache, const char* end_ptr, const char* v) noexcept;

    /// @brief Checks for edge cases in string conversion to a numeric type.
    /// This function evaluates various conditions that might indicate an edge
    /// case or an error in the conversion process from a string to a numeric type.
    /// It checks for invalid input strings, conversion failures, errors indicated
    /// by 'errno', and whether the converted value falls within the expected range.
    /// @tparam TargetType The numeric type to which the string is being converted.
    /// @tparam RequireCheckValType The type of the value being checked against edge cases.
    /// @param errno_cache Cached value of 'errno' to check for conversion errors.
    /// @param end_ptr Pointer to the character following the last character used
    ///                in the conversion, used to check for conversion failures.
    /// @param v Pointer to the input string being converted.
    /// @param require_check_val The value to be checked against edge cases.
    /// @return True if no edge cases or errors are detected, false otherwise.
    template <typename TargetType, typename RequireCheckValType>
    static bool check_edge_case(decltype(errno) errno_cache,
                                const char* end_ptr,
                                const char* v,
                                const RequireCheckValType& require_check_val) noexcept;

    /// @brief Determines if a given string starts with a negative sign after skipping any leading spaces.
    /// This function examines the input string and returns true if the first non-space character
    /// is a negative sign ('-'). It is useful for parsing strings representing numeric values,
    /// where the sign of the number needs to be determined.
    /// @param v Pointer to the null-terminated input string to be checked.
    /// @return True if the string starts with a negative sign after any leading spaces, false otherwise.

    static bool start_with_neg_sign(const char* v) noexcept;

    /// @brief Checks if the string input is valid based on the results of its conversion.
    /// This function determines the validity of the input string by analyzing the outcome
    /// of a conversion function like strtoull. It checks whether the end_ptr overlaps with
    /// the beginning of v, indicating no conversion occurred, or if the conversion did not
    /// terminate at a null character '\0', both scenarios being indicative of an invalid input.
    /// @tparam RequireCheckValType Type of the value being checked.
    /// @param end_ptr Pointer to the character after the last character used in the conversion.
    /// @param v Pointer to the input string.
    /// @param require_check_val The value obtained from the conversion, used for comparison.
    /// @return True if the input is valid for conversion, false otherwise.
    template <typename RequireCheckValType>
    static bool
    is_valid_input(const char* end_ptr, const char* v, const RequireCheckValType& require_check_val) noexcept;


    /// @brief Checks if the cached errno indicates a valid conversion.
    /// This function assesses the errno value to determine if it represents
    /// a common error scenario during string-to-numeric conversion.
    /// @param errno_cache Cached errno value to be checked.
    /// @return True if errno does not indicate an error, false otherwise.
    static bool is_valid_errno(decltype(errno) errno_cache) noexcept;


    /// @brief Checks if the given value is within the allowable range for the target type.
    /// This function evaluates whether the specified value falls within the numeric limits
    /// of the target type, considering both upper and lower bounds.
    /// @tparam TargetType The numeric type to be checked against.
    /// @tparam RequireCheckValType Type of the value being checked.
    /// @param require_check_val The value to be evaluated against the target type's range.
    /// @return True if the value is within range, false otherwise.
    template <typename TargetType, typename RequireCheckValType>
    static bool is_within_range(const RequireCheckValType& require_check_val) noexcept;


    /// @todo iox-#2055 I believe there's a more elegant way to implement this. What do you think?
  private:
    /// @brief Trait struct to determine if a type is an iox::string.
    /// Provides a compile-time check to identify if a given type is an instance of iox::string.
    /// @tparam T Type to be checked.
    template <typename T>
    struct is_iox_string : std::false_type
    {
    };

    /// Specialization of is_iox_string for iox::string.
    template <uint64_t Capacity>
    struct is_iox_string<iox::string<Capacity>> : std::true_type
    {
    };

    /// @brief Helper struct to extract the capacity of an iox::string at compile time.
    /// Provides a mechanism to obtain the capacity of an iox::string type.
    template <typename T>
    struct GetCapacity;

    /// Specialization of GetCapacity for iox::string.
    template <uint64_t Capacity>
    struct GetCapacity<iox::string<Capacity>>
    {
        static constexpr uint64_t value = Capacity;
    };
};

} // namespace iox

#include "iox/detail/convert.inl"

#endif // IOX_HOOFS_UTILITY_CONVERT_HPP
