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
#ifndef IOX_HOOFS_CXX_STRING_INL
#define IOX_HOOFS_CXX_STRING_INL

#include "iceoryx_hoofs/cxx/string.hpp"

namespace iox
{
namespace cxx
{

template <uint64_t Capacity>
inline string<Capacity>::string(const string& other) noexcept
{
    copy(other);
}

template <uint64_t Capacity>
inline string<Capacity>::string(string&& other) noexcept
{
    move(std::move(other));
}

template <uint64_t Capacity>
inline string<Capacity>& string<Capacity>::operator=(const string& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature) copy() returns *this
    return copy(rhs);
}

template <uint64_t Capacity>
inline string<Capacity>& string<Capacity>::operator=(string&& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature) move() returns *this
    return move(std::move(rhs));
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>::string(const string<N>& other) noexcept
{
    static_assert(N <= Capacity,
                  "Construction failed. The capacity of the given fixed string is larger than the capacity of this.");
    copy(other);
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>::string(string<N>&& other) noexcept
{
    static_assert(N <= Capacity,
                  "Construction failed. The capacity of the given fixed string is larger than the capacity of this.");
    move(std::move(other));
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::operator=(const string<N>& rhs) noexcept
{
    static_assert(N <= Capacity,
                  "Assignment failed. The capacity of the given fixed string is larger than the capacity of this.");
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature) copy() returns *this
    return copy(rhs);
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::operator=(string<N>&& rhs) noexcept
{
    static_assert(N <= Capacity,
                  "Assignment failed. The capacity of the given fixed string is larger than the capacity of this.");
    // NOLINTNEXTLINE(cppcoreguidelines-c-copy-assignment-signature) move() returns *this
    return move(std::move(rhs));
}

template <uint64_t Capacity>
template <uint64_t N>
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) cxx::string wraps char array
inline string<Capacity>::string(const char (&other)[N]) noexcept
{
    *this = other;
}

template <uint64_t Capacity>
// TruncateToCapacity_t is a compile time variable to distinguish between constructors
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
inline string<Capacity>::string(TruncateToCapacity_t, const std::string& other) noexcept
    : string(TruncateToCapacity, other.c_str(), other.size())
{
}

template <uint64_t Capacity>
// TruncateToCapacity_t is a compile time variable to distinguish between constructors
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
inline string<Capacity>::string(TruncateToCapacity_t, const char* const other, const uint64_t count) noexcept
{
    if (other == nullptr)
    {
        clear();
    }
    else if (Capacity < count)
    {
#if defined(__GNUC__) && __GNUC__ == 8 && __GNUC_MINOR__ == 3
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Warray-bounds"
#endif
        std::memcpy(&(m_rawstring[0]), other, Capacity);
#if defined(__GNUC__) && __GNUC__ == 8 && __GNUC_MINOR__ == 3
#pragma GCC diagnostic pop
#endif
        m_rawstring[Capacity] = '\0';
        m_rawstringSize = Capacity;
        std::cerr << "Constructor truncates the last " << count - Capacity << " characters of " << other
                  << ", because the char array length is larger than the capacity." << std::endl;
    }
    else
    {
        std::memcpy(&(m_rawstring[0]), other, count);
        m_rawstring[count] = '\0';
        m_rawstringSize = count;
    }
}

template <uint64_t Capacity>
template <uint64_t N>
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) cxx::string wraps char array
inline string<Capacity>& string<Capacity>::operator=(const char (&rhs)[N]) noexcept
{
    static_assert(N <= Capacity + 1U,
                  "Assignment failed. The given char array is larger than the capacity of the fixed string.");

    if (c_str() == rhs)
    {
        return *this;
    }

    std::memcpy(&(m_rawstring[0]), rhs, N);

    m_rawstringSize = std::min(Capacity, static_cast<uint64_t>(strnlen(rhs, N)));
    m_rawstring[m_rawstringSize] = '\0';

    if (rhs[m_rawstringSize] != '\0')
    {
        std::cerr << "iox::cxx::string: Assignment of array which is not zero-terminated! Last value of array "
                     "overwritten with 0!"
                  << std::endl;
    }
    return *this;
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::assign(const string<N>& str) noexcept
{
    static_assert(N <= Capacity,
                  "Assignment failed. The capacity of the given fixed string is larger than the capacity of this.");
    *this = str;
    return *this;
}

template <uint64_t Capacity>
template <uint64_t N>
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays) cxx::string wraps char array
inline string<Capacity>& string<Capacity>::assign(const char (&str)[N]) noexcept
{
    *this = str;
    return *this;
}

template <uint64_t Capacity>
inline bool string<Capacity>::unsafe_assign(const std::string& str) noexcept
{
    uint64_t strSize = str.size();
    if (Capacity < strSize)
    {
        std::cerr << "Assignment failed. The given std::string is larger than the capacity of the fixed string."
                  << std::endl;
        return false;
    }
    std::memcpy(&(m_rawstring[0]), str.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    return true;
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArray<T, int64_t> string<Capacity>::compare(const T& other) const noexcept
{
    uint64_t otherSize = internal::GetSize<T>::call(other);
    auto result = memcmp(c_str(), internal::GetData<T>::call(other), std::min(m_rawstringSize, otherSize));
    if (result == 0)
    {
        if (m_rawstringSize < otherSize)
        {
            return -1;
        }
        return (m_rawstringSize > otherSize ? 1 : 0);
    }
    return result;
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::operator==(const T& rhs) const noexcept
{
    return (compare(rhs) == 0);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::operator!=(const T& rhs) const noexcept
{
    return (compare(rhs) != 0);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::operator<(const T& rhs) const noexcept
{
    return (compare(rhs) < 0);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::operator<=(const T& rhs) const noexcept
{
    return (compare(rhs) <= 0);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::operator>(const T& rhs) const noexcept
{
    return (compare(rhs) > 0);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::operator>=(const T& rhs) const noexcept
{
    return (compare(rhs) >= 0);
}

template <typename T, uint64_t Capacity>
inline IsStdStringOrCharArrayOrChar<T, bool> operator==(const T& lhs, const string<Capacity>& rhs) noexcept
{
    return (rhs.compare(lhs) == 0);
}

template <typename T, uint64_t Capacity>
inline IsStdStringOrCharArrayOrChar<T, bool> operator!=(const T& lhs, const string<Capacity>& rhs) noexcept
{
    return (rhs.compare(lhs) != 0);
}

template <typename T, uint64_t Capacity>
inline IsStdStringOrCharArrayOrChar<T, bool> operator<(const T& lhs, const string<Capacity>& rhs) noexcept
{
    return (rhs.compare(lhs) > 0);
}

template <typename T, uint64_t Capacity>
inline IsStdStringOrCharArrayOrChar<T, bool> operator<=(const T& lhs, const string<Capacity>& rhs) noexcept
{
    return (rhs.compare(lhs) >= 0);
}

template <typename T, uint64_t Capacity>
inline IsStdStringOrCharArrayOrChar<T, bool> operator>(const T& lhs, const string<Capacity>& rhs) noexcept
{
    return (rhs.compare(lhs) < 0);
}

template <typename T, uint64_t Capacity>
inline IsStdStringOrCharArrayOrChar<T, bool> operator>=(const T& lhs, const string<Capacity>& rhs) noexcept
{
    return (rhs.compare(lhs) <= 0);
}

template <uint64_t Capacity>
inline int64_t string<Capacity>::compare(char other) const noexcept
{
    auto result = memcmp(c_str(), &other, 1U);
    if (result == 0)
    {
        if (empty())
        {
            return -1;
        }
        return (m_rawstringSize > 1U ? 1 : 0);
    }
    return result;
}

template <uint64_t Capacity>
inline const char* string<Capacity>::c_str() const noexcept
{
    return &m_rawstring[0];
}

template <uint64_t Capacity>
inline constexpr uint64_t string<Capacity>::size() const noexcept
{
    return m_rawstringSize;
}

template <uint64_t Capacity>
inline constexpr uint64_t string<Capacity>::capacity() noexcept
{
    return Capacity;
}

template <uint64_t Capacity>
inline constexpr bool string<Capacity>::empty() const noexcept
{
    return m_rawstringSize == 0U;
}

template <uint64_t Capacity>
inline constexpr void string<Capacity>::clear() noexcept
{
    m_rawstring[0U] = '\0';
    m_rawstringSize = 0U;
}

template <uint64_t Capacity>
inline string<Capacity>::operator std::string() const
{
    return std::string(c_str());
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::copy(const string<N>& rhs) noexcept
{
    static_assert(N <= Capacity,
                  "Assignment failed. The capacity of the given fixed string is larger than the capacity of this.");
    uint64_t strSize = rhs.size();
    std::memcpy(&(m_rawstring[0]), rhs.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    return *this;
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::move(string<N>&& rhs) noexcept
{
    static_assert(N <= Capacity,
                  "Assignment failed. The capacity of the given fixed string is larger than the capacity of this.");
    uint64_t strSize = rhs.size();
    std::memcpy(&(m_rawstring[0]), rhs.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    rhs.clear();
    return *this;
}

template <uint64_t Capacity>
inline std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str) noexcept
{
    stream << str.c_str();
    return stream;
}

template <uint64_t Capacity>
template <typename T>
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter) method is disabled via static_assert
inline string<Capacity>& string<Capacity>::operator+=(const T&) noexcept
{
    static_assert(cxx::always_false_v<string<Capacity>>,
                  "operator += is not supported by cxx::string, please use append or unsafe_append instead");
    return *this;
}

template <typename T1, typename T2>
inline IsCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2>::value>>
concatenate(const T1& str1, const T2& str2) noexcept
{
    uint64_t size1 = internal::GetSize<T1>::call(str1);
    uint64_t size2 = internal::GetSize<T2>::call(str2);
    using NewStringType = string<internal::SumCapa<T1, T2>::value>;
    NewStringType newString;
    std::memcpy(&(newString.m_rawstring[0]), internal::GetData<T1>::call(str1), size1);
    std::memcpy(&(newString.m_rawstring[0]) + size1, internal::GetData<T2>::call(str2), size2);
    newString.m_rawstring[size1 + size2] = '\0';
    newString.m_rawstringSize = size1 + size2;

    return newString;
}

template <typename T1, typename T2, typename... Targs>
inline IsCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2, Targs...>::value>>
concatenate(const T1& str1, const T2& str2, const Targs&... targs) noexcept
{
    return concatenate(concatenate(str1, str2), targs...);
}

template <typename T1, typename T2>
inline IsCxxStringAndCxxStringOrCharArrayOrChar<T1, T2, string<internal::SumCapa<T1, T2>::value>>
operator+(const T1& str1, const T2& str2) noexcept
{
    return concatenate(str1, str2);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArrayOrChar<T, bool> string<Capacity>::unsafe_append(const T& str) noexcept
{
    uint64_t tSize = internal::GetSize<T>::call(str);
    const char* tData = internal::GetData<T>::call(str);
    uint64_t clampedTSize = std::min(Capacity - m_rawstringSize, tSize);

    if (tSize > clampedTSize)
    {
        std::cerr << "Appending failed because the sum of sizes exceeds this' capacity." << std::endl;
        return false;
    }

    std::memcpy(&(m_rawstring[m_rawstringSize]), tData, clampedTSize);
    m_rawstringSize += clampedTSize;
    m_rawstring[m_rawstringSize] = '\0';
    return true;
}

template <uint64_t Capacity>
template <typename T>
// TruncateToCapacity_t is a compile time variable to distinguish between constructors
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
inline IsStringOrCharArrayOrChar<T, string<Capacity>&> string<Capacity>::append(TruncateToCapacity_t,
                                                                                const T& str) noexcept
{
    uint64_t tSize = internal::GetSize<T>::call(str);
    const char* tData = internal::GetData<T>::call(str);
    uint64_t clampedTSize = std::min(Capacity - m_rawstringSize, tSize);

    std::memcpy(&(m_rawstring[m_rawstringSize]), tData, clampedTSize);
    if (tSize > clampedTSize)
    {
        std::cerr << "The last " << tSize - Capacity + m_rawstringSize << " characters of " << tData
                  << " are truncated, because the length is larger than the capacity." << std::endl;
    }

    m_rawstringSize += clampedTSize;
    m_rawstring[m_rawstringSize] = '\0';
    return *this;
}

template <uint64_t Capacity>
// TruncateToCapacity_t is a compile time variable to distinguish between constructors
// NOLINTNEXTLINE(hicpp-named-parameter, readability-named-parameter)
inline string<Capacity>& string<Capacity>::append(TruncateToCapacity_t, char cstr) noexcept
{
    if (m_rawstringSize == Capacity)
    {
        std::cerr << "Appending of " << cstr << " failed because this' capacity would be exceeded." << std::endl;
        return *this;
    }
    m_rawstring[m_rawstringSize] = cstr;
    m_rawstringSize += 1U;
    m_rawstring[m_rawstringSize] = '\0';
    return *this;
}

template <uint64_t Capacity>
template <typename T>
inline IsCxxStringOrCharArray<T, bool>
string<Capacity>::insert(const uint64_t pos, const T& str, const uint64_t count) noexcept
{
    if (count > internal::GetSize<T>::call(str))
    {
        return false;
    }
    const auto new_size = m_rawstringSize + count;
    // check if the new size would exceed capacity or a size overflow occured
    if (new_size > Capacity || new_size < m_rawstringSize)
    {
        return false;
    }

    if (pos > m_rawstringSize)
    {
        return false;
    }
    std::memmove(&m_rawstring[pos + count], &m_rawstring[pos], m_rawstringSize - pos);
    std::memcpy(&m_rawstring[pos], internal::GetData<T>::call(str), count);

    m_rawstring[new_size] = '\0';
    m_rawstringSize = new_size;

    return true;
}

template <uint64_t Capacity>
inline optional<string<Capacity>> string<Capacity>::substr(const uint64_t pos, const uint64_t count) const noexcept
{
    if (pos > m_rawstringSize)
    {
        return nullopt;
    }

    uint64_t length = std::min(count, m_rawstringSize - pos);
    string subString;
    std::memcpy(&(subString.m_rawstring[0]), &m_rawstring[pos], length);
    subString.m_rawstring[length] = '\0';
    subString.m_rawstringSize = length;
    return subString;
}

template <uint64_t Capacity>
inline optional<string<Capacity>> string<Capacity>::substr(const uint64_t pos) const noexcept
{
    return substr(pos, m_rawstringSize);
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArray<T, optional<uint64_t>> string<Capacity>::find(const T& str,
                                                                         const uint64_t pos) const noexcept
{
    if (pos > m_rawstringSize)
    {
        return nullopt;
    }
    const char* found = std::strstr(c_str() + pos, internal::GetData<T>::call(str));
    if (found == nullptr)
    {
        return nullopt;
    }
    return (static_cast<uint64_t>(found - c_str()));
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArray<T, optional<uint64_t>> string<Capacity>::find_first_of(const T& str,
                                                                                  const uint64_t pos) const noexcept
{
    if (pos > m_rawstringSize)
    {
        return nullopt;
    }
    const char* found = nullptr;
    const char* data = internal::GetData<T>::call(str);
    for (auto p = pos; p < m_rawstringSize; ++p)
    {
        found = std::strchr(data, m_rawstring[p]);
        if (found != nullptr)
        {
            return p;
        }
    }
    return nullopt;
}

template <uint64_t Capacity>
template <typename T>
inline IsStringOrCharArray<T, optional<uint64_t>> string<Capacity>::find_last_of(const T& str,
                                                                                 const uint64_t pos) const noexcept
{
    if (m_rawstringSize == 0U)
    {
        return nullopt;
    }

    auto p = pos;
    if (m_rawstringSize - 1U < p)
    {
        p = m_rawstringSize - 1U;
    }
    const char* found = nullptr;
    const char* data = internal::GetData<T>::call(str);
    for (; p > 0U; --p)
    {
        found = std::strchr(data, m_rawstring[p]);
        if (found != nullptr)
        {
            return p;
        }
    }
    found = std::strchr(data, m_rawstring[p]);
    if (found != nullptr)
    {
        return 0U;
    }
    return nullopt;
}

template <uint64_t Capacity>
inline constexpr char& string<Capacity>::at(const uint64_t pos) noexcept
{
    // const_cast to avoid code duplication, safe since it's first casted to a const type and then the const is removed
    // again
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
    return const_cast<char&>(const_cast<const string<Capacity>*>(this)->at(pos));
}

template <uint64_t Capacity>
inline constexpr const char& string<Capacity>::at(const uint64_t pos) const noexcept
{
    Expects(pos < size() && "Out of bounds access!");
    return m_rawstring[pos];
}

template <uint64_t Capacity>
inline constexpr char& string<Capacity>::operator[](const uint64_t pos) noexcept
{
    return at(pos);
}

template <uint64_t Capacity>
inline constexpr const char& string<Capacity>::operator[](const uint64_t pos) const noexcept
{
    return at(pos);
}
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_STRING_INL
