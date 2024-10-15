// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#include "iceoryx_platform/acl.hpp"

#ifndef IOX_PLATFORM_OVERRIDE_ACL_ALL

#if IOX_FEATURE_ACL

int iox_acl_valid(acl_t acl)
{
    return acl_valid(acl);
}

int iox_acl_set_fd(int fd, acl_t acl)
{
    return acl_set_fd(fd, acl);
}

acl_t iox_acl_init(int count)
{
    return acl_init(count);
}

int iox_acl_free(void* obj_p)
{
    return acl_free(obj_p);
}

int iox_acl_create_entry(acl_t* acl_p, acl_entry_t* entry_p)
{
    return acl_create_entry(acl_p, entry_p);
}

int iox_acl_set_tag_type(acl_entry_t entry_d, acl_tag_t tag_type)
{
    return acl_set_tag_type(entry_d, tag_type);
}

int iox_acl_set_qualifier(acl_entry_t entry_d, const void* qualifier_p)
{
    return acl_set_qualifier(entry_d, qualifier_p);
}

int iox_acl_get_permset(acl_entry_t entry_d, acl_permset_t* permset_p)
{
    return acl_get_permset(entry_d, permset_p);
}

int iox_acl_add_perm(acl_permset_t permset_d, acl_perm_t perm)
{
    return acl_add_perm(permset_d, perm);
}

char* iox_acl_to_text(acl_t acl, iox_ssize_t* len_p)
{
    return acl_to_text(acl, len_p);
}

acl_t iox_acl_from_text(const char* buf_p)
{
    return acl_from_text(buf_p);
}

acl_t iox_acl_get_fd(int fd)
{
    return acl_get_fd(fd);
}

#endif // IOX_FEATURE_ACL

#endif // IOX_PLATFORM_OVERRIDE_ACL_ALL
