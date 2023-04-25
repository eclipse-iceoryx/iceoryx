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

#include "iceoryx_hoofs/internal/posix_wrapper/semaphore_interface.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/optional.hpp"

namespace iox
{
namespace posix
{
/// @brief A unnamed posix semaphore.
/// NOLINTJUSTIFICATION m_handle is always initialized during create in the UnnamedSemaphoreBuilder
///                     hence it is impossible to create a UnnamedSemaphore without an initialized
///                     m_handle
/// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init,hicpp-member-init)
class UnnamedSemaphore final : public internal::SemaphoreInterface<UnnamedSemaphore>
{
  public:
    UnnamedSemaphore(const UnnamedSemaphore&) noexcept = delete;
    UnnamedSemaphore(UnnamedSemaphore&&) noexcept = delete;
    UnnamedSemaphore& operator=(const UnnamedSemaphore&) noexcept = delete;
    UnnamedSemaphore& operator=(UnnamedSemaphore&&) noexcept = delete;
    ~UnnamedSemaphore() noexcept;

  private:
    friend class UnnamedSemaphoreBuilder;
    friend class iox::optional<UnnamedSemaphore>;
    friend class internal::SemaphoreInterface<UnnamedSemaphore>;

    UnnamedSemaphore() noexcept = default;
    iox_sem_t* getHandle() noexcept;

    iox_sem_t m_handle;
    bool m_destroyHandle = true;
};

class UnnamedSemaphoreBuilder
{
    /// @brief Set the initial value of the unnamed posix semaphore
    IOX_BUILDER_PARAMETER(uint32_t, initialValue, 0U)

    /// @brief Set if the unnamed semaphore can be stored in the shared memory
    ///        for inter process usage
    IOX_BUILDER_PARAMETER(bool, isInterProcessCapable, true)

  public:
    /// @brief create an unnamed semaphore
    /// @param[in] uninitializedSemaphore since the semaphore is not movable the user has to provide
    ///            memory to store the semaphore into - packed in an optional
    /// @return an error describing the failure or success
    expected<void, SemaphoreError> create(optional<UnnamedSemaphore>& uninitializedSemaphore) const noexcept;
};
} // namespace posix
} // namespace iox

#endif
