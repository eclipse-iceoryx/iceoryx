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

#include "iox/unnamed_semaphore.hpp"
#include "iox/logging.hpp"
#include "iox/posix_call.hpp"

namespace iox
{
expected<void, SemaphoreError>
UnnamedSemaphoreBuilder::create(optional<UnnamedSemaphore>& uninitializedSemaphore) const noexcept
{
    if (m_initialValue > IOX_SEM_VALUE_MAX)
    {
        IOX_LOG(ERROR,
                "The unnamed semaphore initial value of " << m_initialValue << " exceeds the maximum semaphore value "
                                                          << IOX_SEM_VALUE_MAX);
        return err(SemaphoreError::SEMAPHORE_OVERFLOW);
    }

    uninitializedSemaphore.emplace();

    auto result = IOX_POSIX_CALL(iox_sem_init)(&uninitializedSemaphore.value().m_handle,
                                               (m_isInterProcessCapable) ? 1 : 0,
                                               static_cast<unsigned int>(m_initialValue))
                      .failureReturnValue(-1)
                      .evaluate();

    if (result.has_error())
    {
        uninitializedSemaphore.value().m_destroyHandle = false;
        uninitializedSemaphore.reset();

        switch (result.error().errnum)
        {
        case EINVAL:
            IOX_LOG(ERROR, "The initial value of " << m_initialValue << " exceeds " << IOX_SEM_VALUE_MAX);
            break;
        case ENOSYS:
            IOX_LOG(ERROR, "The system does not support process-shared semaphores");
            break;
        default:
            IOX_LOG(ERROR, "This should never happen. An unknown error occurred.");
            break;
        }
    }

    return ok();
}

UnnamedSemaphore::~UnnamedSemaphore() noexcept
{
    if (m_destroyHandle)
    {
        auto result = IOX_POSIX_CALL(iox_sem_destroy)(getHandle()).failureReturnValue(-1).evaluate();
        if (result.has_error())
        {
            switch (result.error().errnum)
            {
            case EINVAL:
                IOX_LOG(ERROR, "The semaphore handle was no longer valid. This can indicate a corrupted system.");
                break;
            default:
                IOX_LOG(ERROR, "This should never happen. An unknown error occurred.");
                break;
            }
        }
    }
}

iox_sem_t* UnnamedSemaphore::getHandle() noexcept
{
    return &m_handle;
}
} // namespace iox
