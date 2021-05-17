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
//
// SPDX-License-Identifier: Apache-2.0

#include "iceoryx_posh/internal/popo/building_blocks/locking_policy.hpp"
#include "iceoryx_hoofs/error_handling/error_handling.hpp"

namespace iox
{
namespace popo
{
void ThreadSafePolicy::lock() const noexcept
{
    if (!m_mutex.lock())
    {
        errorHandler(Error::kPOPO__CHUNK_LOCKING_ERROR, nullptr, ErrorLevel::FATAL);
    }
}

void ThreadSafePolicy::unlock() const noexcept
{
    if (!m_mutex.unlock())
    {
        errorHandler(Error::kPOPO__CHUNK_UNLOCKING_ERROR, nullptr, ErrorLevel::FATAL);
    }
}

bool ThreadSafePolicy::tryLock() const noexcept
{
    return m_mutex.try_lock();
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
