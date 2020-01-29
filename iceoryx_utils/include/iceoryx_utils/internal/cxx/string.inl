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

namespace iox
{
namespace cxx
{
template <uint64_t Capacity>
inline string<Capacity>::string() noexcept
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
    size_t strSize = rhs.size();
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
    size_t strSize = rhs.size();
    std::memcpy(m_rawstring, rhs.c_str(), strSize);
    m_rawstring[strSize] = '\0';
    m_rawstringSize = strSize;
    rhs.m_rawstring[0] = '\0';
    rhs.m_rawstringSize = 0;
    return *this;
}

template <uint64_t Capacity>
inline string<Capacity>::~string() noexcept
{
}

template <uint64_t Capacity>
template <int N>
inline string<Capacity>::string(const char (&other)[N]) noexcept
{
    *this = other;
}

template <uint64_t Capacity>
inline string<Capacity>::string(UnsafeCheckPreconditions_t, const char* const other) noexcept
    : string(UnsafeCheckPreconditions, other, strlen(other))
{
}

template <uint64_t Capacity>
inline string<Capacity>::string(UnsafeCheckPreconditions_t, const std::string& other) noexcept
    : string(UnsafeCheckPreconditions, other.c_str(), other.size())
{
}

template <uint64_t Capacity>
inline string<Capacity>::string(UnsafeCheckPreconditions_t, const char* const other, const uint64_t count) noexcept
{
    if (Capacity < count)
    {
        std::memcpy(m_rawstring, other, Capacity);
        m_rawstring[Capacity] = '\0';
        m_rawstringSize = Capacity;
        std::cerr << "Conversion constructor truncates the last " << count - Capacity
                  << " characters, because the char array length is larger than the capacity." << std::endl;
    }
    else
    {
        std::memcpy(m_rawstring, other, count);
        m_rawstring[count] = '\0';
        m_rawstringSize = count;
    }
}

template <uint64_t Capacity>
template <int N>
inline string<Capacity>& string<Capacity>::operator=(const char (&rhs)[N]) noexcept
{
    static_assert((N - 1) <= Capacity,
                  "Assignment failed. The given char array is larger than the capacity of the fixed string.");

    if (c_str() == rhs)
    {
        return *this;
    }
    std::memcpy(m_rawstring, rhs, N - 1);
    m_rawstring[N - 1] = '\0';
    m_rawstringSize = N - 1;
    return *this;
}

template <uint64_t Capacity>
inline string<Capacity>& string<Capacity>::assign(const string& str) noexcept
{
    *this = str;
    return *this;
}

template <uint64_t Capacity>
template <int N>
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
    size_t strSize = strlen(str);
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
    size_t strSize = str.size();
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
inline int string<Capacity>::compare(const string other) const noexcept
{
    return strncmp(c_str(), other.c_str(), Capacity);
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
inline uint64_t string<Capacity>::size() const noexcept
{
    return m_rawstringSize;
}

template <uint64_t Capacity>
inline uint64_t string<Capacity>::capacity() const noexcept
{
    return Capacity;
}

template <uint64_t Capacity>
inline string<Capacity>::operator std::string() const noexcept
{
    return std::string(c_str());
}
} // namespace cxx
} // namespace iox
