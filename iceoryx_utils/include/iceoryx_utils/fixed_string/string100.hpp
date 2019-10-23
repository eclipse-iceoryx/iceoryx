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

#pragma once

#include "iceoryx_utils/cxx/vector.hpp"

#include <cstdint>
#include <cstring>
#include <iostream>

namespace iox
{
namespace cxx
{
class CString100
{
  private:
    static constexpr uint64_t MaxStringSize = 100;
    using base_t = cxx::vector<char, MaxStringSize>;

  public:
    CString100();
    CString100(const CString100& str);


    /// @brief converts a null terminated cstring into a CString100
    CString100(const char* const cstring);
    /// @brief conversion constructor from cstring to CString100. If the
    ///         cstring contains \0 it will copy this \0 and continue till the
    ///         full length is copied into the CString100. For a null
    ///         terminated string conversion use the char* only constructor
    CString100(const char* const cstring, const uint64_t length);

    CString100(const std::string& str);
    ~CString100();

    //! Assignment operator.
    CString100& operator=(const CString100& str2);
    bool operator==(const CString100& other) const;
    bool operator!=(const CString100& other) const;
    bool operator<(const CString100& rhs) const;

    //! <0  the first character that does not match has a lower value in str1 than in str2
    // 0 the contents of both strings are equal
    // >0  the first character that does not match has a greater value in str1 than in str2
    int32_t compare(const CString100& str2) const;

    uint capacity() const;

    const char* to_cstring() const;

    explicit operator const char*() const;

    operator std::string() const;

  protected:
    base_t m_string_vector;
};

inline bool operator==(const std::string& stdstring, const CString100& cstring100)
{
    return stdstring.compare(cstring100.to_cstring()) == 0;
}

inline bool operator!=(const std::string& stdstring, const CString100& cstring100)
{
    return stdstring.compare(cstring100.to_cstring()) != 0;
}

} // namespace cxx
} // namespace iox

inline std::ostream& operator<<(std::ostream& stream, const iox::cxx::CString100 string)
{
    stream << string.to_cstring();
    return stream;
}

