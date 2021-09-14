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

#include "iceoryx_hoofs/internal/posix_wrapper/access_control.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_access_rights.hpp"
#include "iceoryx_hoofs/posix_wrapper/posix_call.hpp"

#include <iostream>

#include "iceoryx_hoofs/platform/platform_correction.hpp"
namespace iox
{
namespace posix
{
bool AccessController::writePermissionsToFile(const int32_t f_fileDescriptor) const noexcept
{
    if (m_permissions.empty())
    {
        std::cerr << "Error: No ACL entries defined." << std::endl;
        return false;
    }

    auto maybeWorkingACL = createACL(static_cast<int32_t>(m_permissions.size()) + (m_useACLMask ? 1 : 0));

    if (maybeWorkingACL.has_error())
    {
        std::cerr << "Error: Creating ACL failed." << std::endl;
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
        std::cerr << "Error: Invalid ACL, cannot write to file." << std::endl;
        return false;
    }

    // set acl in the file given by descriptor
    auto aclSetFdCall = posixCall(acl_set_fd)(f_fileDescriptor, workingACL.get()).successReturnValue(0).evaluate();
    if (aclSetFdCall.has_error())
    {
        std::cerr << "Error: Could not set file ACL." << std::endl;
        return false;
    }

    return true;
}

cxx::expected<AccessController::smartAclPointer_t, AccessController::AccessControllerError>
AccessController::createACL(const int32_t f_numEntries) noexcept
{
    // allocate memory for a new ACL
    auto aclInitCall = posixCall(acl_init)(f_numEntries).failureReturnValue(nullptr).evaluate();

    if (aclInitCall.has_error())
    {
        return cxx::error<AccessControllerError>(AccessControllerError::COULD_NOT_ALLOCATE_NEW_ACL);
    }

    // define how to free the memory (custom deleter for the smart pointer)
    std::function<void(acl_t)> freeACL = [&](acl_t acl) {
        auto aclFreeCall = posixCall(acl_free)(acl).successReturnValue(0).evaluate();
        // We ensure here instead of returning as this lambda will be called by unique_ptr
        cxx::Ensures(!aclFreeCall.has_error() && "Could not free ACL memory");
    };

    return cxx::success<smartAclPointer_t>(reinterpret_cast<acl_t>(aclInitCall->value), freeACL);
}

bool AccessController::addPermissionEntry(const Category f_category,
                                          const Permission f_permission,
                                          const string_t& f_name) noexcept
{
    switch (f_category)
    {
    case Category::SPECIFIC_USER:
    {
        if (f_name.empty())
        {
            std::cerr << "Error: specific users must have an explicit name." << std::endl;
            return false;
        }

        auto id = posix::PosixUser::getUserID(f_name);
        if (!id.has_value())
        {
            return false;
        }
        else
        {
            return addPermissionEntry(f_category, f_permission, id.value());
        }

        break;
    }
    case Category::SPECIFIC_GROUP:
    {
        if (f_name.empty())
        {
            std::cerr << "Error: specific groups must have an explicit name." << std::endl;
            return false;
        }

        auto id = posix::PosixGroup::getGroupID(f_name);
        if (!id.has_value())
        {
            return false;
        }
        else
        {
            return addPermissionEntry(f_category, f_permission, id.value());
        }

        break;
    }
    default:
    {
        std::cerr << "Error: Cannot add a name to a default file owner" << std::endl;
    }
    }
    return false;
}

bool AccessController::addPermissionEntry(const Category f_category,
                                          const Permission f_permission,
                                          const uint32_t f_id) noexcept
{
    if (m_permissions.size() >= m_permissions.capacity())
    {
        std::cerr << "Error: Number of allowed permission entries exceeded." << std::endl;
        return false;
    }

    switch (f_category)
    {
    case Category::SPECIFIC_USER:
    {
        if (!posix::PosixUser::getUserName(f_id).has_value())
        {
            std::cerr << "Error: No such user" << std::endl;
            return false;
        }

        m_useACLMask = true;
        break;
    }
    case Category::SPECIFIC_GROUP:
    {
        if (!posix::PosixGroup::getGroupName(f_id).has_value())
        {
            std::cerr << "Error: No such group" << std::endl;
            return false;
        }

        m_useACLMask = true;
        break;
    }
    default:
    {
    }
    }

    m_permissions.emplace_back(PermissionEntry{static_cast<uint32_t>(f_category), f_permission, f_id});
    return true;
}

bool AccessController::createACLEntry(const acl_t f_ACL, const PermissionEntry& f_entry) noexcept
{
    // create new entry in acl
    acl_entry_t newEntry{};
    acl_t l_ACL{f_ACL};

    auto aclCreateEntryCall = posixCall(acl_create_entry)(&l_ACL, &newEntry).successReturnValue(0).evaluate();

    if (aclCreateEntryCall.has_error())
    {
        std::cerr << "Error: Could not create new ACL entry." << std::endl;
        return false;
    }

    // set tag type for new entry (user, group, ...)
    auto tagType = static_cast<acl_tag_t>(f_entry.m_category);
    auto aclSetTagTypeCall = posixCall(acl_set_tag_type)(newEntry, tagType).successReturnValue(0).evaluate();

    if (aclSetTagTypeCall.has_error())
    {
        std::cerr << "Error: Could not add tag type to ACL entry." << std::endl;
        return false;
    }

    // set qualifier for new entry (names of specific users or groups)
    switch (f_entry.m_category)
    {
    case ACL_USER:
    {
        auto aclSetQualifierCall =
            posixCall(acl_set_qualifier)(newEntry, &(f_entry.m_id)).successReturnValue(0).evaluate();

        if (aclSetQualifierCall.has_error())
        {
            std::cerr << "Error: Could not set ACL qualifier of user " << f_entry.m_id << std::endl;
            return false;
        }

        break;
    }
    case ACL_GROUP:
    {
        auto aclSetQualifierCall =
            posixCall(acl_set_qualifier)(newEntry, &(f_entry.m_id)).successReturnValue(0).evaluate();

        if (aclSetQualifierCall.has_error())
        {
            std::cerr << "Error: Could not set ACL qualifier of group " << f_entry.m_id << std::endl;
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
        std::cerr << "Error: Could not obtain ACL permission set of new ACL entry." << std::endl;
        return false;
    }

    switch (f_entry.m_permission)
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

bool AccessController::addAclPermission(acl_permset_t f_permset, acl_perm_t f_perm) noexcept
{
    auto aclAddPermCall = posixCall(acl_add_perm)(f_permset, f_perm).successReturnValue(0).evaluate();

    if (aclAddPermCall.has_error())
    {
        std::cerr << "Error: Could not add permission to ACL permission set." << std::endl;
        return false;
    }
    return true;
}

} // namespace posix
} // namespace iox
