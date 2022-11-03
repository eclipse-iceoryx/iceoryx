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
#ifndef IOX_DUST_STRING_CONVERSION_HPP
#define IOX_DUST_STRING_CONVERSION_HPP

// rename to string conversion

#include "iceoryx_hoofs/cxx/string.hpp"

namespace iox
{
namespace cxx
{
template <typename Source, typename Destination>
struct From;

/// @brief Converts an object of type F to an object of type T
/// @tparam F type of source
/// @tparam T type of destination
/// @param[value] value to convert
/// @return converted value
template <typename F, typename T>
constexpr T convert(const F value) noexcept;

template <uint64_t N>
struct From<string<N>, std::string>
{
    static std::string convert(const string<N>& value);
};

template <uint64_t N>
struct From<std::string, string<N>>
{
    static string<N> convert(const std::string& value) noexcept;
};

} // namespace cxx
} // namespace iox

#include "iceoryx_dust/internal/cxx/string_conversion.inl"

#endif // IOX_DUST_STRING_CONVERSION_HPP