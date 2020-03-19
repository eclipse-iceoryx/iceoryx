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

#include <grp.h>
#include <unistd.h>

// linux version looks like
//   int getgrouplist(const char *user, gid_t group, gid_t *groups, int *ngroups);
// mac version
//   int getgrouplist(const char* name, int basegid, int* groups, int* ngroups);
//
// in our user template code we need the correct argument types since we are deducing the
// function type. to avoid weird casts directly in the code we create a function
// with the correct argument types and just forward all arguments
inline int getgrouplist(const char* user, gid_t group, gid_t* groups, int* ngroups)
{
    return getgrouplist(user, group, groups, ngroups);
}
