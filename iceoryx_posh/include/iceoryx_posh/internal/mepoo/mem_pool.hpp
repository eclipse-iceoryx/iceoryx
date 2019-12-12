// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/internal/concurrent/loffli.hpp"
#include "iceoryx_utils/internal/posix_wrapper/shared_memory_object/allocator.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <atomic>
#include <cstdint>


namespace iox
{
namespace mepoo
{
struct MemPoolInfo
{
    MemPoolInfo(uint32_t f_usedChunks, uint32_t f_minFreeChunks, uint32_t f_numChunks, uint32_t f_chunkSize)
        : m_usedChunks(f_usedChunks)
        , m_minFreeChunks(f_minFreeChunks)
        , m_numChunks(f_numChunks)
        , m_chunkSize(f_chunkSize)
    {
    }
    uint32_t m_usedChunks{0};
    uint32_t m_minFreeChunks{0};
    uint32_t m_numChunks{0};
    uint32_t m_chunkSize{0};
};

class MemPool
{
  public:
    using freeList_t = concurrent::LoFFLi;
    static constexpr uint64_t MEMORY_ALIGNMENT = posix::Allocator::MEMORY_ALIGNMENT;

    MemPool(const cxx::greater_or_equal<uint32_t, MEMORY_ALIGNMENT> f_chunkSize,
            const cxx::greater_or_equal<uint32_t, 1> f_numberOfChunks,
            posix::Allocator* f_managementAllocator,
            posix::Allocator* f_payloadAllocator);

    MemPool(const MemPool&) = delete;
    MemPool(MemPool&&) = delete;
    MemPool& operator=(const MemPool&) = delete;
    MemPool& operator=(MemPool&&) = delete;

    void* getChunk();
    uint32_t getChunkSize() const;
    uint32_t getChunkCount() const;
    uint32_t getUsedChunks() const;
    uint32_t getMinFree() const;
    MemPoolInfo getInfo() const;

    void freeChunk(const void* chunk);

  private:
    void adjustMinFree();
    bool isMultipleOf32(const uint32_t value) const;

    relative_ptr<uint8_t> m_rawMemory;

    uint32_t m_chunkSize{0};
    /// needs to be 32 bit since loffli supports only 32 bit numbers
    /// (cas is only 64 bit and we need the other 32 bit for the aba counter)
    uint32_t m_numberOfChunks{0};

    /// @todo: put this into one struct and in a separate class in concurrent.
    std::atomic<uint32_t> m_usedChunks{0};
    std::atomic<uint32_t> m_minFree{0};
    /// @todo: end

    freeList_t m_freeIndices;
};

} // namespace mepoo
} // namespace iox
