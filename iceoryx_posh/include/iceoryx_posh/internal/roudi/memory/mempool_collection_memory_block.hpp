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

#include "iceoryx_posh/roudi/memory/memory_block.hpp"

#include "iceoryx_posh/mepoo/mepoo_config.hpp"

#include "iceoryx_utils/cxx/optional.hpp"

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

    /// @brief Implementation of MemoryBlock::size
    /// @return the size of type T
    uint64_t size() const noexcept override;

    /// @brief Implementation of MemoryBlock::alignment
    /// @return the alignment of type T
    uint64_t alignment() const noexcept override;

    /// @brief Implementation of MemoryBlock::memoryAvailable
    /// This will create the MemPools
    /// @param [in] memory pointer to a valid memory location to place the mempools
    void memoryAvailable(void* memory) noexcept override;

    /// @brief Implementation of MemoryBlock::destroy
    /// This will clean up the MemPools
    void destroy() noexcept override;

    /// @brief This function enables the access to the MemoryManager for the MemPools
    /// @return an optional pointer to the underlying type, cxx::nullopt_t if value is not initialized
    cxx::optional<mepoo::MemoryManager*> memoryManager() const noexcept;

  private:
    mepoo::MePooConfig m_memPoolConfig;
    mepoo::MemoryManager* m_memoryManager{nullptr};
};

} // namespace roudi
} // namespace iox
