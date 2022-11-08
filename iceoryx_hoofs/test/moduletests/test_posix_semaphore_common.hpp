// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_HOOFS_MODULETESTS_TEST_POSIX_SEMAPHORE_COMMON_HPP
#define IOX_HOOFS_MODULETESTS_TEST_POSIX_SEMAPHORE_COMMON_HPP

#include "iceoryx_platform/semaphore.hpp"
#include "test.hpp"

template <typename SemaphoreType>
inline bool setSemaphoreToZeroAndVerifyValue(SemaphoreType& semaphore, const uint32_t expectedValue)
{
    uint64_t count = 0U;
    for (; count + 1 < IOX_SEM_VALUE_MAX; ++count)
    {
        auto result = semaphore.tryWait();
        if (result.has_error())
        {
            return false;
        }
        if (!*result)
        {
            return (count == expectedValue);
        }
    }
    return false;
}

template <typename SemaphoreType>
inline bool setSemaphoreValueTo(SemaphoreType& semaphore, const uint32_t value)
{
    for (uint32_t i = 0U; i < value; ++i)
    {
        auto result = semaphore.post();
        if (result.has_error())
        {
            return false;
        }
    }
    return true;
}
#endif
