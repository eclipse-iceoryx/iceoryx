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
constexpr char ASCII_DASH{'-'};
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

/// @brief this class implements the filesystem perms feature of C++17 but as a new type
///        instead of an enum.
///        The class satisfies also all requirements of the BitmaskType, this means
///        the operators '|', '&', '^', '~', '|=', '&=' and '^=' are implemented as
///        free functions as C++17 requires it.
/// @code
/// #include <iox/filesystem.hpp>
/// void foo()
/// {
///     iox::access_rights bar { iox::perms::owner_all | iox::perms::group_read };
/// }
///
/// @endcode
class access_rights final
{
  public:
    using value_type = uint16_t;

    access_rights() noexcept = default;
    access_rights(const access_rights&) noexcept = default;
    access_rights(access_rights&&) noexcept = default;

    access_rights& operator=(const access_rights&) noexcept = default;
    access_rights& operator=(access_rights&&) noexcept = default;

    ~access_rights() noexcept = default;

    constexpr value_type value() const noexcept;

    struct detail
    {
        // AXIVION DISABLE STYLE AutosarC++19_03-M2.13.2 : Filesystem permissions are defined in octal representation

        // the code cannot be moved to the '*.inl' file since the functions are used in the 'perms' namespace to define
        // the respective constants

        /// @note Implementation detail! Please use 'iox::perms::none'.
        static constexpr access_rights none() noexcept
        {
            constexpr value_type VALUE{0};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::owner_read'.
        static constexpr access_rights owner_read() noexcept
        {
            constexpr value_type VALUE{0400};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::owner_write'.
        static constexpr access_rights owner_write() noexcept
        {
            constexpr value_type VALUE{0200};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::owner_exec'.
        static constexpr access_rights owner_exec() noexcept
        {
            constexpr value_type VALUE{0100};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::owner_all'.
        static constexpr access_rights owner_all() noexcept
        {
            constexpr value_type VALUE{0700};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::group_read'.
        static constexpr access_rights group_read() noexcept
        {
            constexpr value_type VALUE{040};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::group_write'.
        static constexpr access_rights group_write() noexcept
        {
            constexpr value_type VALUE{020};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::group_exec'.
        static constexpr access_rights group_exec() noexcept
        {
            constexpr value_type VALUE{010};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::group_all'.
        static constexpr access_rights group_all() noexcept
        {
            constexpr value_type VALUE{070};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::others_read'.
        static constexpr access_rights others_read() noexcept
        {
            constexpr value_type VALUE{04};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::others_write'.
        static constexpr access_rights others_write() noexcept
        {
            constexpr value_type VALUE{02};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::others_exec'.
        static constexpr access_rights others_exec() noexcept
        {
            constexpr value_type VALUE{01};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::others_all'.
        static constexpr access_rights others_all() noexcept
        {
            constexpr value_type VALUE{07};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::all'.
        static constexpr access_rights all() noexcept
        {
            constexpr value_type VALUE{0777};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::set_uid'.
        static constexpr access_rights set_uid() noexcept
        {
            constexpr value_type VALUE{04000};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::set_gid'.
        static constexpr access_rights set_gid() noexcept
        {
            constexpr value_type VALUE{02000};
            return access_rights{VALUE};
        }
        /// @note Implementation detail! Please use 'iox::perms::sticky_bit'.
        static constexpr access_rights sticky_bit() noexcept
        {
            constexpr value_type VALUE{01000};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::mask'.
        static constexpr access_rights mask() noexcept
        {
            constexpr value_type VALUE{07777};
            return access_rights{VALUE};
        }

        /// @note Implementation detail! Please use 'iox::perms::unknown'.
        static constexpr access_rights unknown() noexcept
        {
            constexpr value_type VALUE{0xFFFFU};
            return access_rights{VALUE};
        }

        // AXIVION ENABLE STYLE AutosarC++19_03-M2.13.2
    };

    friend constexpr bool operator==(const access_rights lhs, const access_rights rhs) noexcept;
    friend constexpr bool operator!=(const access_rights lhs, const access_rights rhs) noexcept;

    friend constexpr access_rights operator|(const access_rights lhs, const access_rights rhs) noexcept;
    friend constexpr access_rights operator&(const access_rights lhs, const access_rights rhs) noexcept;
    friend constexpr access_rights operator^(const access_rights lhs, const access_rights rhs) noexcept;
    friend constexpr access_rights operator~(const access_rights value) noexcept;
    friend constexpr access_rights operator|=(const access_rights lhs, const access_rights rhs) noexcept;
    friend constexpr access_rights operator&=(const access_rights lhs, const access_rights rhs) noexcept;
    friend constexpr access_rights operator^=(const access_rights lhs, const access_rights rhs) noexcept;

  private:
    template <typename>
    friend struct FileManagementInterface;
    explicit constexpr access_rights(value_type value) noexcept
        : m_value(value)
    {
    }

  private:
    value_type m_value{0};
};

namespace perms
{
// AXIVION DISABLE STYLE AutosarC++19_03-A2.10.5 : Name reuse is intentional since they refer to the same value. Additionally, different namespaces are used.

/// @brief Deny everything
static constexpr access_rights none{access_rights::detail::none()};

/// @brief owner has read permission
static constexpr access_rights owner_read{access_rights::detail::owner_read()};
/// @brief owner has write permission
static constexpr access_rights owner_write{access_rights::detail::owner_write()};
/// @brief owner has execution permission
static constexpr access_rights owner_exec{access_rights::detail::owner_exec()};
/// @brief owner has all permissions
static constexpr access_rights owner_all{access_rights::detail::owner_all()};

/// @brief group has read permission
static constexpr access_rights group_read{access_rights::detail::group_read()};
/// @brief group has write permission
static constexpr access_rights group_write{access_rights::detail::group_write()};
/// @brief group has execution permission
static constexpr access_rights group_exec{access_rights::detail::group_exec()};
/// @brief group has all permissions
static constexpr access_rights group_all{access_rights::detail::group_all()};

/// @brief others have read permission
static constexpr access_rights others_read{access_rights::detail::others_read()};
/// @brief others have write permission
static constexpr access_rights others_write{access_rights::detail::others_write()};
/// @brief others have execution permission
static constexpr access_rights others_exec{access_rights::detail::others_exec()};
/// @brief others have all permissions
static constexpr access_rights others_all{access_rights::detail::others_all()};

/// @brief all permissions for everyone
static constexpr access_rights all{access_rights::detail::all()};

/// @brief set uid bit
/// @note introduction into setgit/setuid: https://en.wikipedia.org/wiki/Setuid
// AXIVION Next Construct AutosarC++19_03-M2.10.1: The constant is in a namespace and mimics the C++17 STL equivalent
static constexpr access_rights set_uid{access_rights::detail::set_uid()};
/// @brief set gid bit
/// @note introduction into setgit/setuid: https://en.wikipedia.org/wiki/Setuid
// AXIVION Next Construct AutosarC++19_03-M2.10.1: The constant is in a namespace and mimics the C++17 STL equivalent
static constexpr access_rights set_gid{access_rights::detail::set_gid()};
/// @brief set sticky bit
/// @note sticky bit introduction: https://en.wikipedia.org/wiki/Sticky_bit
static constexpr access_rights sticky_bit{access_rights::detail::sticky_bit()};

/// @brief all permissions for everyone as well as uid, gid and sticky bit
static constexpr access_rights mask{access_rights::detail::mask()};

/// @brief unknown permissions
static constexpr access_rights unknown{access_rights::detail::unknown()};

// AXIVION ENABLE STYLE AutosarC++19_03-A2.10.5
} // namespace perms


/// @brief Implements the equal operator
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs == rhs
constexpr bool operator==(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the not equal operator
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs != rhs
constexpr bool operator!=(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the binary or operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs | rhs
constexpr access_rights operator|(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the binary and operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs & rhs
constexpr access_rights operator&(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the binary exclusive or operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs ^ rhs
constexpr access_rights operator^(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the binary complement operation
/// @param[in] value the value used for the operation
/// @return ~value
constexpr access_rights operator~(const access_rights value) noexcept;

/// @brief Implements the binary or assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs | rhs
constexpr access_rights operator|=(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the binary and assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs & rhs
constexpr access_rights operator&=(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief Implements the binary exclusive or assignment operation
/// @param[in] lhs left hand side of the operation
/// @param[in] rhs right hand side of the operation
/// @return lhs = lhs ^ rhs
constexpr access_rights operator^=(const access_rights lhs, const access_rights rhs) noexcept;

/// @brief The 'ostream' operator for the 'access_rights' class. It handles the class as if
///        it was a bitset and always lists the values for owner, group, others, special bits
/// @param[in] stream reference to the 'ostream'
/// @param[in] value the file permission
/// @return the reference to the stream
std::ostream& operator<<(std::ostream& stream, const access_rights value) noexcept;

/// @brief The 'LogStream' operator for the 'access_rights' class. It handles the class as if
///        it was a bitset and always lists the values for owner, group, others, special bits
/// @param[in] stream reference to the 'LogStream'
/// @param[in] value the file permission
/// @return the reference to the stream
iox::log::LogStream& operator<<(iox::log::LogStream& stream, const access_rights value) noexcept;
} // namespace iox

#include "iox/detail/filesystem.inl"

#endif
