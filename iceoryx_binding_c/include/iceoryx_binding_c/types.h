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

struct wait_set_storage_t
{
    // the value of the array size is the result of the following formula:
    // sizeof(WaitSet) / 8
    uint64_t do_not_touch_me[133];
};

struct guard_cond_storage_t
{
    // the value of the array size is the result of the following formula:
    // sizeof(GuardCondition) / 8
    uint64_t do_not_touch_me[9];
};

struct iox_sub_storage_t
{
};


#endif
