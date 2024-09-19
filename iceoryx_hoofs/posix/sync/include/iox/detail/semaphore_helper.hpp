// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2024 by ekxide IO GmbH. All rights reserved.
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

#ifndef IOX_HOOFS_POSIX_SYNC_SEMAPHORE_HELPER_HPP
#define IOX_HOOFS_POSIX_SYNC_SEMAPHORE_HELPER_HPP

#include "iceoryx_platform/semaphore.hpp"
#include "iox/duration.hpp"
#include "iox/expected.hpp"
#include "iox/semaphore_interface.hpp"

namespace iox
{
namespace detail
{
/// @brief Increments the semaphore by one
/// @return Fails when the value of the semaphore overflows or when the
///         semaphore was removed from outside the process
expected<void, SemaphoreError> sem_post(iox_sem_t* handle) noexcept;

/// @brief Decrements the semaphore by one. When the semaphore value is zero
///        it blocks until the semaphore value is greater zero
/// @return Fails when semaphore was removed from outside the process
expected<void, SemaphoreError> sem_wait(iox_sem_t* handle) noexcept;

/// @brief Tries to decrement the semaphore by one. When the semaphore value is zero
///        it returns false otherwise it returns true and decrement the value by one.
/// @return Fails when semaphore was removed from outside the process
expected<bool, SemaphoreError> sem_try_wait(iox_sem_t* handle) noexcept;

/// @brief Tries to decrement the semaphore by one. When the semaphore value is zero
///        it waits until the timeout has passed.
/// @return If during the timeout time the semaphore value increases to non zero
///         it returns SemaphoreWaitState::NO_TIMEOUT and decreases the semaphore by one
///         otherwise returns SemaphoreWaitState::TIMEOUT
expected<SemaphoreWaitState, SemaphoreError> sem_timed_wait(iox_sem_t* handle, const units::Duration& timeout) noexcept;

} // namespace detail
} // namespace iox

#endif // IOX_HOOFS_POSIX_SYNC_SEMAPHORE_HELPER_HPP
