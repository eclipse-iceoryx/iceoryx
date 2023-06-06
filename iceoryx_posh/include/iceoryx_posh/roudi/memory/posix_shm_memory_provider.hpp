// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_ROUDI_MEMORY_POSIX_SHM_MEMORY_PROVIDER_HPP
#define IOX_POSH_ROUDI_MEMORY_POSIX_SHM_MEMORY_PROVIDER_HPP

#include "iceoryx_posh/roudi/memory/memory_provider.hpp"

#include "iceoryx_hoofs/internal/posix_wrapper/shared_memory_object.hpp"
#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iox/expected.hpp"
#include "iox/filesystem.hpp"
#include "iox/optional.hpp"
#include "iox/string.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
/// @brief Creates the shared memory based on a provided configuration
class PosixShmMemoryProvider : public MemoryProvider
{
  public:
    /// @brief Constructs a PosixShmMemoryProvider which can be used to request memory via MemoryBlocks
    /// @param [in] shmName is the name of the posix share memory
    /// @param [in] accessMode defines the read and write access to the memory
    /// @param [in] openMode defines the creation/open mode of the shared memory.
    PosixShmMemoryProvider(const ShmName_t& shmName,
                           const posix::AccessMode accessMode,
                           const posix::OpenMode openMode) noexcept;
    ~PosixShmMemoryProvider() noexcept;

    PosixShmMemoryProvider(PosixShmMemoryProvider&&) = delete;
    PosixShmMemoryProvider& operator=(PosixShmMemoryProvider&&) = delete;

    PosixShmMemoryProvider(const PosixShmMemoryProvider&) = delete;
    PosixShmMemoryProvider& operator=(const PosixShmMemoryProvider&) = delete;

  protected:
    /// @copydoc MemoryProvider::createMemory
    /// @note This creates and maps a POSIX shared memory to the address space of the application
    expected<void*, MemoryProviderError> createMemory(const uint64_t size, const uint64_t alignment) noexcept;

    /// @copydoc MemoryProvider::destroyMemory
    /// @note This closes and unmaps a POSIX shared memory
    expected<void, MemoryProviderError> destroyMemory() noexcept;

  private:
    ShmName_t m_shmName;
    posix::AccessMode m_accessMode{posix::AccessMode::READ_ONLY};
    posix::OpenMode m_openMode{posix::OpenMode::OPEN_EXISTING};
    optional<posix::SharedMemoryObject> m_shmObject;

    static constexpr access_rights SHM_MEMORY_PERMISSIONS =
        perms::owner_read | perms::owner_write | perms::group_read | perms::group_write;
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_POSIX_SHM_MEMORY_PROVIDER_HPP
