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

#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"

#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <algorithm>

namespace iox
{
namespace mepoo
{
MemPool::MemPool(const cxx::greater_or_equal<uint32_t, MEMORY_ALIGNMENT> f_chunkSize,
                 const cxx::greater_or_equal<uint32_t, 1> f_numberOfChunks,
                 posix::Allocator* f_managementAllocator,
                 posix::Allocator* f_payloadAllocator)
    : m_chunkSize(f_chunkSize)
    , m_numberOfChunks(f_numberOfChunks)
    , m_minFree(f_numberOfChunks)
{
    if (isMultipleOf32(f_chunkSize))
    {
        m_rawMemory =
            static_cast<uint8_t*>(f_payloadAllocator->allocate(static_cast<uint64_t>(m_numberOfChunks) * m_chunkSize));
        auto memoryLoFFLi =
            static_cast<uint32_t*>(f_managementAllocator->allocate(freeList_t::requiredMemorySize(m_numberOfChunks)));
        m_freeIndices.init(memoryLoFFLi, m_numberOfChunks);
    }
    else
    {
        std::cerr << f_chunkSize << " :: " << f_numberOfChunks << std::endl;
        errorHandler(Error::kMEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_LARGER_32_AND_MULTIPLE_OF_32);
    }
}

bool MemPool::isMultipleOf32(const uint32_t value) const
{
    return (value % 32 == 0);
}

void MemPool::adjustMinFree()
{
    // @todo rethink the concurrent change that can happen. do we need a CAS loop?
    m_minFree.store(std::min(m_numberOfChunks - m_usedChunks.load(std::memory_order_relaxed),
                             m_minFree.load(std::memory_order_relaxed)));
}

void* MemPool::getChunk()
{
    uint32_t l_index{0};
    if (!m_freeIndices.pop(l_index))
    {
        std::cerr << "Mempool [m_chunkSize = " << m_chunkSize << ", numberOfChunks = " << m_numberOfChunks
                  << ", used_chunks = " << m_usedChunks << " ] has no more space left" << std::endl;
        return nullptr;
    }

    /// @todo: verify that m_usedChunk is not changed during adjustMInFree
    ///         without changing m_minFree
    m_usedChunks.fetch_add(1, std::memory_order_relaxed);
    adjustMinFree();

    return m_rawMemory + l_index * m_chunkSize;
}

void MemPool::freeChunk(const void* chunk)
{
    cxx::Expects(m_rawMemory <= chunk
                 && chunk <= m_rawMemory + (static_cast<uint64_t>(m_chunkSize) * (m_numberOfChunks - 1)));

    auto offset = static_cast<const uint8_t*>(chunk) - m_rawMemory;
    cxx::Expects(offset % m_chunkSize == 0);

    uint32_t index = static_cast<uint32_t>(offset / m_chunkSize);

    if (!m_freeIndices.push(index))
    {
        errorHandler(Error::kPOSH__MEMPOOL_POSSIBLE_DOUBLE_FREE);
    }

    m_usedChunks.fetch_sub(1, std::memory_order_relaxed);
}

uint32_t MemPool::getChunkSize() const
{
    return m_chunkSize;
}

uint32_t MemPool::getChunkCount() const
{
    return m_numberOfChunks;
}

uint32_t MemPool::getUsedChunks() const
{
    return m_usedChunks.load(std::memory_order_relaxed);
}

uint32_t MemPool::getMinFree() const
{
    return m_minFree.load(std::memory_order_relaxed);
}

MemPoolInfo MemPool::getInfo() const
{
    return {m_usedChunks.load(std::memory_order_relaxed),
            m_minFree.load(std::memory_order_relaxed),
            m_numberOfChunks,
            m_chunkSize};
}

} // namespace mepoo
} // namespace iox
