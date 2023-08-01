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
#ifndef IOX_POSH_ROUDI_MEMORY_MEMPOOL_COLLECTION_MEMORY_BLOCK_HPP
#define IOX_POSH_ROUDI_MEMORY_MEMPOOL_COLLECTION_MEMORY_BLOCK_HPP

#include "iceoryx_posh/roudi/memory/memory_block.hpp"

#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "iox/not_null.hpp"
#include "iox/optional.hpp"

#include <cstdint>

namespace iox
{
namespace mepoo
{
class MemoryManager;
}

namespace roudi
{
/// @brief The MemPoolCollectionMemoryBlock is an implementation of a MemoryBlock for a MemPool MemoryManager.
class MemPoolCollectionMemoryBlock final : public MemoryBlock
{
  public:
    MemPoolCollectionMemoryBlock(const mepoo::MePooConfig& memPoolConfig) noexcept;
    ~MemPoolCollectionMemoryBlock() noexcept;

    MemPoolCollectionMemoryBlock(const MemPoolCollectionMemoryBlock&) = delete;
    MemPoolCollectionMemoryBlock(MemPoolCollectionMemoryBlock&&) = delete;
    MemPoolCollectionMemoryBlock& operator=(const MemPoolCollectionMemoryBlock&) = delete;
    MemPoolCollectionMemoryBlock& operator=(MemPoolCollectionMemoryBlock&&) = delete;

    /// @copydoc MemoryBlock::size
    /// @note The size for all the MemPools
    uint64_t size() const noexcept override;

    /// @copydoc MemoryBlock::alignment
    /// @note The memory alignment for the MemPools
    uint64_t alignment() const noexcept override;

    /// @brief This function enables the access to the MemoryManager for the MemPools
    /// @return an optional pointer to the underlying type, nullopt_t if value is not initialized
    optional<mepoo::MemoryManager*> memoryManager() const noexcept;

  protected:
    /// @copydoc MemoryBlock::onMemoryAvailable
    /// @note This will create the MemPools at the location 'memory' points to
    void onMemoryAvailable(not_null<void*> memory) noexcept override;

    /// @copydoc MemoryBlock::destroy
    /// @note This will clean up the MemPools
    void destroy() noexcept override;

  private:
    mepoo::MePooConfig m_memPoolConfig;
    mepoo::MemoryManager* m_memoryManager{nullptr};
};

} // namespace roudi
} // namespace iox

#endif // IOX_POSH_ROUDI_MEMORY_MEMPOOL_COLLECTION_MEMORY_BLOCK_HPP
