// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#ifndef IOX_HOOFS_WIN_PLATFORM_GRP_HPP
#define IOX_HOOFS_WIN_PLATFORM_GRP_HPP

#include "iceoryx_hoofs/platform/grp.hpp"

struct group
{
    const char* gr_name;
    const char* gr_passwd;
    gid_t gr_gid;
    const char** gr_mem;
};

inline gid_t getegid(void)
{
    return 0;
}

inline struct group* getgrnam(const char* name)
{
    static const char* groupName = "iceoryx_windows_group";
    static const char* groupPasswd = "iceoryx_windows_passwd";
    static struct group dummy;
    dummy.gr_name = groupName;
    dummy.gr_passwd = groupPasswd;
    dummy.gr_mem = nullptr;
    dummy.gr_gid = 0;
    return &dummy;
}

inline struct group* getgrgid(gid_t gid)
{
    static const char* groupName = "iceoryx_windows_group";
    static const char* groupPasswd = "iceoryx_windows_passwd";
    static struct group dummy;
    dummy.gr_name = groupName;
    dummy.gr_passwd = groupPasswd;
    dummy.gr_mem = nullptr;
    dummy.gr_gid = 0;
    return &dummy;
}

inline int iox_getgrouplist(const char* user, gid_t group, gid_t* groups, int* ngroups)
{
    groups[0] = 0;
    *ngroups = 1;
    return 0;
}

#endif // IOX_HOOFS_WIN_PLATFORM_GRP_HPP
