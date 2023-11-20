// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by Mathias Kraus <elboberido@m-hias.de>. All rights reserved.
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

    /// @brief Creates an 'access_rights' object from a sanitized value, ensuring that only the bits defined in
    /// 'iox::perms::mask' are set when the 'iox::perm::unknown' bit is not set. If the 'iox::perm::unknown' bit is set
    /// all other bits will be reset.
    /// @param[in] value for the 'access_rights'
    /// @return the 'access_rights' object
    static constexpr access_rights from_value_sanitized(const value_type value) noexcept;

    /// @brief Creates an 'access_rights' object from an unchecked value. The user has to ensure that only the bits
    /// defined in 'iox::perms::mask' are set when the 'iox::perm::unknown' bit is not set. If the 'iox::perm::unknown'
    /// bit is set all other bits should be reset.
    /// @param[in] value for the 'access_rights'
    /// @return the 'access_rights' object
    static constexpr access_rights unsafe_from_value_unchecked(const value_type value) noexcept
    {
        // the code cannot be moved to the '*.inl' file since the function is used in the 'perms' namespace to define
        // the 'access_rights' constants
        return access_rights{value};
    }

    constexpr value_type value() const noexcept;

    struct detail
    {
        // AXIVION DISABLE STYLE AutosarC++19_03-M2.13.2 : Filesystem permissions are defined in octal representation

        static constexpr value_type NONE{0};

        static constexpr value_type OWNER_READ{0400};
        static constexpr value_type OWNER_WRITE{0200};
        static constexpr value_type OWNER_EXEC{0100};
        static constexpr value_type OWNER_ALL{0700};

        static constexpr value_type GROUP_READ{040};
        static constexpr value_type GROUP_WRITE{020};
        static constexpr value_type GROUP_EXEC{010};
        static constexpr value_type GROUP_ALL{070};

        static constexpr value_type OTHERS_READ{04};
        static constexpr value_type OTHERS_WRITE{02};
        static constexpr value_type OTHERS_EXEC{01};
        static constexpr value_type OTHERS_ALL{07};

        static constexpr value_type ALL{0777};

        static constexpr value_type SET_UID{04000};
        static constexpr value_type SET_GID{02000};
        static constexpr value_type STICKY_BIT{01000};

        static constexpr value_type MASK{07777};

        // intentionally different from 'std::filesystem::perms::unknown' to prevent unexpected results. Combining a
        // permission set to 'std::filesystem::perms::unknown' with other permission flags using bitwise OR may result
        // in a value that is not representative of a valid permission state. By setting the value to '0x8000' only the
        // MSB is 1 and all bits representing permissions are set to 0 and a bitwise OR will therefore also always
        // result in a 0.
        static constexpr value_type UNKNOWN{0x8000U};

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
static constexpr auto none{access_rights::unsafe_from_value_unchecked(access_rights::detail::NONE)};

/// @brief owner has read permission
static constexpr auto owner_read{access_rights::unsafe_from_value_unchecked(access_rights::detail::OWNER_READ)};
/// @brief owner has write permission
static constexpr auto owner_write{access_rights::unsafe_from_value_unchecked(access_rights::detail::OWNER_WRITE)};
/// @brief owner has execution permission
static constexpr auto owner_exec{access_rights::unsafe_from_value_unchecked(access_rights::detail::OWNER_EXEC)};
/// @brief owner has all permissions
static constexpr auto owner_all{access_rights::unsafe_from_value_unchecked(access_rights::detail::OWNER_ALL)};

/// @brief group has read permission
static constexpr auto group_read{access_rights::unsafe_from_value_unchecked(access_rights::detail::GROUP_READ)};
/// @brief group has write permission
static constexpr auto group_write{access_rights::unsafe_from_value_unchecked(access_rights::detail::GROUP_WRITE)};
/// @brief group has execution permission
static constexpr auto group_exec{access_rights::unsafe_from_value_unchecked(access_rights::detail::GROUP_EXEC)};
/// @brief group has all permissions
static constexpr auto group_all{access_rights::unsafe_from_value_unchecked(access_rights::detail::GROUP_ALL)};

/// @brief others have read permission
static constexpr auto others_read{access_rights::unsafe_from_value_unchecked(access_rights::detail::OTHERS_READ)};
/// @brief others have write permission
static constexpr auto others_write{access_rights::unsafe_from_value_unchecked(access_rights::detail::OTHERS_WRITE)};
/// @brief others have execution permission
static constexpr auto others_exec{access_rights::unsafe_from_value_unchecked(access_rights::detail::OTHERS_EXEC)};
/// @brief others have all permissions
static constexpr auto others_all{access_rights::unsafe_from_value_unchecked(access_rights::detail::OTHERS_ALL)};

/// @brief all permissions for everyone
static constexpr auto all{access_rights::unsafe_from_value_unchecked(access_rights::detail::ALL)};

/// @brief set uid bit
/// @note introduction into setgit/setuid: https://en.wikipedia.org/wiki/Setuid
// AXIVION Next Construct AutosarC++19_03-M2.10.1: The constant is in a namespace and mimics the C++17 STL equivalent
static constexpr auto set_uid{access_rights::unsafe_from_value_unchecked(access_rights::detail::SET_UID)};
/// @brief set gid bit
/// @note introduction into setgit/setuid: https://en.wikipedia.org/wiki/Setuid
// AXIVION Next Construct AutosarC++19_03-M2.10.1: The constant is in a namespace and mimics the C++17 STL equivalent
static constexpr auto set_gid{access_rights::unsafe_from_value_unchecked(access_rights::detail::SET_GID)};
/// @brief set sticky bit
/// @note sticky bit introduction: https://en.wikipedia.org/wiki/Sticky_bit
static constexpr auto sticky_bit{access_rights::unsafe_from_value_unchecked(access_rights::detail::STICKY_BIT)};

/// @brief all permissions for everyone as well as uid, gid and sticky bit
static constexpr auto mask{access_rights::unsafe_from_value_unchecked(access_rights::detail::MASK)};

/// @brief unknown permissions
static constexpr auto unknown{access_rights::unsafe_from_value_unchecked(access_rights::detail::UNKNOWN)};

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
