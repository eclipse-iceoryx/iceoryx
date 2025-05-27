
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

#ifndef IOX_HOOFS_CONCURRENT_SYNC_SPIN_SEMAPHORE_HPP
#define IOX_HOOFS_CONCURRENT_SYNC_SPIN_SEMAPHORE_HPP

#include "iox/atomic.hpp"
#include "iox/deadline_timer.hpp"
#include "iox/detail/adaptive_wait.hpp"
#include "iox/optional.hpp"
#include "iox/semaphore_interface.hpp"
#include "iox/spin_lock.hpp"

namespace iox
{
namespace concurrent
{
class SpinSemaphoreBuilder;

class SpinSemaphore : public detail::SemaphoreInterface<SpinSemaphore>
{
  public:
    using Builder = SpinSemaphoreBuilder;

    SpinSemaphore(const SpinSemaphore&) = delete;
    SpinSemaphore(SpinSemaphore&&) = delete;
    SpinSemaphore& operator=(const SpinSemaphore&) = delete;
    SpinSemaphore& operator=(SpinSemaphore&&) = delete;

    ~SpinSemaphore() noexcept;

  private:
    friend class optional<SpinSemaphore>;
    friend class detail::SemaphoreInterface<SpinSemaphore>;

    explicit SpinSemaphore(int32_t initial_value) noexcept;

    expected<void, SemaphoreError> post_impl() noexcept;

    expected<void, SemaphoreError> wait_impl() noexcept;

    expected<bool, SemaphoreError> try_wait_impl() noexcept;

    expected<SemaphoreWaitState, SemaphoreError> timed_wait_impl(const units::Duration& timeout) noexcept;

  private:
    concurrent::Atomic<int32_t> m_count{0};
    concurrent::Atomic<bool> m_to_be_destroyed{false};
    optional<concurrent::SpinLock> m_spinlock;
};

class SpinSemaphoreBuilder
{
    /// @brief Set the initial value of the spin semaphore
    IOX_BUILDER_PARAMETER(uint32_t, initialValue, 0U)

    /// @brief Set if the spin semaphore can be stored in the shared memory
    ///        for inter process usage
    IOX_BUILDER_PARAMETER(bool, isInterProcessCapable, true)

  public:
    /// @brief Create a spin semaphore
    /// @param[in] uninitializedSemaphore since the semaphore is not movable the user has to provide
    ///            memory to store the semaphore into - packed in an optional
    /// @return an error describing the failure or success
    expected<void, SemaphoreError> create(optional<SpinSemaphore>& uninitializedSemaphore) const noexcept;
};

} // namespace concurrent
} // namespace iox

#endif // IOX_HOOFS_CONCURRENT_SYNC_SPIN_LOCK_HPP
