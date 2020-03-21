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

#include "iceoryx_posh/internal/mepoo/segment_manager.hpp"
#include "iceoryx_posh/mepoo/segment_config.hpp"

#include "iceoryx_utils/cxx/optional.hpp"

#include <cstdint>

namespace iox
{
namespace roudi
{
class MemPoolSegmentManagerMemoryBlock : public MemoryBlock
{
  public:
    MemPoolSegmentManagerMemoryBlock(const mepoo::SegmentConfig& segmentConfig) noexcept;
    ~MemPoolSegmentManagerMemoryBlock() noexcept;

    MemPoolSegmentManagerMemoryBlock(const MemPoolSegmentManagerMemoryBlock&) = delete;
    MemPoolSegmentManagerMemoryBlock(MemPoolSegmentManagerMemoryBlock&&) = delete;
    MemPoolSegmentManagerMemoryBlock& operator=(const MemPoolSegmentManagerMemoryBlock&) = delete;
    MemPoolSegmentManagerMemoryBlock& operator=(MemPoolSegmentManagerMemoryBlock&&) = delete;

    /// @brief Implementation of MemoryBlock::size
    /// @return the size of for SegmentManager
    uint64_t size() const noexcept override;

    /// @brief Implementation of MemoryBlock::alignment
    /// @return the memory alignment for SegmentManager
    uint64_t alignment() const noexcept override;

    /// @brief Implementation of MemoryBlock::memoryAvailable
    /// This will create the SegmentManager
    /// @param [in] memory pointer to a valid memory location to place the mempools
    void memoryAvailable(void* memory) noexcept override;

    /// @brief Implementation of MemoryBlock::destroy
    /// This will clean up the SegmentManager
    void destroy() noexcept override;

    /// @brief This function enables the access to the SegmentManager
    /// @return an optional pointer to the underlying type, cxx::nullopt_t if value is not initialized
    cxx::optional<mepoo::SegmentManager<>*> segmentManager() const noexcept;

  private:
    mepoo::SegmentManager<>* m_segmentManager{nullptr};
    mepoo::SegmentConfig m_segmentConfig;
};

} // namespace roudi
} // namespace iox
