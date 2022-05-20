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

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/internal/units/duration.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"

namespace iox
{
namespace posix
{
namespace internal
{
struct SemaphoreState
{
    uint32_t value = 0U;
    uint32_t numberOfBlockedWait = 0U;
};

template <typename SemaphoreChild>
class SemaphoreInterface
{
  public:
    SemaphoreInterface(const SemaphoreInterface&) noexcept = delete;
    SemaphoreInterface(SemaphoreInterface&&) noexcept = delete;
    SemaphoreInterface& operator=(const SemaphoreInterface&) noexcept = delete;
    SemaphoreInterface& operator=(SemaphoreInterface&&) noexcept = delete;
    ~SemaphoreInterface() noexcept = default;

    void post() noexcept;
    cxx::expected<SemaphoreError> postUnsafe() noexcept;

    SemaphoreState getState() noexcept;
    cxx::expected<SemaphoreState, SemaphoreError> getStateUnsafe() noexcept;

    SemaphoreWaitState timedWait(const units::Duration& timeout) noexcept;
    cxx::expected<SemaphoreWaitState, SemaphoreError> timedWaitUnsafe(const units::Duration& timeout) noexcept;

    bool tryWait() noexcept;
    cxx::expected<bool, SemaphoreError> tryWaitUnsafe() noexcept;

  protected:
    SemaphoreInterface() noexcept = default;

  private:
    iox_sem_t* getHandle() noexcept;
};
} // namespace internal
} // namespace posix
} // namespace iox

#endif
