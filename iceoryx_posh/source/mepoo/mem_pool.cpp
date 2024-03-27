// Copyright (c) 2019 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
// Copyright (c) 2023 by ekxide IO GmbH. All rights reserved.
// Copyright (c) 2024 by Bartlomiej Kozaryna <kozarynabartlomiej@gmail.com>. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/assertions.hpp"

#include <algorithm>

namespace iox
{
namespace mepoo
{
MemPoolInfo::MemPoolInfo(const uint32_t usedChunks,
                         const uint32_t minFreeChunks,
                         const uint32_t numChunks,
                         const uint64_t chunkSize) noexcept
    : m_usedChunks(usedChunks)
    , m_minFreeChunks(minFreeChunks)
    , m_numChunks(numChunks)
    , m_chunkSize(chunkSize)
{
}

constexpr uint64_t MemPool::CHUNK_MEMORY_ALIGNMENT;

MemPool::MemPool(const greater_or_equal<uint64_t, CHUNK_MEMORY_ALIGNMENT> chunkSize,
                 const greater_or_equal<uint32_t, 1> numberOfChunks,
                 iox::BumpAllocator& managementAllocator,
                 iox::BumpAllocator& chunkMemoryAllocator) noexcept
    : m_chunkSize(chunkSize)
    , m_numberOfChunks(numberOfChunks)
    , m_minFree(numberOfChunks)
{
    if (isMultipleOfAlignment(chunkSize))
    {
        IOX_ENFORCE(m_chunkSize <= std::numeric_limits<uint64_t>::max() / m_numberOfChunks,
                    "Chunk size * number of chunks must not exceed the maximum value of uint64_t!");

        m_rawMemory = static_cast<uint8_t*>(
            chunkMemoryAllocator.allocate(static_cast<uint64_t>(m_numberOfChunks) * m_chunkSize, CHUNK_MEMORY_ALIGNMENT)
                .expect("Allocating raw memory for 'MemPool'"));

        auto* memoryFreeList =
            managementAllocator.allocate(freeList_t::requiredIndexMemorySize(m_numberOfChunks), CHUNK_MEMORY_ALIGNMENT)
                .expect("Allocating free list memory for 'MemPool'");
        m_freeIndices.init(static_cast<freeList_t::Index_t*>(memoryFreeList), m_numberOfChunks);
    }
    else
    {
        IOX_LOG(FATAL,
                "Chunk size must be multiple of '" << CHUNK_MEMORY_ALIGNMENT << "'! Requested size is " << chunkSize
                                                   << " for " << numberOfChunks << " chunks!");
        IOX_REPORT_FATAL(PoshError::MEPOO__MEMPOOL_CHUNKSIZE_MUST_BE_MULTIPLE_OF_CHUNK_MEMORY_ALIGNMENT);
    }
}

bool MemPool::isMultipleOfAlignment(const uint64_t value) const noexcept
{
    return (value % CHUNK_MEMORY_ALIGNMENT == 0U);
}

void MemPool::adjustMinFree() noexcept
{
    // @todo iox-#1714 rethink the concurrent change that can happen. do we need a CAS loop?
    m_minFree.store(std::min(m_numberOfChunks - m_usedChunks.load(std::memory_order_relaxed),
                             m_minFree.load(std::memory_order_relaxed)));
}

void* MemPool::getChunk() noexcept
{
    uint32_t index{0U};
    if (!m_freeIndices.pop(index))
    {
        IOX_LOG(WARN,
                "Mempool [m_chunkSize = " << m_chunkSize << ", numberOfChunks = " << m_numberOfChunks
                                          << ", used_chunks = " << m_usedChunks.load() << " ] has no more space left");
        return nullptr;
    }

    /// @todo iox-#1714 verify that m_usedChunk is not changed during adjustMInFree
    ///         without changing m_minFree
    m_usedChunks.fetch_add(1U, std::memory_order_relaxed);
    adjustMinFree();

    return indexToPointer(index, m_chunkSize, m_rawMemory.get());
}

void* MemPool::indexToPointer(uint32_t index, uint64_t chunkSize, void* const rawMemoryBase) noexcept
{
    const auto offset = static_cast<uint64_t>(index) * chunkSize;
    return static_cast<void*>(static_cast<uint8_t*>(rawMemoryBase) + offset);
}

uint32_t
MemPool::pointerToIndex(const void* const chunk, const uint64_t chunkSize, const void* const rawMemoryBase) noexcept
{
    const auto offset =
        static_cast<uint64_t>(static_cast<const uint8_t*>(chunk) - static_cast<const uint8_t*>(rawMemoryBase));
    if (offset % chunkSize != 0)
    {
        IOX_LOG(FATAL,
                "Trying to convert a pointer to an index which is not aligned to the array! Base address: "
                    << iox::log::hex(rawMemoryBase) << "; item size: " << chunkSize
                    << "; pointer address: " << iox::log::hex(chunk));
        IOX_PANIC("Invalid access");
    }

    const auto index = static_cast<uint32_t>(offset / chunkSize);
    return index;
}

void MemPool::freeChunk(const void* chunk) noexcept
{
    const auto memPoolStartAddress = m_rawMemory.get();
    const auto offsetToLastChunk = m_chunkSize * (m_numberOfChunks - 1U);
    if (chunk < memPoolStartAddress)
    {
        IOX_LOG(FATAL,
                "Try to free chunk with address " << iox::log::hex(chunk) << " while the memory pool starts at address "
                                                  << iox::log::hex(memPoolStartAddress));
        IOX_PANIC("Invalid chunk to free");
    }

    if (chunk > static_cast<uint8_t*>(memPoolStartAddress) + offsetToLastChunk)
    {
        IOX_LOG(FATAL,
                "Try to free chunk with address " << iox::log::hex(chunk)
                                                  << " while the last valid memory pool address is "
                                                  << iox::log::hex(memPoolStartAddress));
        IOX_PANIC("Invalid chunk to free");
    }

    const auto index = pointerToIndex(chunk, m_chunkSize, memPoolStartAddress);

    if (!m_freeIndices.push(index))
    {
        IOX_REPORT_FATAL(PoshError::POSH__MEMPOOL_POSSIBLE_DOUBLE_FREE);
    }

    m_usedChunks.fetch_sub(1U, std::memory_order_relaxed);
}

uint64_t MemPool::getChunkSize() const noexcept
{
    return m_chunkSize;
}

uint32_t MemPool::getChunkCount() const noexcept
{
    return m_numberOfChunks;
}

uint32_t MemPool::getUsedChunks() const noexcept
{
    return m_usedChunks.load(std::memory_order_relaxed);
}

uint32_t MemPool::getMinFree() const noexcept
{
    return m_minFree.load(std::memory_order_relaxed);
}

MemPoolInfo MemPool::getInfo() const noexcept
{
    return {m_usedChunks.load(std::memory_order_relaxed),
            m_minFree.load(std::memory_order_relaxed),
            m_numberOfChunks,
            m_chunkSize};
}

} // namespace mepoo
} // namespace iox
