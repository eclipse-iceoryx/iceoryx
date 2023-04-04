// Copyright (c) 2022 - 2023 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_DUST_STD_STRING_SUPPORT_HPP
#define IOX_DUST_STD_STRING_SUPPORT_HPP

#include "iox/into.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"

#include <ostream>
#include <string>

namespace iox
{

template <>
struct is_custom_string<std::string> : public std::true_type
{
};

namespace internal
{
/// @brief struct to get a pointer to the char array of the std::string
template <>
struct GetData<std::string>
{
    static const char* call(const std::string& data) noexcept
    {
        return data.data();
    }
};

/// @brief struct to get size of a std::string
template <>
struct GetSize<std::string>
{
    static uint64_t call(const std::string& data) noexcept
    {
        return data.size();
    }
};
} // namespace internal

template <uint64_t N>
struct FromImpl<string<N>, std::string>
{
    static std::string fromImpl(const string<N>& value) noexcept;
};

template <uint64_t N>
struct FromImpl<std::string, string<N>>
{
    static string<N> fromImpl(const std::string& value) noexcept;
};

template <uint64_t N>
struct FromImpl<std::string, optional<string<N>>>
{
    static optional<string<N>> fromImpl(const std::string& value) noexcept;
};

template <uint64_t N>
struct FromImpl<std::string, lossy<string<N>>>
{
    static string<N> fromImpl(const std::string& value) noexcept;
};

/// @brief outputs the fixed string on stream
///
/// @param [in] stream is the output stream
/// @param [in] str is the fixed string
///
/// @return the stream output of the fixed string
template <uint64_t Capacity>
std::ostream& operator<<(std::ostream& stream, const string<Capacity>& str) noexcept;
} // namespace iox

#include "iceoryx_dust/internal/cxx/std_string_support.inl"

#endif // IOX_DUST_STD_STRING_SUPPORT_HPP
