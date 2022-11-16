// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_hoofs/internal/posix_wrapper/access_control.hpp"
#include "iceoryx_hoofs/cxx/function.hpp"
#include "iceoryx_hoofs/log/logging.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <iostream>

#include "iceoryx_platform/platform_correction.hpp"
namespace iox
{
namespace posix
{
bool AccessController::writePermissionsToFile(const int32_t fileDescriptor) const noexcept
{
    if (m_permissions.empty())
    {
        IOX_LOG(ERROR) << "Error: No ACL entries defined.";
        return false;
    }

    auto maybeWorkingACL = createACL(static_cast<int32_t>(m_permissions.size()) + (m_useACLMask ? 1 : 0));

    if (maybeWorkingACL.has_error())
    {
        IOX_LOG(ERROR) << "Error: Creating ACL failed.";
        return false;
    }

    auto& workingACL = maybeWorkingACL.value();

    // add acl entries
    for (const auto& entry : m_permissions)
    {
        if (!createACLEntry(workingACL.get(), entry))
        {
            return false;
        }
    }

    // add mask to acl if specific users or groups have been added
    if (m_useACLMask)
    {
        createACLEntry(workingACL.get(), {ACL_MASK, Permission::READWRITE, -1U});
    }

    // check if acl is valid
    auto aclCheckCall = posixCall(acl_valid)(workingACL.get()).successReturnValue(0).evaluate();

    if (aclCheckCall.has_error())
    {
        IOX_LOG(ERROR) << "Error: Invalid ACL, cannot write to file.";
        return false;
    }

    // set acl in the file given by descriptor
    auto aclSetFdCall = posixCall(acl_set_fd)(fileDescriptor, workingACL.get()).successReturnValue(0).evaluate();
    if (aclSetFdCall.has_error())
    {
        IOX_LOG(ERROR) << "Error: Could not set file ACL.";
        return false;
    }

    return true;
}

expected<AccessController::smartAclPointer_t, AccessController::AccessControllerError>
AccessController::createACL(const int32_t numEntries) noexcept
{
    // allocate memory for a new ACL
    auto aclInitCall = posixCall(acl_init)(numEntries).failureReturnValue(nullptr).evaluate();

    if (aclInitCall.has_error())
    {
        return error<AccessControllerError>(AccessControllerError::COULD_NOT_ALLOCATE_NEW_ACL);
    }

    // define how to free the memory (custom deleter for the smart pointer)
    cxx::function<void(acl_t)> freeACL = [&](acl_t acl) {
        auto aclFreeCall = posixCall(acl_free)(acl).successReturnValue(0).evaluate();
        // We ensure here instead of returning as this lambda will be called by unique_ptr
        /// NOLINTJUSTIFICATION @todo iox-#1032 will be replaced with refactored error handling
        /// NOLINTNEXTLINE(hicpp-no-array-decay,cppcoreguidelines-pro-bounds-array-to-pointer-decay)
        cxx::Ensures(!aclFreeCall.has_error() && "Could not free ACL memory");
    };

    return success<smartAclPointer_t>(aclInitCall->value, freeACL);
}

bool AccessController::addUserPermission(const Permission permission, const PosixUser::userName_t& name) noexcept
{
    if (name.empty())
    {
        IOX_LOG(ERROR) << "Error: specific users must have an explicit name.";
        return false;
    }

    auto id = posix::PosixUser::getUserID(name);
    if (!id.has_value())
    {
        return false;
    }

    return addPermissionEntry(Category::SPECIFIC_USER, permission, id.value());
}

bool AccessController::addGroupPermission(const Permission permission, const PosixGroup::groupName_t& name) noexcept
{
    if (name.empty())
    {
        IOX_LOG(ERROR) << "Error: specific groups must have an explicit name.";
        return false;
    }

    auto id = posix::PosixGroup::getGroupID(name);
    if (!id.has_value())
    {
        return false;
    }

    return addPermissionEntry(Category::SPECIFIC_GROUP, permission, id.value());
}

bool AccessController::addPermissionEntry(const Category category,
                                          const Permission permission,
                                          const uint32_t id) noexcept
{
    if (m_permissions.size() >= m_permissions.capacity())
    {
        IOX_LOG(ERROR) << "Error: Number of allowed permission entries exceeded.";
        return false;
    }

    switch (category)
    {
    case Category::SPECIFIC_USER:
    {
        if (!posix::PosixUser::getUserName(id).has_value())
        {
            IOX_LOG(ERROR) << "Error: No such user";
            return false;
        }

        m_useACLMask = true;
        break;
    }
    case Category::SPECIFIC_GROUP:
    {
        if (!posix::PosixGroup::getGroupName(id).has_value())
        {
            IOX_LOG(ERROR) << "Error: No such group";
            return false;
        }

        m_useACLMask = true;
        break;
    }
    default:
    {
    }
    }

    m_permissions.emplace_back(PermissionEntry{static_cast<uint32_t>(category), permission, id});
    return true;
}

bool AccessController::createACLEntry(const acl_t ACL, const PermissionEntry& entry) noexcept
{
    // create new entry in acl
    acl_entry_t newEntry{};
    acl_t l_ACL{ACL};

    auto aclCreateEntryCall = posixCall(acl_create_entry)(&l_ACL, &newEntry).successReturnValue(0).evaluate();

    if (aclCreateEntryCall.has_error())
    {
        IOX_LOG(ERROR) << "Error: Could not create new ACL entry.";
        return false;
    }

    // set tag type for new entry (user, group, ...)
    auto tagType = static_cast<acl_tag_t>(entry.m_category);
    auto aclSetTagTypeCall = posixCall(acl_set_tag_type)(newEntry, tagType).successReturnValue(0).evaluate();

    if (aclSetTagTypeCall.has_error())
    {
        IOX_LOG(ERROR) << "Error: Could not add tag type to ACL entry.";
        return false;
    }

    // set qualifier for new entry (names of specific users or groups)
    switch (entry.m_category)
    {
    case ACL_USER:
    {
        auto aclSetQualifierCall =
            posixCall(acl_set_qualifier)(newEntry, &(entry.m_id)).successReturnValue(0).evaluate();

        if (aclSetQualifierCall.has_error())
        {
            IOX_LOG(ERROR) << "Error: Could not set ACL qualifier of user " << entry.m_id;
            return false;
        }

        break;
    }
    case ACL_GROUP:
    {
        auto aclSetQualifierCall =
            posixCall(acl_set_qualifier)(newEntry, &(entry.m_id)).successReturnValue(0).evaluate();

        if (aclSetQualifierCall.has_error())
        {
            IOX_LOG(ERROR) << "Error: Could not set ACL qualifier of group " << entry.m_id;
            return false;
        }
        break;
    }
    default:
        // no qualifier required
        break;
    }

    // get reference to permission set in new entry
    acl_permset_t entryPermissionSet{};

    auto aclGetPermsetCall = posixCall(acl_get_permset)(newEntry, &entryPermissionSet).successReturnValue(0).evaluate();

    if (aclGetPermsetCall.has_error())
    {
        IOX_LOG(ERROR) << "Error: Could not obtain ACL permission set of new ACL entry.";
        return false;
    }

    switch (entry.m_permission)
    {
    case Permission::READ:
    {
        return addAclPermission(entryPermissionSet, ACL_READ);
    }
    case Permission::WRITE:
    {
        return addAclPermission(entryPermissionSet, ACL_WRITE);
    }
    case Permission::READWRITE:
    {
        if (!addAclPermission(entryPermissionSet, ACL_READ))
        {
            return false;
        }
        return addAclPermission(entryPermissionSet, ACL_WRITE);
    }
    case Permission::NONE:
    { // don't add any permission
        return true;
    }
    default:
    {
        return false;
    }
    }
}

bool AccessController::addAclPermission(acl_permset_t permset, acl_perm_t perm) noexcept
{
    auto aclAddPermCall = posixCall(acl_add_perm)(permset, perm).successReturnValue(0).evaluate();

    if (aclAddPermCall.has_error())
    {
        IOX_LOG(ERROR) << "Error: Could not add permission to ACL permission set.";
        return false;
    }
    return true;
}

} // namespace posix
} // namespace iox
