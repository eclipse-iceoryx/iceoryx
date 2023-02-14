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
#ifndef IOX_HOOFS_FILESYSTEM_FILESYSTEM_HPP
#define IOX_HOOFS_FILESYSTEM_FILESYSTEM_HPP

#include "iox/string.hpp"

#include <cstdint>
#include <type_traits>

namespace iox
{
namespace internal
{
// AXIVION DISABLE STYLE AutosarC++19_03-A3.9.1: Not used as an integer but as actual character.
constexpr char ASCII_A{'a'};
constexpr char ASCII_Z{'z'};
constexpr char ASCII_CAPITAL_A{'A'};
constexpr char ASCII_CAPITAL_Z{'Z'};
constexpr char ASCII_0{'0'};
constexpr char ASCII_9{'9'};
constexpr char ASCII_MINUS{'-'};
constexpr char ASCII_DOT{'.'};
constexpr char ASCII_COLON{':'};
constexpr char ASCII_UNDERSCORE{'_'};
} // namespace internal
// AXIVION ENABLE STYLE AutosarC++19_03-A3.9.1

enum class RelativePathComponents : std::uint32_t
{
    REJECT,
    ACCEPT
};

/// @brief checks if the given string is a valid path entry. A path entry is the string between
///        two path separators.
/// @note A valid path entry for iceoryx must be platform independent and also supported
///       by various file systems. The file systems we intend to support are
///         * linux: ext3, ext4, btrfs
///         * windows: ntfs, exfat, fat
///         * freebsd: ufs, ffs
///         * apple: apfs
///         * qnx: etfs
///         * android: ext3, ext4, fat
///
///       Sometimes it is also possible that a certain file character is supported by the filesystem
///       itself but not by the platforms SDK. One example are files which end with a dot like "myFile."
///       which are supported by ntfs but not by the Windows SDK.
/// @param[in] name the path entry in question
/// @param[in] relativePathComponents are relative path components are allowed for this path entry
/// @return true if it is valid, otherwise false
template <uint64_t StringCapacity>
bool isValidPathEntry(const string<StringCapacity>& name, const RelativePathComponents relativePathComponents) noexcept;

/// @brief checks if the given string is a valid filename. It must fulfill the
///        requirements of a valid path entry (see, isValidPathEntry) and is not allowed
///        to contain relative path components
/// @param[in] name the string to verify
/// @return true if the string is a filename, otherwise false
template <uint64_t StringCapacity>
bool isValidFileName(const string<StringCapacity>& name) noexcept;

/// @brief verifies if the given string is a valid path to a file
/// @param[in] name the string to verify
/// @return true if the string is a path to a file, otherwise false
template <uint64_t StringCapacity>
bool isValidPathToFile(const string<StringCapacity>& name) noexcept;

/// @brief returns true if the provided name is a valid path, otherwise false
/// @param[in] name the string to verify
template <uint64_t StringCapacity>
bool isValidPathToDirectory(const string<StringCapacity>& name) noexcept;

/// @brief returns true if the provided name ends with a path separator, otherwise false
/// @param[in] name the string which may contain a path separator at the end
template <uint64_t StringCapacity>
bool doesEndWithPathSeparator(const string<StringCapacity>& name) noexcept;

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
} // namespace iox

#include "iox/detail/filesystem.inl"

#endif
