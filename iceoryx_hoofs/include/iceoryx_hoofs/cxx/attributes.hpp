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

#ifndef IOX_HOOFS_CXX_ATTRIBUTES_HPP
#define IOX_HOOFS_CXX_ATTRIBUTES_HPP

namespace iox
{
namespace cxx
{
namespace internal
{
/// We use this as an alternative to "static_cast<void>(someVar)" to signal the
/// compiler an unused variable. "static_cast" produces an useless-cast warning
/// on gcc and this approach solves it cleanly.
template <typename T>
inline void IOX_DISCARD_RESULT_IMPL(T&&) noexcept
{
}
} // namespace internal

// NOLINTJUSTIFICATION cannot be implemented with a function, required as inline code
// NOLINTBEGIN(cppcoreguidelines-macro-usage)
/// @brief if a function has a return value which you do not want to use then you can wrap the function with that macro.
/// Purpose is to suppress the unused compiler warning by adding an attribute to the return value
/// @param[in] expr name of the function where the return value is not used.
/// @code
///     uint32_t foo();
///     IOX_DISCARD_RESULT(foo()); // suppress compiler warning for unused return value
/// @endcode
#define IOX_DISCARD_RESULT(expr) ::iox::cxx::internal::IOX_DISCARD_RESULT_IMPL(expr)

/// @brief IOX_NO_DISCARD adds the [[nodiscard]] keyword if it is available for the current compiler.
///        If additionally the keyword [[gnu::warn_unused]] is present it will be added as well.
/// @note
//    [[nodiscard]], [[gnu::warn_unused]] supported since gcc 4.8 (https://gcc.gnu.org/projects/cxx-status.html)
///   [[nodiscard]], [[gnu::warn_unused]] supported since clang 3.9 (https://clang.llvm.org/cxx_status.html)
///   activate keywords for gcc>=5 or clang>=4
#if defined(_WIN32)
// On WIN32 we are using C++17 which makes the keyword [[nodiscard]] available
#define IOX_NO_DISCARD [[nodiscard]]
#elif defined(__APPLE__) && defined(__clang__)
// On APPLE we are using C++17 which makes the keyword [[nodiscard]] available
#define IOX_NO_DISCARD [[nodiscard, gnu::warn_unused]]
#elif (defined(__clang__) && (__clang_major__ >= 4))
#define IOX_NO_DISCARD [[gnu::warn_unused]]
#elif (defined(__GNUC__) && (__GNUC__ >= 5))
#define IOX_NO_DISCARD [[nodiscard, gnu::warn_unused]]
#else
// on an unknown platform we use for now nothing since we do not know what is supported there
#define IOX_NO_DISCARD
#endif

/// @brief IOX_FALLTHROUGH adds the [[fallthrough]] keyword when it is available for the current compiler.
/// @note
//    [[fallthrough]] supported since gcc 7 (https://gcc.gnu.org/projects/cxx-status.html)
///   [[fallthrough]] supported since clang 3.9 (https://clang.llvm.org/cxx_status.html)
///   activate keywords for gcc>=7 or clang>=4
#if defined(_WIN32)
// On WIN32 we are using C++17 which makes the keyword [[fallthrough]] available
#define IOX_FALLTHROUGH [[fallthrough]]
#elif defined(__APPLE__) && defined(__clang__)
// On APPLE we are using C++17 which makes the keyword [[fallthrough]] available
#define IOX_FALLTHROUGH [[fallthrough]]
// with C++17 fallthrough was introduced and we can use it
#elif __cplusplus >= 201703L
// clang prints a warning therefore we exclude it here
#define IOX_FALLTHROUGH [[fallthrough]]
#elif (defined(__GNUC__) && (__GNUC__ >= 7)) && !defined(__clang__)
#define IOX_FALLTHROUGH [[gnu::fallthrough]]
#else
// on an unknown platform we use for now nothing since we do not know what is supported there
#define IOX_FALLTHROUGH
#endif

/// @brief IOX_MAYBE_UNUSED adds the [[gnu::unused]] or [[maybe_unused]] attribute when it is available for the current
/// compiler.
/// @note
///   activate attribute for gcc or clang
#if defined(__GNUC__) || defined(__clang__)
#define IOX_MAYBE_UNUSED [[gnu::unused]]
#elif defined(_WIN32)
// On WIN32 we are using C++17 which makes the attribute [[maybe_unused]] available
#define IOX_MAYBE_UNUSED [[maybe_unused]]
// on an unknown platform we use for now nothing since we do not know what is supported there
#else
#define IOX_MAYBE_UNUSED
#endif

// NOLINTEND(cppcoreguidelines-macro-usage)
} // namespace cxx
} // namespace iox

#endif
