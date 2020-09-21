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

#ifndef IOX_BINDING_C_TYPES_H
#define IOX_BINDING_C_TYPES_H

#include "internal/c2cpp_binding.h"

/// The issue iox-308: https://github.com/eclipse/iceoryx/issues/308
/// was created to explore other options then a magic number to create
/// the structs of a specific size in C.

/// The size and the alignment of all structs are verified by the
/// binding c integration test iox_types_test

struct iox_ws_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(WaitSet) / 8
    uint64_t do_not_touch_me[133];
};
typedef struct iox_ws_storage_t_ iox_ws_storage_t;

struct iox_guard_cond_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(GuardCondition) / 8
#if defined(__APPLE__)
    uint64_t do_not_touch_me[12];
#else
    uint64_t do_not_touch_me[9];
#endif
};
typedef struct iox_guard_cond_storage_t_ iox_guard_cond_storage_t;

struct iox_sub_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(cpp2c_Subscriber) / 8
    uint64_t do_not_touch_me[4];
};
typedef struct iox_sub_storage_t_ iox_sub_storage_t;

struct iox_pub_storage_t_
{
    // the value of the array size is the result of the following formula:
    // sizeof(cpp2c_Publisher) / 8
    uint64_t do_not_touch_me[1];
};
typedef struct iox_pub_storage_t_ iox_pub_storage_t;


#endif
