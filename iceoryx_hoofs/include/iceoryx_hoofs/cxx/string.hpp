// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/cxx/type_traits.hpp"
#include "iceoryx_hoofs/internal/cxx/string_internal.hpp"
#include "optional.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>

namespace iox
{
namespace cxx
{
/// @brief concatenates two fixed strings/string literals
///
/// @param [in] fixed strings/string literals to concatenate
///
/// @return a new fixed string with capacity equal to the sum of the capacities of the concatenated strings
///
/// @code
///     string<5> fuu("cdefg");
///     auto bar = iox::cxx::concatenate(fuu, "ahc");
/// @endcode
template <typename T1, typename T2>
typename std::enable_if<(internal::IsCharArray<T1>::value || internal::IsCxxString<T1>::value)
                            && (internal::IsCharArray<T2>::value || internal::IsCxxString<T2>::value),
                        string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa>>::type
concatenate(const T1& t1, const T2& t2) noexcept;

/// @brief concatenates an arbitrary number of fixed strings or string literals
///
/// @param [in] fixed strings/string literals to concatenate
///
/// @return a new fixed string with capacity equal to the sum of the capacities of the concatenated strings
///
/// @code
///     string<4> fuu("cdef");
///     auto bar = iox::cxx::concatenate(fuu, "g", "ah", fuu);
/// @endcode
template <typename T1, typename T2, typename... Targs>
typename std::enable_if<(internal::IsCharArray<T1>::value || internal::IsCxxString<T1>::value)
                            && (internal::IsCharArray<T2>::value || internal::IsCxxString<T2>::value),
                        string<internal::SumCapa<T1, T2, Targs...>::value>>::type
concatenate(const T1& t1, const T2& t2, const Targs&... targs) noexcept;

/// @brief concatenates two fixed strings or one fixed fixed string and one string literal; concatenation of two string
/// literals is not possible
///
/// @param [in] fixed strings/string literal to concatenate
///
/// @return a new fixed string with capacity equal to the sum of the capacities of the concatenated strings
template <typename T1, typename T2>
typename std::enable_if<(internal::IsCharArray<T1>::value && internal::IsCxxString<T2>::value)
                            || (internal::IsCxxString<T1>::value && internal::IsCharArray<T2>::value)
                            || (internal::IsCxxString<T1>::value && internal::IsCxxString<T2>::value),
                        string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa>>::type
operator+(const T1& t1, const T2& t2) noexcept;

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

    /// @brief creates a new string of given capacity as a copy of other with compile time check whether the capacity of
    /// other is less than or equal to this' capacity
    ///
    /// @param [in] other is the copy origin
    template <uint64_t N>
    string(const string<N>& other) noexcept;

    /// @brief moves other to this with compile time check whether the capacity of other is less than or equal to
    /// this' capacity
    ///
    /// @param [in] other is the move origin
    template <uint64_t N>
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
    string(const char (&other)[N]) noexcept;

    /// @brief conversion constructor for cstring to string which truncates characters if the size is greater than
    /// the string capacity
    ///
    /// @param [in] TruncateToCapacity_t is a compile time variable which is used to distinguish between
    /// constructors with certain behavior
    /// @param [in] other is the cstring to convert
    /// @attention truncates characters if the size is greater than the string capacity
    ///
    /// @code
    ///     #include "iceoryx_hoofs/cxx/string.hpp"
    ///     using namespace iox::cxx;
    ///
    ///     int main()
    ///     {
    ///         string<4> fuu(TruncateToCapacity, "abcd");
    ///     }
    /// @endcode
    string(TruncateToCapacity_t, const char* const other) noexcept;

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
    string(TruncateToCapacity_t, const std::string& other) noexcept;

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
    string& assign(const char (&str)[N]) noexcept;

    /// @brief assigns a cstring to string. The assignment fails if the cstring size is greater than the string
    /// capacity.
    ///
    /// @param [in] str is the cstring to assign
    ///
    /// @return true if the assignment succeeds, otherwise false
    bool unsafe_assign(const char* const str) noexcept;

    /// @brief assigns a std::string to string. The assignment fails if the std::string size is greater than the string
    /// capacity.
    ///
    /// @param [in] str is the std::string to assign
    ///
    /// @return true if the assignment succeeds, otherwise false
    bool unsafe_assign(const std::string& str) noexcept;

    /// @brief compares two strings
    ///
    /// @param [in] other is the string to compare with self
    ///
    /// @return an integer < 0 if the first character that does not match has a lower value in self than in other, 0 if
    /// the contents of both strings are equal, an integer > 0 if the first character that does not match has a greater
    /// value in self than in other
    template <uint64_t N>
    int64_t compare(const string<N>& other) const noexcept;

    /// @brief checks if self is equal to rhs
    ///
    /// @param [in] rhs is the string to compare with self
    ///
    /// @return true if both strings are equal, otherwise false
    template <uint64_t N>
    bool operator==(const string<N>& rhs) const noexcept;

    /// @brief checks if self is not equal to rhs
    ///
    /// @param [in] rhs is the string to compare with self
    ///
    /// @return true if both strings are not equal, otherwise false
    template <uint64_t N>
    bool operator!=(const string<N>& rhs) const noexcept;

    /// @brief checks if self is less than rhs, in lexicographical order
    ///
    /// @param [in] rhs is the string to compare with self
    ///
    /// @return true if self is less than rhs, otherwise false
    template <uint64_t N>
    bool operator<(const string<N>& rhs) const noexcept;

    /// @brief checks if self is less than or equal to rhs, in lexicographical order
    ///
    /// @param [in] rhs is the string to compare with self
    ///
    /// @return true if self is less than or equal to rhs, otherwise false
    template <uint64_t N>
    bool operator<=(const string<N>& rhs) const noexcept;

    /// @brief checks if self is greater than rhs, in lexicographical order
    ///
    /// @param [in] rhs is the string to compare with self
    ///
    /// @return true if self is greater than rhs, otherwise false
    template <uint64_t N>
    bool operator>(const string<N>& rhs) const noexcept;

    /// @brief checks if self is greater than or equal to rhs, in lexicographical order
    ///
    /// @param [in] rhs is the string to compare with self
    ///
    /// @return true if self is greater than or equal to rhs, otherwise false
    template <uint64_t N>
    bool operator>=(const string<N>& rhs) const noexcept;

    /// @brief The equality operator for fixed string and char pointer is disabled via a static_assert, because it may
    /// lead to undefined behavior if the char array is not null-terminated. Please convert the char array to a fixed
    /// string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) before compare it to a
    /// fixed string.
    ///
    /// @param [in] rhs is the char pointer to the array to compare
    ///
    /// @return false
    ///
    /// @todo consider implementing the equality operator for a char array for which the size is known at compile time;
    /// it could have the following signature
    /// template <int N>
    /// bool operator==(const char (&rhs)[N]) const noexcept
    bool operator==(const char* const rhs) const noexcept;

    /// @brief The inequality operator for fixed string and char pointer is disabled via a static_assert, because it may
    /// lead to undefined behavior if the char array is not null-terminated. Please convert the char array to a fixed
    /// string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) before compare it to a
    /// fixed string.
    ///
    /// @param [in] rhs is the char pointer to the array to compare
    ///
    /// @return false
    ///
    /// @todo consider implementing the inequality operator for a char array for which the size is known at compile
    /// time; it could have the following signature
    /// template <int N>
    /// bool operator!=(const char (&rhs)[N]) const noexcept
    bool operator!=(const char* const rhs) const noexcept;

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

    /// @brief converts the string to a std::string
    ///
    /// @return a std::string with data equivalent to those stored in the string
    operator std::string() const noexcept;

    /// @brief since there are two valid options for what should happen when appending a string larger than this'
    /// capacity (failing or truncating), the fixed string does not support operator+=; use append for truncating or
    /// unsafe_append for failing in that case
    template <typename T>
    string& operator+=(const T&) noexcept;

    /// @brief appends a fixed string or string literal to the end of this. If this' capacity is too small for appending
    /// the whole string (literal) the remainder of the characters are truncated.
    ///
    /// @param [in] TruncateToCapacity_t is a compile time variable which is used to make the user aware of the possible
    /// truncation
    /// @param [in] t is the fixed string/string literal to append
    ///
    /// @return reference to self
    ///
    /// @code
    ///     string<5> fuu("cde");
    ///     fuu.append(TruncateToCapacity, "fgahc");
    /// @endcode
    template <typename T>
    typename std::enable_if<internal::IsCharArray<T>::value || internal::IsCxxString<T>::value, string&>::type
    append(TruncateToCapacity_t, const T& t) noexcept;

    /// @brief appends a fixed string or string literal to the end of this. The appending fails if the sum of both sizes
    /// is greater than this' capacity.
    ///
    /// @param [in] fixed string/string literal to append
    ///
    /// @return true if the appending succeeds, otherwise false
    template <typename T>
    typename std::enable_if<internal::IsCharArray<T>::value || internal::IsCxxString<T>::value, bool>::type
    unsafe_append(const T& t) noexcept;

    /// @brief creates a substring containing the characters from pos until count; if pos+count is greater than the size
    /// of the original string the returned substring only contains the characters from pos until size();
    /// iox::cxx::nullopt is returned if pos is greater than the size of the original string;
    ///
    /// @param [in] pos is the position of the first character used for the substring
    /// @param [in] count is the requested length of the substring
    ///
    /// @return an optional containing the substring, iox::cxx::nullopt if pos is greater than the size of the original
    /// string
    iox::cxx::optional<string<Capacity>> substr(const uint64_t pos, const uint64_t count) const noexcept;

    /// @brief creates a substring containing the characters from pos until size(); iox::cxx::nullopt is returned if pos
    /// is greater than the size of the original string
    ///
    /// @param [in] pos is the position of the first character used for the substring
    ///
    /// @return an optional containing the substring, iox::cxx::nullopt if pos is greater than the size of the original
    /// string
    iox::cxx::optional<string<Capacity>> substr(const uint64_t pos = 0U) const noexcept;

    /// @brief finds the first occurence of the given character sequence; returns the position of the first character of
    /// the found substring, returns iox::cxx::nullopt if no substring is found or if pos is greater than this' size
    ///
    /// @param [in] t is the character sequence to search for; must be a cxx::string, string literal or std::string
    /// @param [in] pos is the position at which to start the search
    ///
    /// @return an optional containing the position of the first character of the found substring, iox::cxx::nullopt if
    /// no substring is found
    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value || internal::IsCharArray<T>::value
                                || internal::IsCxxString<T>::value,
                            iox::cxx::optional<uint64_t>>::type
    find(const T& t, const uint64_t pos = 0U) const noexcept;

    /// @brief finds the first occurence of a character equal to one of the characters of the given character sequence
    /// and returns its position; returns iox::cxx::nullopt if no character is found or if pos is greater than this'
    /// size
    ///
    /// @param [in] t is the character sequence to search for; must be a cxx::string, string literal or std::string
    /// @param [in] pos is the position at which to start the search
    ///
    /// @return an optional containing the position of the first character equal to one of the characters of the given
    /// character sequence, iox::cxx::nullopt if no character is found
    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value || internal::IsCharArray<T>::value
                                || internal::IsCxxString<T>::value,
                            iox::cxx::optional<uint64_t>>::type
    find_first_of(const T& t, const uint64_t pos = 0U) const noexcept;

    /// @brief finds the last occurence of a character equal to one of the characters of the given character sequence
    /// and returns its position; returns iox::cxx::nullopt if no character is found
    ///
    /// @param [in] t is the character sequence to search for; must be a cxx::string, string literal or std::string
    /// @param [in] pos is the position at which to finish the search
    ///
    /// @return an optional containing the position of the last character equal to one of the characters of the given
    /// character sequence, iox::cxx::nullopt if no character is found
    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value || internal::IsCharArray<T>::value
                                || internal::IsCxxString<T>::value,
                            iox::cxx::optional<uint64_t>>::type
    find_last_of(const T& t, const uint64_t pos = Capacity) const noexcept;

    template <uint64_t N>
    friend class string;

    template <typename T1, typename T2>
    friend typename std::enable_if<(internal::IsCharArray<T1>::value || internal::IsCxxString<T1>::value)
                                       && (internal::IsCharArray<T2>::value || internal::IsCxxString<T2>::value),
                                   string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa>>::type
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

    char m_rawstring[Capacity + 1U]{'\0'};
    uint64_t m_rawstringSize{0U};
};

/// @brief checks if a rhs fixed string is equal to a lhs std::string
///
/// @param [in] lhs is the std::string
/// @param [in] rhs is the fixed string
///
/// @return true if both strings are equal, otherwise false
template <uint64_t Capacity>
inline bool operator==(const std::string& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a rhs std::string is equal to a lhs fixed string
///
/// @param [in] lhs is the fixed string
/// @param [in] rhs is the std::string
///
/// @return true if both strings are equal, otherwise false
template <uint64_t Capacity>
inline bool operator==(const string<Capacity>& lhs, const std::string& rhs) noexcept;

/// @brief checks if a rhs fixed string is not equal to a lhs std::string
///
/// @param [in] lhs is the std::string
/// @param [in] rhs is the fixed string
///
/// @return true if both strings are not equal, otherwise false
template <uint64_t Capacity>
inline bool operator!=(const std::string& lhs, const string<Capacity>& rhs) noexcept;

/// @brief checks if a rhs std::string is not equal to a lhs fixed string
///
/// @param [in] lhs is the fixed string
/// @param [in] rhs is the std::string
///
/// @return true if both strings are not equal, otherwise false
template <uint64_t Capacity>
inline bool operator!=(const string<Capacity>& lhs, const std::string& rhs) noexcept;

/// @brief The equality operator for char pointer and fixed string is disabled via a static_assert, because it may
/// lead to undefined behavior if the char array is not null-terminated. Please convert the char array to a fixed
/// string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) before compare it to a
/// fixed string.
///
/// @param [in] lhs is the char pointer to the array to compare
/// @param [in] rhs is the fixed string
///
/// @return false
template <uint64_t Capacity>
inline bool operator==(const char* const lhs, const string<Capacity>& rhs) noexcept;

/// @brief The inequality operator for char pointer and fixed string is disabled via a static_assert, because it may
/// lead to undefined behavior if the char array is not null-terminated. Please convert the char array to a fixed
/// string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) before compare it to a
/// fixed string.
///
/// @param [in] lhs is the char pointer to the array to compare
/// @param [in] rhs is the fixed string
///
/// @return false
template <uint64_t Capacity>
inline bool operator!=(const char* const lhs, const string<Capacity>& rhs) noexcept;

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
