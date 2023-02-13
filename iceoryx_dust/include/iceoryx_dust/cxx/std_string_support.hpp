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
#include "iox/string.hpp"

#include <string>

namespace iox
{
template <uint64_t N>
struct FromImpl<string<N>, std::string>
{
    static std::string fromImpl(const string<N>& value);
};

template <uint64_t N>
struct FromImpl<std::string, string<N>>
{
    static string<N> fromImpl(const std::string& value) noexcept;
};

template <uint64_t N>
struct FromImpl<std::string, lossy<string<N>>>
{
    static string<N> fromImpl(const std::string& value) noexcept;
};
} // namespace iox

#include "iceoryx_dust/internal/cxx/std_string_support.inl"

#endif // IOX_DUST_STD_STRING_SUPPORT_HPP
