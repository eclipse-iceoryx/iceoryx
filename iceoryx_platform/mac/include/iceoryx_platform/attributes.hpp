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

#ifndef IOX_HOOFS_MAC_PLATFORM_ATTRIBUTES_HPP
#define IOX_HOOFS_MAC_PLATFORM_ATTRIBUTES_HPP

/// @brief IOX_NO_DISCARD adds the [[nodiscard]] keyword if it is available for the current compiler.
///        If additionally the keyword [[gnu::warn_unused]] is present it will be added as well.
/// @note
//    [[nodiscard]], [[gnu::warn_unused]] supported since gcc 4.8 (https://gcc.gnu.org/projects/cxx-status.html)
///   [[nodiscard]], [[gnu::warn_unused]] supported since clang 3.9 (https://clang.llvm.org/cxx_status.html)
///   activate keywords for gcc>=5 or clang>=4

#if defined(__clang__)
#define IOX_NO_DISCARD [[nodiscard, gnu::warn_unused]]
#elif (defined(__clang__) && (__clang_major__ >= 4))
#define IOX_NO_DISCARD [[gnu::warn_unused]]
#elif (defined(__GNUC__) && (__GNUC__ >= 5))
#define IOX_NO_DISCARD [[nodiscard, gnu::warn_unused]]
#endif

/// @brief IOX_FALLTHROUGH adds the [[fallthrough]] keyword when it is available for the current compiler.
/// @note
//    [[fallthrough]] supported since gcc 7 (https://gcc.gnu.org/projects/cxx-status.html)
///   [[fallthrough]] supported since clang 3.9 (https://clang.llvm.org/cxx_status.html)
///   activate keywords for gcc>=7 or clang>=4
#if defined(__clang__)
#define IOX_FALLTHROUGH [[fallthrough]]
#else
#define IOX_FALLTHROUGH
#endif

/// @brief IOX_MAYBE_UNUSED adds the [[gnu::unused]] or [[maybe_unused]] attribute when it is available for the current
/// compiler.
/// @note
///   activate attribute for gcc or clang
#if defined(__GNUC__) || defined(__clang__)
#define IOX_MAYBE_UNUSED [[gnu::unused]]
#endif

#endif // IOX_HOOFS_MAC_PLATFORM_ATTRIBUTES_HPP
