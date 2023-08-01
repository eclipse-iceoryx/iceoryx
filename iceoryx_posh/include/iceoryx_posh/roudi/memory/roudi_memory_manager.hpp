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
#ifndef IOX_POSH_ROUDI_MEMORY_ROUDI_MEMORY_MANAGER_HPP
#define IOX_POSH_ROUDI_MEMORY_ROUDI_MEMORY_MANAGER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/roudi/memory/mempool_collection_memory_block.hpp"
#include "iceoryx_posh/internal/roudi/memory/mempool_segment_manager_memory_block.hpp"
#include "iceoryx_posh/internal/roudi/memory/port_pool_memory_block.hpp"
#include "iceoryx_posh/roudi/memory/posix_shm_memory_provider.hpp"

#include "iox/expected.hpp"
#include "iox/optional.hpp"
#include "iox/vector.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class MemoryProvider;

enum class RouDiMemoryManagerError
{
    /// attempt to add more memory provider than the capacity allows
    MEMORY_PROVIDER_EXHAUSTED,
    /// an action was performed which requires memory provider
    NO_MEMORY_PROVIDER_PRESENT,
    /// generic error if memory creation failed
    MEMORY_CREATION_FAILED,
    /// generic error if memory destruction failed
    MEMORY_DESTRUCTION_FAILED,
};

iox::log::LogStream& operator<<(iox::log::LogStream& logstream, const RouDiMemoryManagerError& error) noexcept;

class RouDiMemoryManager
{
  public:
    RouDiMemoryManager() noexcept = default;
    /// @brief The Destructor of the RouDiMemoryManager also calls destroy on the registered MemoryProvider
    virtual ~RouDiMemoryManager() noexcept;

    RouDiMemoryManager(RouDiMemoryManager&&) = delete;
    RouDiMemoryManager& operator=(RouDiMemoryManager&&) = delete;

    RouDiMemoryManager(const RouDiMemoryManager&) = delete;
    RouDiMemoryManager& operator=(const RouDiMemoryManager&) = delete;

    /// @brief This function add a MemoryProvider to the memory manager
    /// @param [in] memoryProvider is a pointer to a user defined MemoryProvider
    /// @return an RouDiMemoryManagerError::MEMORY_PROVIDER_EXHAUSTED error if no further memory provider can be added,
    /// otherwise success
    expected<void, RouDiMemoryManagerError> addMemoryProvider(MemoryProvider* memoryProvider) noexcept;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to create the memory and announce the availability
    /// to its MemoryBlocks
    /// @return an RouDiMemoryManagerError if the MemoryProvider cannot create the memory, otherwise success
    expected<void, RouDiMemoryManagerError> createAndAnnounceMemory() noexcept;

    /// @brief The RouDiMemoryManager calls the the MemoryProvider to destroy the memory, which in turn prompts the
    /// MemoryBlocks to destroy their data
    expected<void, RouDiMemoryManagerError> destroyMemory() noexcept;

  private:
    mepoo::MePooConfig introspectionMemPoolConfig() const noexcept;
    vector<MemoryProvider*, MAX_NUMBER_OF_MEMORY_PROVIDER> m_memoryProvider;
};
} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_ROUDI_MEMORY_MANAGER_HPP
