// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_ACL_HPP
#define IOX_HOOFS_WIN_PLATFORM_ACL_HPP

#include "iceoryx_platform/types.hpp"

#define ACL_USER_OBJ 0
#define ACL_USER 1
#define ACL_GROUP_OBJ 2
#define ACL_GROUP 3
#define ACL_OTHER 4
#define ACL_READ 5
#define ACL_WRITE 6
#define ACL_MASK 7

struct __acl_ext
{
};

using acl_t = struct __acl_ext*;
using acl_permset_t = int;
using acl_perm_t = int;
using acl_entry_t = int;
using acl_tag_t = int;

inline int acl_valid(acl_t acl)
{
    return 0;
}

inline int acl_set_fd(int fd, acl_t acl)
{
    return 0;
}

inline acl_t acl_init(int count)
{
    static struct __acl_ext stub;
    return &stub;
}

inline int acl_free(void*)
{
    return 0;
}

inline int acl_create_entry(acl_t* acl_p, acl_entry_t* entry_p)
{
    return 0;
}

inline int acl_set_tag_type(acl_entry_t entry_d, acl_tag_t tag_type)
{
    return 0;
}

inline int acl_set_qualifier(acl_entry_t entry_d, const void* qualifier_p)
{
    return 0;
}

inline int acl_get_permset(acl_entry_t entry_d, acl_permset_t* permset_p)
{
    return 0;
}

inline int acl_add_perm(acl_permset_t permset_d, acl_perm_t perm)
{
    return 0;
}

inline char* acl_to_text(acl_t acl, iox_ssize_t* len_p)
{
    return nullptr;
}

inline acl_t acl_from_text(const char* buf_p)
{
    return acl_t();
}

inline acl_t acl_get_fd(int fd)
{
    return acl_t();
}

#endif // IOX_HOOFS_WIN_PLATFORM_ACL_HPP
