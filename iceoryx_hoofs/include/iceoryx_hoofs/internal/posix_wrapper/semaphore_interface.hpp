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
#ifndef IOX_HOOFS_POSIX_WRAPPER_SEMAPHORE_INTERFACE_HPP
#define IOX_HOOFS_POSIX_WRAPPER_SEMAPHORE_INTERFACE_HPP

#include "iceoryx_platform/semaphore.hpp"
#include "iox/duration.hpp"
#include "iox/expected.hpp"

namespace iox
{
namespace posix
{
enum class SemaphoreError
{
    INVALID_NAME,
    INVALID_SEMAPHORE_HANDLE,
    SEMAPHORE_OVERFLOW,
    INTERRUPTED_BY_SIGNAL_HANDLER,
    PERMISSION_DENIED,
    ALREADY_EXIST,
    FILE_DESCRIPTOR_LIMIT_REACHED,
    NO_SEMAPHORE_WITH_THAT_NAME_EXISTS,
    OUT_OF_MEMORY,
    UNDEFINED
};

enum class SemaphoreWaitState
{
    TIMEOUT,
    NO_TIMEOUT,
};

namespace internal
{
/// @brief Defines the interface of a named and unnamed semaphore.
template <typename SemaphoreChild>
class SemaphoreInterface
{
  public:
    SemaphoreInterface(const SemaphoreInterface&) noexcept = delete;
    SemaphoreInterface(SemaphoreInterface&&) noexcept = delete;
    SemaphoreInterface& operator=(const SemaphoreInterface&) noexcept = delete;
    SemaphoreInterface& operator=(SemaphoreInterface&&) noexcept = delete;
    ~SemaphoreInterface() noexcept = default;

    /// @brief Increments the semaphore by one
    /// @return Fails when the value of the semaphore overflows or when the
    ///         semaphore was removed from outside the process
    expected<void, SemaphoreError> post() noexcept;

    /// @brief Decrements the semaphore by one. When the semaphore value is zero
    ///        it blocks until the semaphore value is greater zero
    /// @return Fails when semaphore was removed from outside the process
    expected<void, SemaphoreError> wait() noexcept;

    /// @brief Tries to decrement the semaphore by one. When the semaphore value is zero
    ///        it returns false otherwise it returns true and decrement the value by one.
    /// @return Fails when semaphore was removed from outside the process
    expected<bool, SemaphoreError> tryWait() noexcept;

    /// @brief Tries to decrement the semaphore by one. When the semaphore value is zero
    ///        it waits until the timeout has passed.
    /// @return If during the timeout time the semaphore value increases to non zero
    ///         it returns SemaphoreWaitState::NO_TIMEOUT and decreases the semaphore by one
    ///         otherwise returns SemaphoreWaitState::TIMEOUT
    expected<SemaphoreWaitState, SemaphoreError> timedWait(const units::Duration& timeout) noexcept;

  protected:
    SemaphoreInterface() noexcept = default;

  private:
    iox_sem_t* getHandle() noexcept;
};
} // namespace internal
} // namespace posix
} // namespace iox

#endif
