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
#ifndef IOX_HOOFS_POSIX_WRAPPER_NAMED_SEMAPHORE_HPP
#define IOX_HOOFS_POSIX_WRAPPER_NAMED_SEMAPHORE_HPP

#include "iceoryx_hoofs/internal/posix_wrapper/semaphore_interface.hpp"
#include "iceoryx_hoofs/posix_wrapper/types.hpp"
#include "iceoryx_platform/platform_settings.hpp"
#include "iox/builder.hpp"
#include "iox/expected.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"

namespace iox
{
namespace posix
{
/// @brief A named posix semaphore.
class NamedSemaphore final : public internal::SemaphoreInterface<NamedSemaphore>
{
  public:
    static constexpr uint64_t LENGTH_OF_SEMAPHORE_SLASH_PREFIX = 1U;
    using Name_t = string<platform::IOX_MAX_SEMAPHORE_NAME_LENGTH - LENGTH_OF_SEMAPHORE_SLASH_PREFIX>;

    NamedSemaphore(const NamedSemaphore&) noexcept = delete;
    NamedSemaphore(NamedSemaphore&&) noexcept = delete;
    NamedSemaphore& operator=(const NamedSemaphore&) noexcept = delete;
    NamedSemaphore& operator=(NamedSemaphore&&) noexcept = delete;
    ~NamedSemaphore() noexcept;

  private:
    friend class NamedSemaphoreBuilder;
    friend class iox::optional<NamedSemaphore>;
    friend class internal::SemaphoreInterface<NamedSemaphore>;

    NamedSemaphore(iox_sem_t* handle, const Name_t& name, const bool hasOwnership) noexcept;
    iox_sem_t* getHandle() noexcept;

    iox_sem_t* m_handle = nullptr;
    Name_t m_name;
    bool m_hasOwnership = true;
};

class NamedSemaphoreBuilder
{
    /// @brief Defines the semaphore name
    IOX_BUILDER_PARAMETER(NamedSemaphore::Name_t, name, "")

    /// @brief Defines how the semaphore is opened
    IOX_BUILDER_PARAMETER(OpenMode, openMode, OpenMode::OPEN_EXISTING)

    /// @brief Defines the access permissions of the semaphore
    IOX_BUILDER_PARAMETER(access_rights, permissions, perms::owner_all)

    /// @brief Set the initial value of the unnamed posix semaphore. This value is only used when a new semaphore is
    ///        created.
    IOX_BUILDER_PARAMETER(uint32_t, initialValue, 0U)

  public:
    /// @brief create a named semaphore
    /// @param[in] uninitializedSemaphore since the semaphore is not movable the user has to provide
    ///            memory to store the semaphore into - packed in an optional
    /// @return an error describing the failure or success
    expected<void, SemaphoreError> create(optional<NamedSemaphore>& uninitializedSemaphore) const noexcept;
};
} // namespace posix
} // namespace iox

#endif
