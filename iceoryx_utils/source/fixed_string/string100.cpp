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

#include "iceoryx_utils/fixed_string/string100.hpp"

namespace iox
{
namespace cxx
{
constexpr uint64_t CString100::MaxStringSize;

CString100::CString100()
    : m_string_vector(MaxStringSize)
{
    m_string_vector[0] = 0; // empty string
}

CString100::CString100(const CString100& str)
    : m_string_vector(MaxStringSize)
{
    *this = str;
}

CString100::CString100(const char* const cstring)
    : m_string_vector(std::min(static_cast<uint64_t>(strlen(cstring) + 1), MaxStringSize),
                      0) /// @todo use cTor with implicit default value once in cxx::vector
{
    uint64_t stringLength = std::min(static_cast<uint64_t>(strlen(cstring) + 1), MaxStringSize);
    for (uint64_t k = 0; k < stringLength; ++k)
    {
        m_string_vector[k] = *(cstring + k);
    }
    m_string_vector[stringLength - 1] = '\0';
}

/// @brief conversion constructor from cstring to CString100. If the
///         cstring contains \0 it will copy this \0 and continue till the
///         full length is copied into the CString100. For a null
///         terminated string conversion use the char* only constructor
CString100::CString100(const char* const cstring, const uint64_t length)
    : m_string_vector(length + 1, 0) /// @todo use cTor with implicit default value once in cxx::vector
{
    for (uint64_t k = 0; k < length; ++k)
    {
        m_string_vector[k] = *(cstring + k);
    }
    m_string_vector[length] = '\0';
}

CString100::CString100(const std::string& str)
    : CString100(str.c_str(), str.size())
{
}

CString100::~CString100()
{
}

//! Assignment operator.
CString100& CString100::operator=(const CString100& str2)
{
    if ((this != &str2))
    {
        m_string_vector = str2.m_string_vector;
    }
    return (*this);
}

bool CString100::operator==(const CString100& other) const
{
    return (compare(other) == 0);
}

bool CString100::operator!=(const CString100& other) const
{
    return !(*this == other);
}

bool CString100::operator<(const CString100& rhs) const
{
    return strncmp(m_string_vector.data(), rhs.m_string_vector.data(), MaxStringSize) < 0;
}

//! <0  the first character that does not match has a lower value in str1 than in str2
// 0 the contents of both strings are equal
// >0  the first character that does not match has a greater value in str1 than in str2
int32_t CString100::compare(const CString100& str2) const
{
    return strncmp(m_string_vector.data(), str2.m_string_vector.data(), MaxStringSize);
}

uint CString100::capacity() const
{
    return MaxStringSize;
}

const char* CString100::to_cstring() const
{
    return m_string_vector.data();
}

CString100::operator const char*() const
{
    return to_cstring();
}

CString100::operator std::string() const
{
    return std::string(to_cstring());
}

} // namespace cxx
} // namespace iox
