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

#include "iox/type_traits.hpp"

namespace iox
{
// NOLINTJUSTIFICATION See definitions in header file.
// NOLINTBEGIN(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
constexpr const char TypeInfo<int8_t>::NAME[];
constexpr const char TypeInfo<int16_t>::NAME[];
constexpr const char TypeInfo<int32_t>::NAME[];
constexpr const char TypeInfo<int64_t>::NAME[];
constexpr const char TypeInfo<uint8_t>::NAME[];
constexpr const char TypeInfo<uint16_t>::NAME[];
constexpr const char TypeInfo<uint32_t>::NAME[];
constexpr const char TypeInfo<uint64_t>::NAME[];
constexpr const char TypeInfo<bool>::NAME[];
constexpr const char TypeInfo<char>::NAME[];
constexpr const char TypeInfo<float>::NAME[];
constexpr const char TypeInfo<double>::NAME[];
constexpr const char TypeInfo<long double>::NAME[];
// NOLINTEND(hicpp-avoid-c-arrays, cppcoreguidelines-avoid-c-arrays)
} // namespace iox
