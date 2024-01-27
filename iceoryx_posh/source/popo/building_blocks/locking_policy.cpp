// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/logging.hpp"

namespace iox
{
namespace popo
{
ThreadSafePolicy::ThreadSafePolicy() noexcept
{
    MutexBuilder()
        .isInterProcessCapable(true)
        .mutexType(MutexType::RECURSIVE)
        .create(m_mutex)
        .expect("Failed to create Mutex");
}

void ThreadSafePolicy::lock() const noexcept
{
    if (!m_mutex->lock())
    {
        IOX_LOG(FATAL,
                "Locking of an inter-process mutex failed! This indicates that the application holding the lock "
                "was terminated or the resources were cleaned up by RouDi due to an unresponsive application.");
        IOX_REPORT_FATAL(PoshError::POPO__CHUNK_LOCKING_ERROR);
    }
}

void ThreadSafePolicy::unlock() const noexcept
{
    if (!m_mutex->unlock())
    {
        IOX_LOG(FATAL,
                "Unlocking of an inter-process mutex failed! This indicates that the resources were cleaned up "
                "by RouDi due to an unresponsive application.");
        IOX_REPORT_FATAL(PoshError::POPO__CHUNK_UNLOCKING_ERROR);
    }
}

bool ThreadSafePolicy::tryLock() const noexcept
{
    auto tryLockResult = m_mutex->try_lock();
    if (tryLockResult.has_error())
    {
        IOX_REPORT_FATAL(PoshError::POPO__CHUNK_TRY_LOCK_ERROR);
    }
    return *tryLockResult == MutexTryLock::LOCK_SUCCEEDED;
}

void SingleThreadedPolicy::lock() const noexcept
{
}

void SingleThreadedPolicy::unlock() const noexcept
{
}

bool SingleThreadedPolicy::tryLock() const noexcept
{
    return true;
}

} // namespace popo
} // namespace iox
