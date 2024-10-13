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

#ifndef IOX_HOOFS_GENERIC_PLATFORM_ACL_HPP
#define IOX_HOOFS_GENERIC_PLATFORM_ACL_HPP

#include "iceoryx_platform/platform_settings.hpp"
#include "iceoryx_platform/types.hpp"

// NOTE: The functions can be individually overwritten by setting the corresponding 'IOX_PLATFORM_OVERRIDE_*' define in
// the respective platform specific 'override/*.h' header


#ifndef IOX_PLATFORM_OVERRIDE_ACL_ALL

#if IOX_FEATURE_ACL

#include <sys/acl.h>

#else

// NOLINTBEGIN(cppcoreguidelines-macro-usage) Macros are required for compatibility with the ones from acl.h

#define ACL_USER_OBJ 0
#define ACL_USER 1
#define ACL_GROUP_OBJ 2
#define ACL_GROUP 3
#define ACL_OTHER 4
#define ACL_READ 5
#define ACL_WRITE 6
#define ACL_MASK 7

// NOLINTEND(cppcoreguidelines-macro-usage)

struct iox_internal_acl_ext
{
};

using acl_t = struct iox_internal_acl_ext*;
using acl_permset_t = int;
using acl_perm_t = int;
using acl_entry_t = int;
using acl_tag_t = int;

#endif // IOX_FEATURE_ACL

#endif // IOX_PLATFORM_OVERRIDE_ACL_ALL

int iox_acl_valid(acl_t /*acl*/);

int iox_acl_set_fd(int /*fd*/, acl_t /*acl*/);

acl_t iox_acl_init(int /*count*/);

int iox_acl_free(void* /*obj_p*/);

int iox_acl_create_entry(acl_t* /*acl_p*/, acl_entry_t* /*entry_p*/);

int iox_acl_set_tag_type(acl_entry_t /*entry_d*/, acl_tag_t /*tag_type*/);

int iox_acl_set_qualifier(acl_entry_t /*entry_d*/, const void* /*qualifier_p*/);

int iox_acl_get_permset(acl_entry_t /*entry_d*/, acl_permset_t* /*permset_p*/);

int iox_acl_add_perm(acl_permset_t /*permset_d*/, acl_perm_t /*perm*/);

char* iox_acl_to_text(acl_t /*acl*/, iox_ssize_t* /*len_p*/);

acl_t iox_acl_from_text(const char* /*buf_p*/);

acl_t iox_acl_get_fd(int /*fd*/);

#endif // IOX_HOOFS_GENERIC_PLATFORM_ACL_HPP
