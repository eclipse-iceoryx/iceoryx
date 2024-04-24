// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_FILESYSTEM_POSIX_ACL_HPP
#define IOX_HOOFS_POSIX_FILESYSTEM_POSIX_ACL_HPP

#include "iceoryx_platform/acl.hpp"
#include "iox/expected.hpp"
#include "iox/posix_group.hpp"
#include "iox/posix_user.hpp"
#include "iox/string.hpp"
#include "iox/unique_ptr.hpp"
#include "iox/vector.hpp"

#include <cstdint>
#include <iostream>
#include <type_traits>

namespace iox
{
namespace detail
{
/// @brief abstraction class for the management of access control lists (ACLs).
///
/// ACLs allow to define fine-grained access rights for files. In addition to the standard access rights, which can only
/// distinguish between user/group/others, ACLs can be used to give specific access rights to named users and groups.
/// ACL is part of the posix specification. The 'PosixAcl' class is used to store ACL permission entries and
/// provides a way to write those entries to a file. A permission entry can be seen as a combination of an access
/// Category, a Permission and an optional name (used to identify specific users and groups.)
class PosixAcl
{
  public:
    enum class Error : uint8_t
    {
        COULD_NOT_ALLOCATE_NEW_ACL,
    };

    /// @brief maximum number of permission entries the 'PosixAcl' can store
    static constexpr int32_t MaxNumOfPermissions = 20;

/// @brief identifier for a permission entry (user, group, others, ...)
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
    enum class Category : std::underlying_type<acl_tag_t>::type
#else
    // NOLINTNEXTLINE(performance-enum-size) required for compatibility with ACL API
    enum class Category : acl_tag_t
#endif
    {
        USER = ACL_USER_OBJ,
        /// a specific user must be identified by a name
        SPECIFIC_USER = ACL_USER,
        GROUP = ACL_GROUP_OBJ,
        /// a specific group must be identified by a name
        SPECIFIC_GROUP = ACL_GROUP,
        OTHERS = ACL_OTHER,
    };

/// @brief access right for a permission entry
#if defined(QNX) || defined(QNX__) || defined(__QNX__)
    enum class Permission : std::underlying_type<acl_perm_t>::type
#else
    // NOLINTNEXTLINE(performance-enum-size) required for compatibility with ACL API
    enum class Permission : acl_perm_t
#endif
    {
        READ = ACL_READ,
        WRITE = ACL_WRITE,
        READWRITE = Permission::READ | Permission::WRITE,
        NONE = 0
    };

    /// @brief Value for an invalid user or group id
    static constexpr uint32_t INVALID_ID = std::numeric_limits<uint32_t>::max();

    /// @brief define and store a specific permission entry to be used by writePermissionsToFile.
    /// @param[id] id of the user or group. For Category::SPECIFIC_USER or Category::SPECIFIC_GROUP the id is
    /// required. Otherwise writing the permission entry to a file will fail. For the default user/group/others
    /// categories the id is ignored and can therefore be left empty. Do not forget to add permissions of the standard
    /// user/group/others categories before writing to a file.
    /// @param[permission] Permissions which should be applied to the category.
    /// @param[id] The group or user id - depending on the category. For Category::USER, Category::GROUP and
    ///   Category::OTHER the id is not required for everything else a valid group or user id is mandatory.
    bool
    addPermissionEntry(const Category category, const Permission permission, const uint32_t id = INVALID_ID) noexcept;

    /// @brief See addPermissionEntry, but one provides the user name instead of user id
    /// @param[in] permissions The permissions which should be applied to the category.
    /// @param[in] name the user name to which the permissions should be applied
    /// @return true when the permissions are applied, on failure false
    bool addUserPermission(const Permission permission, const PosixUser::userName_t& name) noexcept;

    /// @brief See addPermissionEntry, but one provides the user group instead of group id
    /// @param[in] permissions The permissions which should be applied to the category.
    /// @param[in] name the group name to which the permissions should be applied
    /// @return true when the permissions are applied, on failure false
    bool addGroupPermission(const Permission permission, const PosixGroup::groupName_t& name) noexcept;

    /// @brief Write permission entries stored by the 'PosixAcl' to a file identified by a file descriptor.
    /// @param[fileDescriptor] identifier for a file (can be regular file, shared memory file, message queue file...
    /// everything is a file).
    /// @return true if succesful. If false, you can assume that the file has not been touched at all.
    bool writePermissionsToFile(const int32_t fileDescriptor) const noexcept;

  private:
    using smartAclPointer_t = iox::unique_ptr<std::remove_pointer<acl_t>::type>;

    struct PermissionEntry
    {
        unsigned int m_category;
        Permission m_permission;
        unsigned int m_id;
    };

    vector<PermissionEntry, MaxNumOfPermissions> m_permissions;

    static expected<smartAclPointer_t, Error> createACL(const int32_t numEntries) noexcept;
    static bool createACLEntry(const acl_t ACL, const PermissionEntry& entry) noexcept;
    static bool addAclPermission(acl_permset_t permset, acl_perm_t perm) noexcept;

    bool m_useACLMask{false};
};
} // namespace detail
} // namespace iox

#endif // IOX_HOOFS_POSIX_FILESYSTEM_POSIX_ACL_HPP
