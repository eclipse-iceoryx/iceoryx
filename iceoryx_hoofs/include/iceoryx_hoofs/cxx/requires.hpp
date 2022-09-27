// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_CXX_REQUIRES_HPP
#define IOX_HOOFS_CXX_REQUIRES_HPP

#include "iceoryx_platform/platform_correction.hpp"

namespace iox
{
namespace cxx
{
namespace internal
{
void Require(
    const bool condition, const char* file, const int line, const char* function, const char* conditionString) noexcept;

void Require(const bool condition,
             const char* file,
             const int line,
             const char* function,
             const char* conditionString,
             const char* msgString) noexcept;
} // namespace internal

// implementing C++ Core Guideline, I.6. Prefer Expects
// see:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-expects
/// @NOLINTJUSTIFICATION macro required to capture file, line, function origin of call implicitly
/// @NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @NOLINTJUSTIFICATION array decay: needed for source code location, safely wrapped in macro
/// @NOLINTBEGIN(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
#define Expects(condition) internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition)
/// @NOLINTEND(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
/// @NOLINTEND(cppcoreguidelines-macro-usage)

/// @NOLINTJUSTIFICATION macro required to capture file, line, function origin of call implicitly
/// @NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @NOLINTJUSTIFICATION array decay: needed for source code location, safely wrapped in macro
/// @NOLINTBEGIN(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
#define ExpectsWithMsg(condition, msg)                                                                                 \
    internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition, #msg)
/// @NOLINTEND(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
/// @NOLINTEND(cppcoreguidelines-macro-usage)

// implementing C++ Core Guideline, I.8. Prefer Ensures
// see:
// https://github.com/isocpp/CppCoreGuidelines/blob/master/CppCoreGuidelines.md#Ri-ensures
/// @NOLINTJUSTIFICATION macro required to capture file, line, function origin of call implicitly
/// @NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @NOLINTJUSTIFICATION array decay: needed for source code location, safely wrapped in macro
/// @NOLINTBEGIN(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
#define Ensures(condition) internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition)
/// @NOLINTEND(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
/// @NOLINTEND(cppcoreguidelines-macro-usage)

/// @NOLINTJUSTIFICATION macro required to capture file, line, function origin of call implicitly
/// @NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @NOLINTJUSTIFICATION array decay: needed for source code location, safely wrapped in macro
/// @NOLINTBEGIN(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
#define EnsuresWithMsg(condition, msg)                                                                                 \
    internal::Require(condition, __FILE__, __LINE__, __PRETTY_FUNCTION__, #condition, #msg)
/// @NOLINTEND(hicpp-no-array-decay, cppcoreguidelines-pro-bounds-array-to-pointer-decay)
/// @NOLINTEND(cppcoreguidelines-macro-usage)


} // namespace cxx
} // namespace iox

#endif
