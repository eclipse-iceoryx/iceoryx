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
#ifndef IOX_HOOFS_POSIX_WRAPPER_UNNAMED_SEMAPHORE_HPP
#define IOX_HOOFS_POSIX_WRAPPER_UNNAMED_SEMAPHORE_HPP

#include "iceoryx_hoofs/cxx/expected.hpp"
#include "iceoryx_hoofs/cxx/optional.hpp"
#include "iceoryx_hoofs/design_pattern/builder.hpp"
#include "iceoryx_hoofs/posix_wrapper/semaphore.hpp"

namespace iox
{
namespace posix
{
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

    uint32_t getValue() const noexcept;
    cxx::expected<uint32_t, SemaphoreError> getValueUnsafe() const noexcept;

    void timedWait() noexcept;
    cxx::expected<SemaphoreError> timedWaitUnsafe() noexcept;

    void tryWait() noexcept;
    cxx::expected<SemaphoreError> tryWaitUnsafe() noexcept;

  protected:
    SemaphoreInterface() noexcept = default;
};

class UnnamedSemaphore : SemaphoreInterface<UnnamedSemaphore>
{
  public:
    UnnamedSemaphore(const UnnamedSemaphore&) noexcept = delete;
    UnnamedSemaphore(UnnamedSemaphore&&) noexcept = delete;
    UnnamedSemaphore& operator=(const UnnamedSemaphore&) noexcept = delete;
    UnnamedSemaphore& operator=(UnnamedSemaphore&&) noexcept = delete;
    ~UnnamedSemaphore() noexcept;

    friend class UnnamedSemaphoreBuilder;
    friend class iox::cxx::optional<UnnamedSemaphore>;
    friend class SemaphoreInterface<UnnamedSemaphore>;

  private:
    UnnamedSemaphore() noexcept = default;
    iox_sem_t* getHandle() noexcept;
    const iox_sem_t* getHandle() const noexcept;

    iox_sem_t m_handle;
    bool m_destroyHandle = true;
};

class UnnamedSemaphoreBuilder
{
    IOX_BUILDER_PARAMETER(uint32_t, initialValue, 0U)
    IOX_BUILDER_PARAMETER(bool, isInterProcessCapable, true)

  public:
    cxx::expected<SemaphoreError> create(cxx::optional<UnnamedSemaphore>& uninitializedSemaphore) noexcept;
};
} // namespace posix
} // namespace iox

#endif
