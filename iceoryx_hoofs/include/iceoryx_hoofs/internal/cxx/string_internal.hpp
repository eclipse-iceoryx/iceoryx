// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_STRING_INTERNAL_HPP
#define IOX_HOOFS_CXX_STRING_INTERNAL_HPP

#include <cstdint>
#include <cstring>
#include <string>

namespace iox
{
namespace cxx
{
template <uint64_t>
class string;

namespace internal
{
template <uint64_t N>
using charArray = char[N];

/// @brief struct to get capacity of fixed string/string literal
/// @note capa is a dummy value for any type other than cxx::string and char
template <typename T>
struct GetCapa
{
    static constexpr uint64_t capa = 0U;
};

template <uint64_t N>
struct GetCapa<string<N>>
{
    static constexpr uint64_t capa = N;
};

template <uint64_t N>
struct GetCapa<char[N]>
{
    static constexpr uint64_t capa = N - 1U;
};

/// @brief struct to get size of fixed string/string literal/std::string
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

/// @brief struct to get the sum of the capacities of fixed strings/string literals
template <typename... Targs>
struct SumCapa;

template <>
struct SumCapa<>
{
    static constexpr uint64_t value = 0U;
};

template <typename T, typename... Targs>
struct SumCapa<T, Targs...>
{
    static constexpr uint64_t value = GetCapa<T>::capa + SumCapa<Targs...>::value;
};

/// @brief struct to check whether an argument is a char array
template <typename T>
struct IsCharArray
{
    static constexpr bool value = false;
};

template <uint64_t N>
struct IsCharArray<char[N]>
{
    static constexpr bool value = true;
};

/// @brief struct to check whether an argument is a cxx string
template <typename T>
struct IsCxxString
{
    static constexpr bool value = false;
};

template <uint64_t N>
struct IsCxxString<string<N>>
{
    static constexpr bool value = true;
};
} // namespace internal
} // namespace cxx
} // namespace iox
#endif // IOX_HOOFS_CXX_STRING_INTERNAL_HPP
