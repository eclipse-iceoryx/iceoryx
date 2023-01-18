// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_VOCABULARY_STRING_INTERNAL_HPP
#define IOX_HOOFS_VOCABULARY_STRING_INTERNAL_HPP

#include "iceoryx_hoofs/cxx/attributes.hpp"

#include <cstdint>
#include <cstring>
#include <string>

namespace iox
{
// AXIVION DISABLE STYLE AutosarC++19_03-A3.9.1: Basic numerical type of char shall be used
// AXIVION DISABLE STYLE AutosarC++19_03-A18.1.1: C-style arrays are used to acquire size of c
// array safely, strnlen only accesses N elements which is the maximum capacity of the array
// where N is a compile time constant
template <uint64_t>
class string;

namespace internal
{
template <uint64_t N>
// c array not used here, it is a type alias for easier access
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
using charArray = char[N];

/// @brief struct to get capacity of iox::string/char array/char
/// @note capa is a dummy value for any type other than cxx::string and char
template <typename T>
struct GetCapa
{
    static constexpr uint64_t capa{0U};
};

template <uint64_t N>
struct GetCapa<string<N>>
{
    static constexpr uint64_t capa{N};
};

template <uint64_t N>
// used to acquire char array capacity safely at compile time
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
struct GetCapa<char[N]>
{
    static constexpr uint64_t capa{N - 1U};
};

template <>
struct GetCapa<char>
{
    static constexpr uint64_t capa{1U};
};

/// @brief struct to get size of iox::string/std::string/char array/char
template <typename T>
struct GetSize;

template <uint64_t N>
struct GetSize<string<N>>
{
    static uint64_t call(const string<N>& data) noexcept
    {
        return data.size();
    }
};

template <uint64_t N>
// used to acquire size of c array safely, strnlen only accesses N elements which is the maximum capacity of the array
// where N is a compile time constant
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
struct GetSize<char[N]>
{
    static uint64_t call(const charArray<N>& data) noexcept
    {
        return strnlen(data, N);
    }
};

template <>
struct GetSize<std::string>
{
    static uint64_t call(const std::string& data) noexcept
    {
        return data.size();
    }
};

template <>
struct GetSize<char>
{
    static uint64_t call(char) noexcept
    {
        return 1U;
    }
};

/// @brief struct to get a pointer to the char array of the fixed string/string literal/std::string
template <typename T>
struct GetData;

template <uint64_t N>
struct GetData<string<N>>
{
    static const char* call(const string<N>& data) noexcept
    {
        return data.c_str();
    }
};

template <uint64_t N>
// provides uniform and safe access (in combination with GetCapa and GetSize) to string like constructs like
// cxx::string, std::string, string literal, char
// NOLINTNEXTLINE(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
struct GetData<char[N]>
{
    static const char* call(const charArray<N>& data) noexcept
    {
        return &data[0];
    }
};

template <>
struct GetData<std::string>
{
    static const char* call(const std::string& data) noexcept
    {
        return data.data();
    }
};

template <>
struct GetData<char>
{
    static const char* call(const char& data) noexcept
    {
        // AXIVION Next Construct AutosarC++19_03-A7.5.1 : Used for template meta-programming and
        // safe in this context
        return &data;
    }
};

/// @brief struct to get the sum of the capacities of iox::strings/char arrays/chars
template <typename... Targs>
struct SumCapa;

template <>
struct SumCapa<>
{
    static constexpr uint64_t value{0U};
};

template <typename T, typename... Targs>
struct SumCapa<T, Targs...>
{
    static constexpr uint64_t value{GetCapa<T>::capa + SumCapa<Targs...>::value};
};
} // namespace internal
// AXIVION ENABLE STYLE AutosarC++19_03-A3.9.1
// AXIVION ENABLE STYLE AutosarC++19_03-A18.1.1
} // namespace iox
#endif // IOX_HOOFS_VOCABULARY_STRING_INTERNAL_HPP
