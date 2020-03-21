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

#pragma once

#include "iceoryx_posh/roudi/memory/memory_provider.hpp"

#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/cxx/string.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
using ShmNameString = cxx::string<100>;

/// @brief Creates the shared memory based on a provided configuration
class PosixShmMemoryProvider : public MemoryProvider
{
  public:
    /// @brief Constructs a PosixShmMemoryProvider which can be used to request memory via MemoryBlocks
    /// @param [in] shmName is the name of the posix share memory
    /// @param [in] accessMode defines the read and write access to the memory
    /// @param [in] ownership defines the ownership of the shared memory. "mine" controls the lifetime of the memory and
    /// "openExisting" will just use an already existing shared memory
    PosixShmMemoryProvider(const ShmNameString& shmName,
                           const posix::AccessMode accessMode,
                           const posix::OwnerShip ownership) noexcept;
    ~PosixShmMemoryProvider() noexcept;

    PosixShmMemoryProvider(PosixShmMemoryProvider&&) = delete;
    PosixShmMemoryProvider& operator=(PosixShmMemoryProvider&&) = delete;

    PosixShmMemoryProvider(const PosixShmMemoryProvider&) = delete;
    PosixShmMemoryProvider& operator=(const PosixShmMemoryProvider&) = delete;

  protected:
    /// @brief Implementation of MemoryProvider::createMemory
    /// @param [in] size is the size in bytes for the requested memory, the size should already be calculated according
    /// to the alignment requirements
    /// @param [in] alignment the required alignment for the memory
    /// @return the pointer of the begin of the created memory, nullptr if the memory could not be created
    cxx::expected<void*, MemoryProviderError> createMemory(const uint64_t size, const uint64_t alignment) noexcept;

    /// @brief Implementation of MemoryProvider::destroyMemory
    /// @return a MemoryProviderError if the destruction failed, otherwise success
    cxx::expected<MemoryProviderError> destroyMemory() noexcept;

  private:
    ShmNameString m_shmName;
    posix::AccessMode m_accessMode{posix::AccessMode::readOnly};
    posix::OwnerShip m_ownership{posix::OwnerShip::openExisting};
    cxx::optional<posix::SharedMemoryObject> m_shmObject;
};

} // namespace roudi
} // namespace iox

