// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by NXP. All rights reserved.
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

#ifndef IOX_HOOFS_FREERTOS_PLATFORM_GRP_HPP
#define IOX_HOOFS_FREERTOS_PLATFORM_GRP_HPP

#include "iceoryx_platform/types.hpp"

struct group
{
    const char* gr_name;
    const char* gr_passwd;
    iox_gid_t gr_gid;
    const char** gr_mem;
};

inline struct group* getgrnam(const char*)
{
    static const char* groupName = "iceoryx_freertos_group";
    static const char* groupPasswd = "iceoryx_freertos_passwd";
    static struct group dummy;
    dummy.gr_name = groupName;
    dummy.gr_passwd = groupPasswd;
    dummy.gr_mem = nullptr;
    dummy.gr_gid = 0;
    return &dummy;
}

inline struct group* getgrgid(iox_gid_t)
{
    static const char* groupName = "iceoryx_freertos_group";
    static const char* groupPasswd = "iceoryx_freertos_passwd";
    static struct group dummy;
    dummy.gr_name = groupName;
    dummy.gr_passwd = groupPasswd;
    dummy.gr_mem = nullptr;
    dummy.gr_gid = 0;
    return &dummy;
}

inline int iox_getgrouplist(const char*, iox_gid_t, iox_gid_t* groups, int* ngroups)
{
    groups[0] = 0;
    *ngroups = 1;
    return 0;
}

#endif // IOX_HOOFS_FREERTOS_PLATFORM_GRP_HPP
