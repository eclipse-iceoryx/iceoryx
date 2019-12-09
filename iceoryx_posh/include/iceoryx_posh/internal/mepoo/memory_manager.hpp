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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <cstdint>
#include <limits>

namespace iox
{
namespace mepoo
{
struct MePooConfig;

class MemoryManager
{
    using MaxSize_t = cxx::range<uint32_t, 1, std::numeric_limits<uint32_t>::max() - sizeof(ChunkHeader)>;

  public:
    MemoryManager() = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    MemoryManager& operator=(MemoryManager&&) = delete;
    ~MemoryManager() = default;

    void configureMemoryManager(const MePooConfig& f_mePooConfig,
                                posix::Allocator* f_managementAllocator,
                                posix::Allocator* f_payloadAllocator);

    SharedChunk getChunk(const MaxSize_t f_size);

    uint32_t getMempoolChunkSizeForPayloadSize(const uint32_t f_size) const;

    uint32_t getNumberOfMemPools() const;

    MemPoolInfo getMemPoolInfo(uint32_t f_index) const;

    static uint32_t sizeWithChunkHeaderStruct(const MaxSize_t f_size);

    static uint64_t requiredChunkMemorySize(const MePooConfig& f_mePooConfig);
    static uint64_t requiredManagementMemorySize(const MePooConfig& f_mePooConfig);
    static uint64_t requiredFullMemorySize(const MePooConfig& f_mePooConfig);

  private:
    void printMemPoolVector() const;
    void addMemPool(posix::Allocator* f_managementAllocator,
                    posix::Allocator* f_payloadAllocator,
                    const cxx::greater_or_equal<uint32_t, MemPool::MEMORY_ALIGNMENT> f_payloadSize,
                    const cxx::greater_or_equal<uint32_t, 1> f_numberOfChunks);
    void generateChunkManagementPool(posix::Allocator* f_managementAllocator);

  private:
    bool m_denyAddMemPool{false};
    uint32_t m_totalNumberOfChunks{0};

    cxx::vector<MemPool, MAX_NUMBER_OF_MEMPOOLS> m_memPoolVector;
    cxx::vector<MemPool, 1> m_chunkManagementPool;
};

} // namespace mepoo
} // namespace iox

