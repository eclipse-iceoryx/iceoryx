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

#pragma once

#include "iceoryx_utils/platform/grp.hpp"

struct group
{
    char* gr_name;
    char* gr_passwd;
    gid_t gr_gid;
    char** gr_mem;
};

inline gid_t getegid(void)
{
    return 0;
}

inline struct group* getgrnam(const char* name)
{
    return nullptr;
}

inline struct group* getgrgid(gid_t gid)
{
    return nullptr;
}

inline int getgrouplist(const char* user, gid_t group, gid_t* groups, int* ngroups)
{
    return 0;
}
