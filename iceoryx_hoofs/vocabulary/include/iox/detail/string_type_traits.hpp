// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_HOOFS_VOCABULARY_STRING_TYPE_TRAITS_HPP
#define IOX_HOOFS_VOCABULARY_STRING_TYPE_TRAITS_HPP

#include <cstdint>

#include "iceoryx_hoofs/cxx/type_traits.hpp"

namespace iox
{
template <uint64_t Capacity>
class string;

/// @brief struct to check whether an argument is a cxx::string
template <typename T>
struct is_cxx_string : std::false_type
{
};

template <uint64_t N>
struct is_cxx_string<::iox::string<N>> : std::true_type
{
};

//////////////////
/// BEGIN TypeInfo
//////////////////

/// @brief Provides a translation from a type into its human readable name
/// NOLINTJUSTIFICATION The name should be stored in a compile time variable. Access is always
///   safe since it is null terminated and always constant. Other alternatives are not available
///   at compile time.
/// NOLINTBEGIN(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)

template <typename T>
struct TypeInfo
{
    static_assert(cxx::always_false_v<T>, "unknown type");
    static constexpr const char NAME[] = "unknown type";
};
template <typename T>
constexpr const char TypeInfo<T>::NAME[];

template <>
struct TypeInfo<int8_t>
{
    static constexpr const char NAME[] = "int8_t";
};

template <>
struct TypeInfo<int16_t>
{
    static constexpr const char NAME[] = "int16_t";
};

template <>
struct TypeInfo<int32_t>
{
    static constexpr const char NAME[] = "int32_t";
};

template <>
struct TypeInfo<int64_t>
{
    static constexpr const char NAME[] = "int64_t";
};

template <>
struct TypeInfo<uint8_t>
{
    static constexpr const char NAME[] = "uint8_t";
};

template <>
struct TypeInfo<uint16_t>
{
    static constexpr const char NAME[] = "uint16_t";
};

template <>
struct TypeInfo<uint32_t>
{
    static constexpr const char NAME[] = "uint32_t";
};

template <>
struct TypeInfo<uint64_t>
{
    static constexpr const char NAME[] = "uint64_t";
};

template <>
struct TypeInfo<bool>
{
    static constexpr const char NAME[] = "bool";
};

template <>
struct TypeInfo<char>
{
    static constexpr const char NAME[] = "char";
};

template <>
struct TypeInfo<float>
{
    static constexpr const char NAME[] = "float";
};

template <>
struct TypeInfo<double>
{
    static constexpr const char NAME[] = "double";
};

template <>
struct TypeInfo<long double>
{
    static constexpr const char NAME[] = "long double";
};

template <uint64_t N>
struct TypeInfo<iox::string<N>>
{
    static constexpr const char NAME[] = "string";
};
template <uint64_t N>
constexpr const char TypeInfo<iox::string<N>>::NAME[];
/// NOLINTEND(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
//////////////////
/// END TypeInfo
//////////////////

} // namespace iox

#endif // IOX_HOOFS_VOCABULARY_STRING_TYPE_TRAITS_HPP
