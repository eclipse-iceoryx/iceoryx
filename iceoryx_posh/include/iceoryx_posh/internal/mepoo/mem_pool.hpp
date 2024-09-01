// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_MEM_POOL_HPP
#define IOX_POSH_MEPOO_MEM_POOL_HPP

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/algorithm.hpp"
#include "iox/atomic.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/detail/mpmc_loffli.hpp"
#include "iox/relative_pointer.hpp"

#include <cstdint>


namespace iox
{
namespace mepoo
{
struct MemPoolInfo
{
    MemPoolInfo(const uint32_t usedChunks,
                const uint32_t minFreeChunks,
                const uint32_t numChunks,
                const uint64_t chunkSize) noexcept;

    uint32_t m_usedChunks{0};
    uint32_t m_minFreeChunks{0};
    uint32_t m_numChunks{0};
    uint64_t m_chunkSize{0};
};

class MemPool
{
  public:
    using freeList_t = concurrent::MpmcLoFFLi;
    static constexpr uint64_t CHUNK_MEMORY_ALIGNMENT = 8U; // default alignment for 64 bit

    MemPool(const greater_or_equal<uint64_t, CHUNK_MEMORY_ALIGNMENT> chunkSize,
            const greater_or_equal<uint32_t, 1> numberOfChunks,
            iox::BumpAllocator& managementAllocator,
            iox::BumpAllocator& chunkMemoryAllocator) noexcept;

    MemPool(const MemPool&) = delete;
    MemPool(MemPool&&) = delete;
    MemPool& operator=(const MemPool&) = delete;
    MemPool& operator=(MemPool&&) = delete;

    void* getChunk() noexcept;
    uint64_t getChunkSize() const noexcept;
    uint32_t getChunkCount() const noexcept;
    uint32_t getUsedChunks() const noexcept;
    uint32_t getMinFree() const noexcept;
    MemPoolInfo getInfo() const noexcept;

    void freeChunk(const void* chunk) noexcept;

    /// @brief Converts an index to a chunk in the MemPool to a pointer
    /// @param[in] index of the chunk
    /// @param[in] chunkSize is the size of the chunk
    /// @param[in] rawMemoryBase it the pointer to the raw memory of the MemPool
    /// @return the pointer to the chunk
    static void* indexToPointer(const uint32_t index, const uint64_t chunkSize, void* const rawMemoryBase) noexcept;

    /// @brief Converts a pointer to a chunk in the MemPool to an index
    /// @param[in] chunk is the pointer to the chunk
    /// @param[in] chunkSize is the size of the chunk
    /// @param[in] rawMemoryBase it the pointer to the raw memory of the MemPool
    /// @return the index to the chunk
    static uint32_t
    pointerToIndex(const void* const chunk, const uint64_t chunkSize, const void* const rawMemoryBase) noexcept;

  private:
    void adjustMinFree() noexcept;
    bool isMultipleOfAlignment(const uint64_t value) const noexcept;

    RelativePointer<void> m_rawMemory;

    uint64_t m_chunkSize{0U};
    /// needs to be 32 bit since loffli supports only 32 bit numbers
    /// (cas is only 64 bit and we need the other 32 bit for the aba counter)
    uint32_t m_numberOfChunks{0U};

    concurrent::Atomic<uint32_t> m_usedChunks{0U};
    concurrent::Atomic<uint32_t> m_minFree{0U};

    freeList_t m_freeIndices;
};

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEM_POOL_HPP
