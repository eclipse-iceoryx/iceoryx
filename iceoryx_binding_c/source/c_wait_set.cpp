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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_posh/popo/wait_set.hpp"

using namespace iox;
using namespace iox::popo;

extern "C" {
#include "iceoryx_binding_c/wait_set.h"
}

static void condition_vector_to_c_array(const WaitSet::ConditionVector& conditionVector,
                                        iox_cond_t* const conditionArray,
                                        const uint64_t conditionArrayCapacity,
                                        uint64_t& conditionArraySize,
                                        uint64_t& missedElements)
{
    uint64_t conditionVectorSize = conditionVector.size();
    if (conditionVectorSize > conditionArrayCapacity)
    {
        missedElements = conditionVectorSize - conditionArrayCapacity;
        conditionArraySize = conditionArrayCapacity;
    }
    else
    {
        missedElements = 0U;
        conditionArraySize = conditionVectorSize;
    }

    for (uint64_t i = 0; i < conditionArraySize; ++i)
    {
        conditionArray[i] = conditionVector[i];
    }
}

iox_wait_set_t iox_wait_set_init(iox_wait_set_storage_t* self)
{
    new (self) WaitSet();
    return reinterpret_cast<iox_wait_set_t>(self);
}

void iox_wait_set_deinit(iox_wait_set_t const self)
{
    self->~WaitSet();
}

iox_WaitSetResult iox_wait_set_attach_condition(iox_wait_set_t const self, iox_cond_t const condition)
{
    auto result = self->attachCondition(*condition);
    if (!result.has_error())
        return WaitSetResult_SUCCESS;

    return cpp2c::WaitSetResult(result.get_error());
}

bool iox_wait_set_detach_condition(iox_wait_set_t const self, iox_cond_t const condition)
{
    return self->detachCondition(*condition);
}

void iox_wait_set_detach_all_conditions(iox_wait_set_t const self)
{
    self->detachAllConditions();
}

void iox_wait_set_timed_wait(iox_wait_set_t const self,
                             struct timespec timeout,
                             iox_cond_t* const conditionArray,
                             const uint64_t conditionArrayCapacity,
                             uint64_t& conditionArraySize,
                             uint64_t& missedElements)
{
    condition_vector_to_c_array(
        self->timedWait(units::Duration::nanoseconds(static_cast<unsigned long long int>(timeout.tv_nsec))
                        + units::Duration::seconds(static_cast<unsigned long long int>(timeout.tv_sec))),
        conditionArray,
        conditionArrayCapacity,
        conditionArraySize,
        missedElements);
}

void iox_wait_set_wait(iox_wait_set_t const self,
                       iox_cond_t* const conditionArray,
                       const uint64_t conditionArrayCapacity,
                       uint64_t& conditionArraySize,
                       uint64_t& missedElements)
{
    condition_vector_to_c_array(
        self->wait(), conditionArray, conditionArrayCapacity, conditionArraySize, missedElements);
}

