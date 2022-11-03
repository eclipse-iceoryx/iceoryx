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
#ifndef IOX_HOOFS_CXX_STRING_HPP
#define IOX_HOOFS_CXX_STRING_HPP

#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/internal/containers/uninitialized_array.hpp"
#include "iceoryx_hoofs/internal/cxx/string_internal.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace iox
{
namespace cxx
{
template <typename T, typename ReturnType>
using IsStringOrCharArrayOrChar =
    typename std::enable_if<(is_cxx_string<T>::value || is_char_array<T>::value || std::is_same<T, std::string>::value
                             || std::is_same<T, char>::value),
                            ReturnType>::type;

template <typename T, typename ReturnType>
using IsStringOrCharArray =
    typename std::enable_if<(is_cxx_string<T>::value || is_char_array<T>::value || std::is_same<T, std::string>::value),
                            ReturnType>::type;

template <typename T, typename ReturnType>
using IsStdStringOrCharArrayOrChar =
    typename std::enable_if<(is_char_array<T>::value || std::is_same<T, std::string>::value
                             || std::is_same<T, char>::value),
                            ReturnType>::type;

template <typename T, typename ReturnType>
using IsCxxStringOrCharArray =
    typename std::enable_if<(is_cxx_string<T>::value || is_char_array<T>::value), ReturnType>::type;

template <typename T1, typename T2, typename ReturnType>
using IsCxxStringOrCharArrayOrChar =
    typename std::enable_if<(is_char_array<T1>::value || is_cxx_string<T1>::value || std::is_same<T1, char>::value)
                                && (is_char_array<T2>::value || is_cxx_string<T2>::value
                                    || std::is_same<T2, char>::value),
                            ReturnType>::type;

template <typename T1, typename T2, typename ReturnType>
using IsCxxStringAndCxxStringOrCharArrayOrChar =
    typename std::enable_if<((is_char_array<T1>::value || std::is_same<T1, char>::value) && is_cxx_string<T2>::value)
                                || (is_cxx_string<T1>::value
                                    && (is_char_array<T2>::value || std::is_same<T2, char>::value))
                                || (is_cxx_string<T1>::value && is_cxx_string<T2>::value),
                            ReturnType>::type;

/// @brief concatenates two iox::cxx::strings/string literals/chars
///
/// @param [in] iox::cxx::strings/string literals/chars to concatenate
///
/// @return a new iox::cxx::string with capacity equal to the sum of the capacities of the concatenated strings/chars
///
/// @code
///     string<5> fuu("cdefg");
///     auto bar = iox::cxx::concatenate(fuu, "ahc");
/// @endcode
template <typename T1, typename T2>
IsCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2>::value>> concatenate(const T1& str1,
                                                                                           const T2& str2) noexcept;

/// @brief concatenates an arbitrary number of iox::cxx::strings, string literals or chars
///
/// @param [in] iox::cxx::strings/string literals/chars to concatenate
///
/// @return a new iox::cxx::string with capacity equal to the sum of the capacities of the concatenated strings/chars
///
/// @code
///     string<4> fuu("cdef");
///     auto bar = iox::cxx::concatenate(fuu, "g", "ah", fuu);
/// @endcode
template <typename T1, typename T2, typename... Targs>
IsCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2, Targs...>::value>>
concatenate(const T1& str1, const T2& str2, const Targs&... targs) noexcept;

/// @brief concatenates two iox::cxx::strings or one iox::cxx::string and one string literal/char; concatenation of two
/// string literals/chars is not possible
///
/// @param [in] iox::cxx::strings/string literal/char to concatenate
///
/// @return a new iox::cxx::string with capacity equal to the sum of the capacities of the concatenated strings/chars
template <typename T1, typename T2>
IsCxxStringAndCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2>::value>>
operator+(const T1& str1, const T2& str2) noexcept;

/// @brief struct used to define a compile time variable which is used to distinguish between
/// constructors with certain behavior
struct TruncateToCapacity_t
{
    explicit TruncateToCapacity_t() = default;
};
constexpr TruncateToCapacity_t TruncateToCapacity{};

/// @brief string implementation with some adjustments in the API, because we are not allowed to throw exceptions or use
/// heap.
template <uint64_t Capacity>
class string
{
    static_assert(Capacity > 0U, "The capacity of the fixed string must be greater than 0!");

  public:
    /// @brief creates an empty string with size 0
    constexpr string() noexcept = default;

    /// @brief copy constructor
    ///
    /// @param [in] other is the copy origin
    string(const string& other) noexcept;

    /// @brief move constructor
    ///
    /// @param [in] other is the move origin
    string(string&& other) noexcept;

    /// @brief copy assignment
    ///
    /// @param [in] rhs is the copy origin
    ///
    /// @return reference to self
    string& operator=(const string& rhs) noexcept;

    /// @brief move assignment
    ///
    /// @param [in] rhs is the move origin
    ///
    /// @return reference to self
    string& operator=(string&& rhs) noexcept;

    /// @brief destructor
    ~string() noexcept = default;

    /// @brief creates a new string of given capacity as a copy of other with compile time check whether the capacity of
    /// other is less than or equal to this' capacity
    ///
    /// @param [in] other is the copy origin
    template <uint64_t N>
    // NOLINTNEXTLINE(hicpp-explicit-conversions) copy constructor for string with different capacity
    string(const string<N>& other) noexcept;

    /// @brief moves other to this with compile time check whether the capacity of other is less than or equal to
    /// this' capacity
    ///
    /// @param [in] other is the move origin
    template <uint64_t N>
    // NOLINTNEXTLINE(hicpp-explicit-conversions) move constructor for string with different capacity
    string(string<N>&& other) noexcept;

    /// @brief assigns rhs fixed string to this with compile time check whether the capacity of rhs is less than or
    /// equal to this' capacity
    ///
    /// @param [in] rhs is the copy origin
    ///
    /// @return reference to self
    template <uint64_t N>
    string& operator=(const string<N>& rhs) noexcept;

    /// @brief moves rhs fixed string to this with compile time check whether the capacity of rhs is less than or
    /// equal to this' capacity
    ///
    /// @param [in] rhs is the move origin
    ///
    /// @return reference to self
    template <uint64_t N>
    string& operator=(string<N>&& rhs) noexcept;

    /// @brief conversion constructor for char array with compile time check if the array size is less than or equal
    /// to the string capacity
    ///
    /// @tparam N is the implicit template parameter for the char array size
    /// @param [in] other is the char array
    ///
    /// @note if the array is not zero-terminated, the last value will be overwritten with 0
    ///
    /// @code
    ///     #include "iceoryx_hoofs/cxx/string.hpp"
    ///     using namespace iox::cxx;
    ///
    ///     int main()
    ///     {
    ///         string<4> fuu("abcd");
    ///     }
    /// @endcode
    template <uint64_t N>
    // avoid-c-arrays: cxx::string wraps char array
    // explicit-conversions: we want to assign string literals to the string, like string<10> str = "abc"; this is safe
    // because cxx::string wraps char array
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays, hicpp-explicit-conversions)
    string(const char (&other)[N]) noexcept;

    /// @brief conversion constructor for std::string to string which truncates characters if the std::string size is
    /// greater than the string capacity
    ///
    /// @param [in] TruncateToCapacity_t is a compile time variable which is used to distinguish between
    /// constructors with certain behavior
    /// @param [in] other is the std::string to convert
    /// @attention truncates characters if the std::string size is greater than the string capacity
    ///
    /// @code
    ///     #include "iceoryx_hoofs/cxx/string.hpp"
    ///     using namespace iox::cxx;
    ///
    ///     int main()
    ///     {
    ///         std::string bar = "bar";
    ///         string<4> fuu(TruncateToCapacity, bar);
    ///     }
    /// @endcode
    // TruncateToCapacity_t is a compile time variable to distinguish between constructors
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    string(TruncateToCapacity_t, const std::string& other) noexcept;

    // Avoid any implicit conversion, as templates are always preferred in overload resolution
    template <typename T>
    string(TruncateToCapacity_t, const T other) = delete;

    /// @brief constructor from cstring to string. Constructs the string with the first count characters of the cstring
    /// including null characters. If count is greater than the string capacity the remainder of the characters are
    /// truncated.
    ///
    /// @param [in] TruncateToCapacity_t is a compile time variable which is used to distinguish between
    /// constructors with certain behavior
    /// @param [in] other is the cstring to convert
    /// @param [in] count is the number of characters for constructing the string
    ///
    /// @code
    ///     #include "iceoryx_hoofs/cxx/string.hpp"
    ///     using namespace iox::cxx;
    ///
    ///     int main()
    ///     {
    ///         string<4> fuu(TruncateToCapacity, "abcd", 2);
    ///     }
    /// @endcode
    // TruncateToCapacity_t is a compile time variable to distinguish between constructors
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    string(TruncateToCapacity_t, const char* const other, const uint64_t count) noexcept;

    /// @brief assigns a char array to string with compile time check if the array size is less than or equal
    /// to the string capacity
    ///
    /// @tparam [in] N is the implicit template parameter for the char array size
    /// @param [in] rhs is the char array
    ///
    /// @return reference to self
    ///
    /// @note if the array is not zero-terminated, the last value will be overwritten with 0
    ///
    /// @code
    ///     #include "iceoryx_hoofs/cxx/string.hpp"
    ///     using namespace iox::cxx;
    ///
    ///     int main()
    ///     {
    ///         string<4> fuu = "abcd";
    ///     }
    /// @endcode
    template <uint64_t N>
    // We want to assign string literals to the cxx::string, like myString = "abc";
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    string& operator=(const char (&rhs)[N]) noexcept;

    /// @brief fixed string assignment with compile time check if capacity of str is less than or equal to this'
    /// capacity
    ///
    /// @param [in] str is the fixed string object to assign
    ///
    /// @return reference to self
    template <uint64_t N>
    string& assign(const string<N>& str) noexcept;

    /// @brief assigns a char array to string with compile time check if the array size is less than or equal to the
    /// string capacity
    ///
    /// @tparam [in] N is the implicit template parameter for the char array size
    /// @param [in] str is the char array
    ///
    /// @return reference to self
    ///
    /// @note if the array is not zero-terminated, the last value will be overwritten with 0
    ///
    /// @code
    ///
    ///     #include "iceoryx_hoofs/cxx/string.hpp"
    ///     using namespace iox::cxx;
    ///
    ///     int main()
    ///     {
    ///         string<4> fuu;
    ///         char bar[] = "abcd";
    ///         fuu.assign(bar);
    ///     }
    /// @endcode
    template <uint64_t N>
    // We want to assign string literals to the cxx::string, like myString.assign("abc");
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    string& assign(const char (&str)[N]) noexcept;

    /// @brief assigns a std::string to string. The assignment fails if the std::string size is greater than the string
    /// capacity.
    ///
    /// @param [in] str is the std::string to assign
    ///
    /// @return true if the assignment succeeds, otherwise false
    bool unsafe_assign(const std::string& str) noexcept;

    /// @brief compares self and an iox::cxx::string, std::string or char array
    ///
    /// @param [in] other is the string to compare with self
    ///
    /// @return an integer < 0 if the first character that does not match has a lower value in self than in other, 0 if
    /// the contents of both strings are equal, an integer > 0 if the first character that does not match has a greater
    /// value in self than in other
    template <typename T>
    IsStringOrCharArray<T, int64_t> compare(const T& other) const noexcept;

    /// @brief checks if self is equal to rhs
    ///
    /// @param [in] rhs is the iox::cxx::string, std::string, char array or char to compare with self
    ///
    /// @return true if both strings are equal, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> operator==(const T& rhs) const noexcept;

    /// @brief checks if self is not equal to rhs
    ///
    /// @param [in] rhs is the iox::cxx::string, std::string, char array or char to compare with self
    ///
    /// @return true if both strings are not equal, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> operator!=(const T& rhs) const noexcept;

    /// @brief checks if self is less than rhs, in lexicographical order
    ///
    /// @param [in] rhs is the iox::cxx::string, std::string, char array or char to compare with self
    ///
    /// @return true if self is less than rhs, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> operator<(const T& rhs) const noexcept;

    /// @brief checks if self is less than or equal to rhs, in lexicographical order
    ///
    /// @param [in] rhs is the iox::cxx::string, std::string, char array or char to compare with self
    ///
    /// @return true if self is less than or equal to rhs, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> operator<=(const T& rhs) const noexcept;

    /// @brief checks if self is greater than rhs, in lexicographical order
    ///
    /// @param [in] rhs is the iox::cxx::string, std::string, char array or char to compare with self
    ///
    /// @return true if self is greater than rhs, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> operator>(const T& rhs) const noexcept;

    /// @brief checks if self is greater than or equal to rhs, in lexicographical order
    ///
    /// @param [in] rhs is the iox::cxx::string, std::string, char array or char to compare with self
    ///
    /// @return true if self is greater than or equal to rhs, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> operator>=(const T& rhs) const noexcept;

    /// @brief compares self and a char
    ///
    /// @param [in] other is the char to compare with self
    ///
    /// @return an integer < 0 if the first character that does not match has a lower value in self than in other, 0 if
    /// the contents of both strings are equal, an integer > 0 if the first character that does not match has a greater
    /// value in self than in other
    ///
    /// @note the logic is the same as in the other compare method with other treated as a string with size 1
    int64_t compare(char other) const noexcept;

    /// @brief returns a pointer to the char array of self
    ///
    /// @return a pointer to the char array of self
    const char* c_str() const noexcept;

    /// @brief returns the number of characters stored in the string
    ///
    /// @return the number of characters stored in the string
    constexpr uint64_t size() const noexcept;

    /// @brief returns the maximum number of characters that can be stored in the string
    ///
    /// @return the maximum number of characters that can be stored in the string
    static constexpr uint64_t capacity() noexcept;

    /// @brief returns if the string is empty or not
    ///
    /// @return true if size() == 0 otherwise false
    constexpr bool empty() const noexcept;

    /// @brief clears the content of the string
    constexpr void clear() noexcept;

    /// @brief converts the string to a std::string
    ///
    /// @return a std::string with data equivalent to those stored in the string
    // NOLINTNEXTLINE(hicpp-explicit-conversions) @todo iox-#260 remove this conversion and implement toStdString method
    operator std::string() const;

    /// @brief since there are two valid options for what should happen when appending a string larger than this'
    /// capacity (failing or truncating), the fixed string does not support operator+=; use append for truncating or
    /// unsafe_append for failing in that case
    template <typename T>
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter) method is disabled via static_assert
    string& operator+=(const T&) noexcept;

    /// @brief appends a iox::cxx::string/string literal/std::string to the end of this. If this' capacity is too
    /// small for appending the whole string (literal), the remainder of the characters are truncated.
    ///
    /// @param [in] TruncateToCapacity_t is a compile time variable which is used to make the user aware of the possible
    /// truncation
    /// @param [in] str is the iox::cxx::string/string literal/std::string to append
    ///
    /// @return reference to self
    ///
    /// @code
    ///     string<5> fuu("cde");
    ///     fuu.append(TruncateToCapacity, "fgahc");
    /// @endcode
    template <typename T>
    // TruncateToCapacity_t informs how the method behaves when str does not fit into the string
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    IsStringOrCharArrayOrChar<T, string&> append(TruncateToCapacity_t, const T& str) noexcept;

    /// @brief appends a char to the end of this if this' capacity is large enough.
    ///
    /// @param [in] cstr is the char to append
    ///
    /// @return reference to self
    // TruncateToCapacity_t informs how the method behaves when cstr does not fit into the string
    // NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
    string& append(TruncateToCapacity_t, char cstr) noexcept;

    /// @brief appends a iox::cxx::string/string literal/char/std::string to the end of this. The appending fails if the
    /// sum of both sizes is greater than this' capacity.
    ///
    /// @param [in] iox::cxx::string/string literal/char/std::string to append
    ///
    /// @return true if the appending succeeds, otherwise false
    template <typename T>
    IsStringOrCharArrayOrChar<T, bool> unsafe_append(const T& str) noexcept;

    /// @brief inserts a cxx:string or char array in the range [str[0], str[count]) at position pos. The insertion fails
    /// if the string capacity would be exceeded or pos is greater than the string size or count is greater than the
    /// string to be inserted.
    ///
    /// @param [in] pos position at which the string shall be inserted
    /// @param [in] str the cxx::string or char array to be inserted
    /// @param [in] count number of characters to be inserted
    ///
    /// @return true if the insertion was successful, otherwise false
    template <typename T>
    IsCxxStringOrCharArray<T, bool> insert(const uint64_t pos, const T& str, const uint64_t count) noexcept;

    /// @brief creates a substring containing the characters from pos until count; if pos+count is greater than the size
    /// of the original string the returned substring only contains the characters from pos until size();
    /// iox::cxx::nullopt is returned if pos is greater than the size of the original string;
    ///
    /// @param [in] pos is the position of the first character used for the substring
    /// @param [in] count is the requested length of the substring
    ///
    /// @return an optional containing the substring, iox::cxx::nullopt if pos is greater than the size of the original
    /// string
    optional<string<Capacity>> substr(const uint64_t pos, const uint64_t count) const noexcept;

    /// @brief creates a substring containing the characters from pos until size(); iox::cxx::nullopt is returned if pos
    /// is greater than the size of the original string
    ///
    /// @param [in] pos is the position of the first character used for the substring
    ///
    /// @return an optional containing the substring, iox::cxx::nullopt if pos is greater than the size of the original
    /// string
    optional<string<Capacity>> substr(const uint64_t pos = 0U) const noexcept;

    /// @brief finds the first occurence of the given character sequence; returns the position of the first character of
    /// the found substring, returns iox::cxx::nullopt if no substring is found or if pos is greater than this' size
    ///
    /// @param [in] str is the character sequence to search for; must be a cxx::string, string literal or std::string
    /// @param [in] pos is the position at which to start the search
    ///
    /// @return an optional containing the position of the first character of the found substring, iox::cxx::nullopt if
    /// no substring is found
    template <typename T>
    IsStringOrCharArray<T, optional<uint64_t>> find(const T& str, const uint64_t pos = 0U) const noexcept;

    /// @brief finds the first occurence of a character equal to one of the characters of the given character sequence
    /// and returns its position; returns iox::cxx::nullopt if no character is found or if pos is greater than this'
    /// size
    ///
    /// @param [in] str is the character sequence to search for; must be a cxx::string, string literal or std::string
    /// @param [in] pos is the position at which to start the search
    ///
    /// @return an optional containing the position of the first character equal to one of the characters of the given
    /// character sequence, iox::cxx::nullopt if no character is found
    template <typename T>
    IsStringOrCharArray<T, optional<uint64_t>> find_first_of(const T& str, const uint64_t pos = 0U) const noexcept;

    /// @brief finds the last occurence of a character equal to one of the characters of the given character sequence
    /// and returns its position; returns iox::cxx::nullopt if no character is found
    ///
    /// @param [in] str is the character sequence to search for; must be a cxx::string, string literal or std::string
    /// @param [in] pos is the position at which to finish the search
    ///
    /// @return an optional containing the position of the last character equal to one of the characters of the given
    /// character sequence, iox::cxx::nullopt if no character is found
    template <typename T>
    IsStringOrCharArray<T, optional<uint64_t>> find_last_of(const T& str, const uint64_t pos = Capacity) const noexcept;

    /// @brief returns a reference to the character stored at pos
    /// @param[in] pos position of character to return
    /// @return reference to the character
    /// @note out of bounds access leads to program termination
    constexpr char& at(const uint64_t pos) noexcept;

    /// @brief returns a reference to the character stored at pos
    /// @param[in] pos position of character to return
    /// @return const reference to the character
    /// @note out of bounds access leads to program termination
    constexpr const char& at(const uint64_t pos) const noexcept;

    /// @brief returns a reference to the character stored at pos
    /// @param[in] pos position of the character to return
    /// @return reference to the character
    /// @note out of bounds access leads to program termination
    constexpr char& operator[](const uint64_t pos) noexcept;

    /// @brief returns a const reference to the character stored at pos
    /// @param[in] pos position of the character to return
    /// @return const reference to the character
    /// @note out of bounds access leads to program termination
    constexpr const char& operator[](const uint64_t pos) const noexcept;

    template <uint64_t N>
    friend class string;

    template <typename T1, typename T2>
    friend IsCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2>::value>>
    concatenate(const T1& t1, const T2& t2) noexcept;

  private:
    /// @brief copies rhs fixed string to this with compile time check whether rhs capacity is less than or equal to
    /// this' capacity
    ///
    /// @param [in] rhs is the copy origin
    ///
    /// @return reference to self
    template <uint64_t N>
    string& copy(const string<N>& rhs) noexcept;

    /// @brief moves rhs fixed string to this with compile time check whether rhs capacity is less than or equal to
    /// this' capacity
    ///
    /// @param [in] rhs is the move origin
    ///
    /// @return reference to self
    template <uint64_t N>
    string& move(string<N>&& rhs) noexcept;

    // safe access is guaranteed since the char array is wrapped inside the string class
    // NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
    containers::UninitializedArray<char, Capacity + 1U, containers::ZeroedBuffer> m_rawstring;
    uint64_t m_rawstringSize{0U};
};

/// @brief checks if a lhs std::string, char array or char is equal to a rhs iox::cxx::string
///
/// @param [in] rhs is the iox::cxx::string
///
/// @return true if the contents of lhs and rhs are equal, otherwise false
template <typename T, uint64_t Capacity>
IsStdStringOrCharArrayOrChar<T, bool> operator==(const T& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a lhs std::string, char array or char is not equal to a rhs iox::cxx::string
///
/// @param [in] lhs is the std::string, char array or char
/// @param [in] rhs is the iox::cxx::string
///
/// @return true if the contents of lhs and rhs are not equal, otherwise false
template <typename T, uint64_t Capacity>
IsStdStringOrCharArrayOrChar<T, bool> operator!=(const T& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a lhs std::string, char array or char is less than a rhs iox::cxx::string
///
/// @param [in] lhs is the std::string, char array or char
/// @param [in] rhs is the iox::cxx::string
///
/// @return true if lhs is less than rhs, otherwise false
template <typename T, uint64_t Capacity>
IsStdStringOrCharArrayOrChar<T, bool> operator<(const T& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a lhs std::string, char array or char is less than or equal to a rhs iox::cxx::string
///
/// @param [in] lhs is the std::string, char array or char
/// @param [in] rhs is the iox::cxx::string
///
/// @return true if lhs is less than or equal to rhs, otherwise false
template <typename T, uint64_t Capacity>
IsStdStringOrCharArrayOrChar<T, bool> operator<=(const T& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a lhs std::string, char array or char is greater than a rhs iox::cxx::string
///
/// @param [in] lhs is the std::string, char array or char
/// @param [in] rhs is the iox::cxx::string
///
/// @return true if lhs is greater than rhs, otherwise false
template <typename T, uint64_t Capacity>
IsStdStringOrCharArrayOrChar<T, bool> operator>(const T& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a lhs std::string, char array or char is greater than or equal to a rhs iox::cxx::string
///
/// @param [in] lhs is the std::string, char array or char
/// @param [in] rhs is the iox::cxx::string
///
/// @return true if lhs is greater than or equal to rhs, otherwise false
template <typename T, uint64_t Capacity>
IsStdStringOrCharArrayOrChar<T, bool> operator>=(const T& lhs, const string<Capacity>& rhs) noexcept;

/// @brief outputs the fixed string on stream
///
/// @param [in] stream is the output stream
/// @param [in] str is the fixed string
///
/// @return the stream output of the fixed string
template <uint64_t Capacity>
inline std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str) noexcept;
} // namespace cxx
} // namespace iox
#include "iceoryx_hoofs/internal/cxx/string.inl"

#endif // IOX_HOOFS_CXX_STRING_HPP
