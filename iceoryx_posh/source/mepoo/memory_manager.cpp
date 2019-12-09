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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <algorithm>
#include <cstdint>

namespace iox
{
namespace mepoo
{
void MemoryManager::printMemPoolVector() const
{
    for (auto& l_mempool : m_memPoolVector)
    {
        std::cerr << "  MemPool [ ChunkSize = " << l_mempool.getChunkSize()
                  << ", PayloadSize = " << l_mempool.getChunkSize() - sizeof(ChunkHeader)
                  << ", ChunkCount = " << l_mempool.getChunkCount() << " ]" << std::endl;
    }
}

void MemoryManager::addMemPool(posix::Allocator* f_managementAllocator,
                               posix::Allocator* f_payloadAllocator,
                               const cxx::greater_or_equal<uint32_t, MemPool::MEMORY_ALIGNMENT> f_payloadSize,
                               const cxx::greater_or_equal<uint32_t, 1> f_numberOfChunks)
{
    uint32_t adjustedChunkSize = sizeWithChunkHeaderStruct(static_cast<uint32_t>(f_payloadSize));
    if (m_denyAddMemPool)
    {
        std::cerr
            << "\nAfter the generation of the chunk management pool you are not allowed to create new mempools.\n";
        errorHandler(Error::kMEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL);
    }
    else if (m_memPoolVector.size() > 0 && adjustedChunkSize <= m_memPoolVector.back().getChunkSize())
    {
        std::cerr << "The following mempools were already added to the mempool "
                     "handler:"
                  << std::endl;
        printMemPoolVector();
        std::cerr << "\nThese mempools must be added in an increasing chunk "
                     "size ordering. The newly added"
                  << " MemPool [ ChunkSize = " << adjustedChunkSize << ", PayloadSize = " << f_payloadSize
                  << ", ChunkCount = " << f_numberOfChunks << "] breaks that requirement!" << std::endl;
        errorHandler(Error::kMEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE);
    }

    m_memPoolVector.emplace_back(adjustedChunkSize, f_numberOfChunks, f_managementAllocator, f_payloadAllocator);
    m_totalNumberOfChunks += f_numberOfChunks;
}

void MemoryManager::generateChunkManagementPool(posix::Allocator* f_managementAllocator)
{
    m_denyAddMemPool = true;
    uint32_t chunkSize = sizeof(ChunkManagement);
    m_chunkManagementPool.emplace_back(chunkSize, m_totalNumberOfChunks, f_managementAllocator, f_managementAllocator);
}

uint32_t MemoryManager::getNumberOfMemPools() const
{
    return static_cast<uint32_t>(m_memPoolVector.size());
}

MemPoolInfo MemoryManager::getMemPoolInfo(uint32_t index) const
{
    if (index >= m_memPoolVector.size())
    {
        return {0, 0, 0, 0};
    }
    return m_memPoolVector[index].getInfo();
}

uint32_t MemoryManager::getMempoolChunkSizeForPayloadSize(const uint32_t f_size) const
{
    uint32_t adjustedSize = MemoryManager::sizeWithChunkHeaderStruct(f_size);
    for (auto& memPool : m_memPoolVector)
    {
        const auto chunkSize = memPool.getChunkSize();
        if (chunkSize >= adjustedSize)
        {
            return chunkSize;
        }
    }

    return 0;
}

uint32_t MemoryManager::sizeWithChunkHeaderStruct(const MaxSize_t f_size)
{
    return f_size + static_cast<uint32_t>(sizeof(ChunkHeader));
}

uint64_t MemoryManager::requiredChunkMemorySize(const MePooConfig& f_mePooConfig)
{
    uint64_t memorySize{0};
    for (const auto& mempool : f_mePooConfig.m_mempoolConfig)
    {
        memorySize +=
            static_cast<uint64_t>(mempool.m_chunkCount) * MemoryManager::sizeWithChunkHeaderStruct(mempool.m_size);
    }
    return memorySize;
}

uint64_t MemoryManager::requiredManagementMemorySize(const MePooConfig& f_mePooConfig)
{
    uint64_t memorySize{0};
    uint32_t sumOfAllChunks{0};
    for (const auto& mempool : f_mePooConfig.m_mempoolConfig)
    {
        sumOfAllChunks += mempool.m_chunkCount;
        memorySize += cxx::align(MemPool::freeList_t::requiredMemorySize(mempool.m_chunkCount), 32ul);
    }

    memorySize += sumOfAllChunks * sizeof(ChunkManagement);
    memorySize += cxx::align(MemPool::freeList_t::requiredMemorySize(sumOfAllChunks), 32ul);

    return memorySize;
}

uint64_t MemoryManager::requiredFullMemorySize(const MePooConfig& f_mePooConfig)
{
    return requiredManagementMemorySize(f_mePooConfig) + requiredChunkMemorySize(f_mePooConfig);
}

void MemoryManager::configureMemoryManager(const MePooConfig& f_mePooConfig,
                                           posix::Allocator* f_managementAllocator,
                                           posix::Allocator* f_payloadAllocator)
{
    for (auto entry : f_mePooConfig.m_mempoolConfig)
    {
        addMemPool(f_managementAllocator, f_payloadAllocator, entry.m_size, entry.m_chunkCount);
    }

    generateChunkManagementPool(f_managementAllocator);
}

SharedChunk MemoryManager::getChunk(const MaxSize_t f_size)
{
    void* chunk{nullptr};
    MemPool* memPoolPointer{nullptr};
    uint32_t adjustedSize = MemoryManager::sizeWithChunkHeaderStruct(f_size);

    for (auto& memPool : m_memPoolVector)
    {
        if (memPool.getChunkSize() >= adjustedSize)
        {
            chunk = memPool.getChunk();
            memPoolPointer = &memPool;
            break;
        }
    }

    if (memPoolPointer == nullptr)
    {
        std::cerr << "The following mempools are available:" << std::endl;
        printMemPoolVector();
        std::cerr << "\nCould not find a fitting mempool for a chunk of size " << adjustedSize << std::endl;

        errorHandler(Error::kMEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE);
        return SharedChunk(nullptr);
    }
    else if (chunk == nullptr)
    {
        std::cerr << "MemoryManager: unable to acquire a chunk with a payload size of " << f_size << std::endl;
        std::cerr << "The following mempools are available:" << std::endl;
        printMemPoolVector();
        return SharedChunk(nullptr);
    }
    else
    {
        new (chunk) ChunkHeader();
        static_cast<ChunkHeader*>(chunk)->m_info.m_payloadSize = f_size;
        static_cast<ChunkHeader*>(chunk)->m_info.m_usedSizeOfChunk = adjustedSize;
        ChunkManagement* chunkManagement = static_cast<ChunkManagement*>(m_chunkManagementPool[0].getChunk());
        new (chunkManagement)
            ChunkManagement(static_cast<ChunkHeader*>(chunk), memPoolPointer, &m_chunkManagementPool[0]);
        return SharedChunk(chunkManagement);
    }
}
} // namespace mepoo
} // namespace iox
