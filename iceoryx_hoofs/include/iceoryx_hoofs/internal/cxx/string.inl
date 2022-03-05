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
    return copy(rhs);
}

template <uint64_t Capacity>
inline string<Capacity>& string<Capacity>::operator=(string&& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }
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
    return copy(rhs);
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::operator=(string<N>&& rhs) noexcept
{
    static_assert(N <= Capacity,
                  "Assignment failed. The capacity of the given fixed string is larger than the capacity of this.");
    return move(std::move(rhs));
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>::string(const char (&other)[N]) noexcept
{
    *this = other;
}

template <uint64_t Capacity>
inline string<Capacity>::string(TruncateToCapacity_t, const char* const other) noexcept
    : string(TruncateToCapacity, other, [&]() -> uint64_t { return other ? strnlen(other, Capacity) : 0U; }())
{
}

template <uint64_t Capacity>
inline string<Capacity>::string(TruncateToCapacity_t, const std::string& other) noexcept
    : string(TruncateToCapacity, other.c_str(), other.size())
{
}

template <uint64_t Capacity>
inline string<Capacity>::string(TruncateToCapacity_t, const char* const other, const uint64_t count) noexcept
{
    if (other == nullptr)
    {
        m_rawstring[0U] = '\0';
        m_rawstringSize = 0U;
    }
    else if (Capacity < count)
    {
        std::memcpy(&(m_rawstring[0]), other, Capacity);
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
inline string<Capacity>& string<Capacity>::assign(const char (&str)[N]) noexcept
{
    *this = str;
    return *this;
}

template <uint64_t Capacity>
inline bool string<Capacity>::unsafe_assign(const char* const str) noexcept
{
    if ((c_str() == str) || (str == nullptr))
    {
        return false;
    }
    const uint64_t strSize = strnlen(str, Capacity + 1U);
    if (Capacity < strSize)
    {
        std::cerr << "Assignment failed. The given cstring is larger (" << strSize << ") than the capacity ("
                  << Capacity << ") of the fixed string." << std::endl;
        return false;
    }
    std::memcpy(&(m_rawstring[0]), str, strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    return true;
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
template <uint64_t N>
inline int64_t string<Capacity>::compare(const string<N>& other) const noexcept
{
    uint64_t otherSize = other.size();
    if (m_rawstringSize < otherSize)
    {
        return -1;
    }
    else if (m_rawstringSize > otherSize)
    {
        return 1;
    }
    return memcmp(c_str(), other.c_str(), m_rawstringSize);
}

template <uint64_t Capacity>
template <uint64_t N>
inline bool string<Capacity>::operator==(const string<N>& rhs) const noexcept
{
    return (compare(rhs) == 0);
}

template <uint64_t Capacity>
template <uint64_t N>
inline bool string<Capacity>::operator!=(const string<N>& rhs) const noexcept
{
    return (compare(rhs) != 0);
}

template <uint64_t Capacity>
template <uint64_t N>
inline bool string<Capacity>::operator<(const string<N>& rhs) const noexcept
{
    return (compare(rhs) < 0);
}

template <uint64_t Capacity>
template <uint64_t N>
inline bool string<Capacity>::operator<=(const string<N>& rhs) const noexcept
{
    return !(compare(rhs) > 0);
}

template <uint64_t Capacity>
template <uint64_t N>
inline bool string<Capacity>::operator>(const string<N>& rhs) const noexcept
{
    return (compare(rhs) > 0);
}

template <uint64_t Capacity>
template <uint64_t N>
inline bool string<Capacity>::operator>=(const string<N>& rhs) const noexcept
{
    return !(compare(rhs) < 0);
}

template <uint64_t Capacity>
inline const char* string<Capacity>::c_str() const noexcept
{
    return m_rawstring;
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
inline string<Capacity>::operator std::string() const noexcept
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
    rhs.m_rawstring[0U] = '\0';
    rhs.m_rawstringSize = 0U;
    return *this;
}

template <uint64_t Capacity>
inline bool operator==(const std::string& lhs, const string<Capacity>& rhs) noexcept
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    return (memcmp(lhs.c_str(), rhs.c_str(), rhs.size()) == 0);
}

template <uint64_t Capacity>
inline bool operator==(const string<Capacity>& lhs, const std::string& rhs) noexcept
{
    return (rhs == lhs);
}

template <uint64_t Capacity>
inline bool operator!=(const std::string& lhs, const string<Capacity>& rhs) noexcept
{
    return !(lhs == rhs);
}

template <uint64_t Capacity>
inline bool operator!=(const string<Capacity>& lhs, const std::string& rhs) noexcept
{
    return (rhs != lhs);
}

template <uint64_t Capacity>
inline std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str) noexcept
{
    stream << str.c_str();
    return stream;
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator==(const char* const) const noexcept
{
    static_assert(cxx::always_false_v<string<Capacity>>,
                  "The equality operator for fixed string and char pointer is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator!=(const char* const) const noexcept
{
    static_assert(cxx::always_false_v<string<Capacity>>,
                  "The inequality operator for fixed string and char pointer is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
inline bool operator==(const char* const, const string<Capacity>&) noexcept
{
    static_assert(cxx::always_false_v<string<Capacity>>,
                  "The equality operator for char pointer and fixed string is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
inline bool operator!=(const char* const, const string<Capacity>&) noexcept
{
    static_assert(cxx::always_false_v<string<Capacity>>,
                  "The inequality operator for char pointer and fixed string is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
template <typename T>
inline string<Capacity>& string<Capacity>::operator+=(const T&) noexcept
{
    static_assert(cxx::always_false_v<string<Capacity>>,
                  "operator += is not supported by cxx::string, please use append or unsafe_append instead");
    return *this;
}

template <typename T1, typename T2>
inline typename std::enable_if<(internal::IsCharArray<T1>::value || internal::IsCxxString<T1>::value)
                                   && (internal::IsCharArray<T2>::value || internal::IsCxxString<T2>::value),
                               string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa>>::type
concatenate(const T1& t1, const T2& t2) noexcept
{
    uint64_t size1 = internal::GetSize<T1>::call(t1);
    uint64_t size2 = internal::GetSize<T2>::call(t2);
    using NewStringType = string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa>;
    NewStringType newString;
    std::memcpy(&(newString.m_rawstring[0]), internal::GetData<T1>::call(t1), size1);
    std::memcpy(&(newString.m_rawstring[0]) + size1, internal::GetData<T2>::call(t2), size2);
    newString.m_rawstring[size1 + size2] = '\0';
    newString.m_rawstringSize = size1 + size2;

    return newString;
}

template <typename T1, typename T2, typename... Targs>
inline typename std::enable_if<(internal::IsCharArray<T1>::value || internal::IsCxxString<T1>::value)
                                   && (internal::IsCharArray<T2>::value || internal::IsCxxString<T2>::value),
                               string<internal::SumCapa<T1, T2, Targs...>::value>>::type
concatenate(const T1& t1, const T2& t2, const Targs&... targs) noexcept
{
    return concatenate(concatenate(t1, t2), targs...);
}

template <typename T1, typename T2>
inline typename std::enable_if<(internal::IsCharArray<T1>::value && internal::IsCxxString<T2>::value)
                                   || (internal::IsCxxString<T1>::value && internal::IsCharArray<T2>::value)
                                   || (internal::IsCxxString<T1>::value && internal::IsCxxString<T2>::value),
                               string<internal::GetCapa<T1>::capa + internal::GetCapa<T2>::capa>>::type
operator+(const T1& t1, const T2& t2) noexcept
{
    return concatenate(t1, t2);
}

template <uint64_t Capacity>
template <typename T>
inline typename std::enable_if<internal::IsCharArray<T>::value || internal::IsCxxString<T>::value, bool>::type
string<Capacity>::unsafe_append(const T& t) noexcept
{
    uint64_t tSize = internal::GetSize<T>::call(t);
    const char* tData = internal::GetData<T>::call(t);
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
inline
    typename std::enable_if<internal::IsCharArray<T>::value || internal::IsCxxString<T>::value, string<Capacity>&>::type
    string<Capacity>::append(TruncateToCapacity_t, const T& t) noexcept
{
    uint64_t tSize = internal::GetSize<T>::call(t);
    const char* tData = internal::GetData<T>::call(t);
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
inline iox::cxx::optional<string<Capacity>> string<Capacity>::substr(const uint64_t pos,
                                                                     const uint64_t count) const noexcept
{
    if (pos > m_rawstringSize)
    {
        return iox::cxx::nullopt;
    }

    uint64_t length = std::min(count, m_rawstringSize - pos);
    string subString;
    std::memcpy(&(subString.m_rawstring[0]), &m_rawstring[pos], length);
    subString.m_rawstring[length] = '\0';
    subString.m_rawstringSize = length;
    return subString;
}

template <uint64_t Capacity>
inline iox::cxx::optional<string<Capacity>> string<Capacity>::substr(const uint64_t pos) const noexcept
{
    return substr(pos, m_rawstringSize);
}

template <uint64_t Capacity>
template <typename T>
inline typename std::enable_if<std::is_same<T, std::string>::value || internal::IsCharArray<T>::value
                                   || internal::IsCxxString<T>::value,
                               iox::cxx::optional<uint64_t>>::type
string<Capacity>::find(const T& t, const uint64_t pos) const noexcept
{
    if (pos > m_rawstringSize)
    {
        return iox::cxx::nullopt;
    }
    const char* found = std::strstr(c_str() + pos, internal::GetData<T>::call(t));
    if (found == nullptr)
    {
        return iox::cxx::nullopt;
    }
    return (found - c_str());
}

template <uint64_t Capacity>
template <typename T>
inline typename std::enable_if<std::is_same<T, std::string>::value || internal::IsCharArray<T>::value
                                   || internal::IsCxxString<T>::value,
                               iox::cxx::optional<uint64_t>>::type
string<Capacity>::find_first_of(const T& t, const uint64_t pos) const noexcept
{
    if (pos > m_rawstringSize)
    {
        return iox::cxx::nullopt;
    }
    const char* found = nullptr;
    const char* data = internal::GetData<T>::call(t);
    for (auto p = pos; p < m_rawstringSize; ++p)
    {
        found = std::strchr(data, m_rawstring[p]);
        if (found != nullptr)
        {
            return p;
        }
    }
    return iox::cxx::nullopt;
}

template <uint64_t Capacity>
template <typename T>
inline typename std::enable_if<std::is_same<T, std::string>::value || internal::IsCharArray<T>::value
                                   || internal::IsCxxString<T>::value,
                               iox::cxx::optional<uint64_t>>::type
string<Capacity>::find_last_of(const T& t, const uint64_t pos) const noexcept
{
    if (m_rawstringSize == 0U)
    {
        return iox::cxx::nullopt;
    }

    auto p = pos;
    if (m_rawstringSize - 1U < p)
    {
        p = m_rawstringSize - 1U;
    }
    const char* found = nullptr;
    const char* data = internal::GetData<T>::call(t);
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
    return iox::cxx::nullopt;
}
} // namespace cxx
} // namespace iox

#endif // IOX_HOOFS_CXX_STRING_INL
