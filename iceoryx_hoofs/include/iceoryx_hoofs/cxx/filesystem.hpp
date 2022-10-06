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
#ifndef IOX_HOOFS_CXX_FILESYSTEM_HPP
#define IOX_HOOFS_CXX_FILESYSTEM_HPP

#include <cstdint>
#include <type_traits>

namespace iox
{
namespace cxx
{
/// @brief this enum class implements the filesystem perms feature of C++17. The
///        API is identical to the C++17 one so that the class can be removed
///        as soon as iceoryx switches to C++17.
///        The enum satisfies also all requirements of the BitmaskType, this means
///        the operators '|', '&', '^', '~', '|=', '&=' and '^=' are implemented as
///        free functions as C++17 requires it.
enum class perms : uint32_t
{
    /// @brief Deny everything
    none = 0,

    /// @brief owner has read permission
    owner_read = 0400,
    /// @brief owner has write permission
    owner_write = 0200,
    /// @brief owner has execution permission
    owner_exec = 0100,
    /// @brief owner has all permissions
    owner_all = 0700,

    /// @brief group has read permission
    group_read = 040,
    /// @brief group has write permission
    group_write = 020,
    /// @brief group has execution permission
    group_exec = 010,
    /// @brief group has all permissions
    group_all = 070,

    /// @brief others have read permission
    others_read = 04,
    /// @brief others have write permission
    others_write = 02,
    /// @brief others have execution permission
    others_exec = 01,
    /// @brief others have all permissions
    others_all = 07,

    /// @brief all permissions for everyone
    all = 0777,

    /// @brief set uid bit
    /// @note introduction into setgit/setuid: https://en.wikipedia.org/wiki/Setuid
    set_uid = 04000,
    /// @brief set gid bit
    /// @note introduction into setgit/setuid: https://en.wikipedia.org/wiki/Setuid
    set_gid = 02000,
    /// @brief set sticky bit
    /// @note sticky bit introduction: https://en.wikipedia.org/wiki/Sticky_bit
    sticky_bit = 01000,

    /// @brief all permissions for everyone as well as uid, gid and sticky bit
    mask = 07777,

    /// @brief unknown permissions
    unknown = 0xFFFF
};

/// @brief Implements the binary or operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs | rhs
constexpr perms operator|(const perms lhs, const perms rhs) noexcept;

/// @brief Implements the binary and operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs & rhs
constexpr perms operator&(const perms lhs, const perms rhs) noexcept;

/// @brief Implements the binary exclusive or operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs ^ rhs
constexpr perms operator^(const perms lhs, const perms rhs) noexcept;

/// @brief Implements the binary complement operation
/// @param[in] value the value used for the operation
/// @return ~value
constexpr perms operator~(const perms value) noexcept;

/// @brief Implements the binary or assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs | rhs
constexpr perms operator|=(const perms lhs, const perms rhs) noexcept;

/// @brief Implements the binary and assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs & rhs
constexpr perms operator&=(const perms lhs, const perms rhs) noexcept;

/// @brief Implements the binary exclusive or assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs ^ rhs
constexpr perms operator^=(const perms lhs, const perms rhs) noexcept;

/// @brief The streaming operator for the perms enum. It handles the enum as if
///        it was a bitset and always lists the values for owner, group, others, special bits
/// @param[in] stream reference to the stream
/// @param[in] value the file permission
/// @return the reference to the stream
template <typename StreamType>
StreamType& operator<<(StreamType& stream, const perms value) noexcept;
} // namespace cxx
} // namespace iox

#include "iceoryx_hoofs/internal/cxx/filesystem.inl"

#endif
