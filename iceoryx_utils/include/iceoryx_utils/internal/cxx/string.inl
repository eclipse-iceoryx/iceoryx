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
#ifndef IOX_UTILS_CXX_STRING_INL
#define IOX_UTILS_CXX_STRING_INL

namespace iox
{
namespace cxx
{
template <uint64_t Capacity>
inline constexpr string<Capacity>::string() noexcept
{
}

template <uint64_t Capacity>
inline string<Capacity>::string(const string& other) noexcept
{
    *this = other;
}

template <uint64_t Capacity>
inline string<Capacity>::string(string&& other) noexcept
{
    *this = std::move(other);
}

template <uint64_t Capacity>
inline string<Capacity>& string<Capacity>::operator=(const string& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }
    uint64_t strSize = rhs.size();
    std::memcpy(m_rawstring, rhs.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    return *this;
}

template <uint64_t Capacity>
inline string<Capacity>& string<Capacity>::operator=(string&& rhs) noexcept
{
    if (this == &rhs)
    {
        return *this;
    }
    uint64_t strSize = rhs.size();
    std::memcpy(m_rawstring, rhs.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    rhs.m_rawstring[0] = '\0';
    rhs.m_rawstringSize = 0u;
    return *this;
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>::string(const char (&other)[N]) noexcept
{
    *this = other;
}

template <uint64_t Capacity>
inline string<Capacity>::string(TruncateToCapacity_t, const char* const other) noexcept
    : string(TruncateToCapacity, other, strlen(other))
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
        m_rawstring[0] = '\0';
        m_rawstringSize = 0u;
    }
    else if (Capacity < count)
    {
        std::memcpy(m_rawstring, other, Capacity);
        m_rawstring[Capacity] = '\0';
        m_rawstringSize = Capacity;
        std::cerr << "Constructor truncates the last " << count - Capacity << " characters of " << other
                  << ", because the char array length is larger than the capacity." << std::endl;
    }
    else
    {
        std::memcpy(m_rawstring, other, count);
        m_rawstring[count] = '\0';
        m_rawstringSize = count;
    }
}

template <uint64_t Capacity>
template <uint64_t N>
inline string<Capacity>& string<Capacity>::operator=(const char (&rhs)[N]) noexcept
{
    static_assert((N - 1u) <= Capacity,
                  "Assignment failed. The given char array is larger than the capacity of the fixed string.");

    if (c_str() == rhs)
    {
        return *this;
    }

    m_rawstringSize = strnlen(rhs, Capacity);
    std::memcpy(m_rawstring, rhs, m_rawstringSize);
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
inline string<Capacity>& string<Capacity>::assign(const string& str) noexcept
{
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
    if (c_str() == str)
    {
        return false;
    }
    uint64_t strSize = strlen(str);
    if (Capacity < strSize)
    {
        std::cerr << "Assignment failed. The given cstring is larger than the capacity of the fixed string."
                  << std::endl;
        return false;
    }
    std::memcpy(m_rawstring, str, strSize);
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
    std::memcpy(m_rawstring, str.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    return true;
}

template <uint64_t Capacity>
inline int64_t string<Capacity>::compare(const string other) const noexcept
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
inline bool string<Capacity>::operator==(const string& rhs) const noexcept
{
    return (compare(rhs) == 0);
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator!=(const string& rhs) const noexcept
{
    return (compare(rhs) != 0);
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator<(const string& rhs) const noexcept
{
    return (compare(rhs) < 0);
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator<=(const string& rhs) const noexcept
{
    return !(compare(rhs) > 0);
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator>(const string& rhs) const noexcept
{
    return (compare(rhs) > 0);
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator>=(const string& rhs) const noexcept
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
inline constexpr uint64_t string<Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <uint64_t Capacity>
inline constexpr bool string<Capacity>::empty() const noexcept
{
    return m_rawstringSize == 0;
}

template <uint64_t Capacity>
inline string<Capacity>::operator std::string() const noexcept
{
    return std::string(c_str());
}

template <uint64_t Capacity>
inline bool operator==(const std::string& lhs, const string<Capacity>& rhs)
{
    if (lhs.size() != rhs.size())
    {
        return false;
    }
    return (memcmp(lhs.c_str(), rhs.c_str(), rhs.size()) == 0);
}

template <uint64_t Capacity>
inline bool operator==(const string<Capacity>& lhs, const std::string& rhs)
{
    return (rhs == lhs);
}

template <uint64_t Capacity>
inline bool operator!=(const std::string& lhs, const string<Capacity>& rhs)
{
    return !(lhs == rhs);
}

template <uint64_t Capacity>
inline bool operator!=(const string<Capacity>& lhs, const std::string& rhs)
{
    return (rhs != lhs);
}

template <uint64_t Capacity>
inline std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str)
{
    stream << str.c_str();
    return stream;
}

/// @brief struct used to disable the equality operators for fixed string and char pointer; it is needed, because a
/// simple false will be evaluated before the template is instanciated and therefore the program won't be compiled
template <uint64_t>
struct always_false
{
    static constexpr bool value = false;
};

template <uint64_t Capacity>
inline bool string<Capacity>::operator==(const char* const) const noexcept
{
    static_assert(always_false<Capacity>::value,
                  "The equality operator for fixed string and char pointer is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
inline bool string<Capacity>::operator!=(const char* const) const noexcept
{
    static_assert(always_false<Capacity>::value,
                  "The inequality operator for fixed string and char pointer is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
inline bool operator==(const char* const, const string<Capacity>&)
{
    static_assert(always_false<Capacity>::value,
                  "The equality operator for char pointer and fixed string is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}

template <uint64_t Capacity>
inline bool operator!=(const char* const, const string<Capacity>&)
{
    static_assert(always_false<Capacity>::value,
                  "The inequality operator for char pointer and fixed string is disabled, because it may lead to "
                  "undefined behavior if the char array is not null-terminated. Please convert the char array to a "
                  "fixed string with string(TruncateToCapacity_t, const char* const other, const uint64_t count) "
                  "before comparing it to a fixed string.");
    return false;
}
} // namespace cxx
} // namespace iox

#endif // IOX_UTILS_CXX_STRING_INL
