// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/mepoo_config.hpp"
#include "iox/logging.hpp"

#include <cstdint>

namespace iox
{
namespace mepoo
{
void MemoryManager::printMemPoolVector(log::LogStream& log) const noexcept
{
    for (auto& l_mempool : m_memPoolVector)
    {
        log << "  MemPool [ ChunkSize = " << l_mempool.getChunkSize()
            << ", ChunkPayloadSize = " << l_mempool.getChunkSize() - sizeof(ChunkHeader)
            << ", ChunkCount = " << l_mempool.getChunkCount() << " ]";
    }
}

void MemoryManager::addMemPool(BumpAllocator& managementAllocator,
                               BumpAllocator& chunkMemoryAllocator,
                               const greater_or_equal<uint64_t, MemPool::CHUNK_MEMORY_ALIGNMENT> chunkPayloadSize,
                               const greater_or_equal<uint32_t, 1> numberOfChunks) noexcept
{
    uint64_t adjustedChunkSize = sizeWithChunkHeaderStruct(static_cast<uint64_t>(chunkPayloadSize));
    if (m_denyAddMemPool)
    {
        IOX_LOG(FATAL, "After the generation of the chunk management pool you are not allowed to create new mempools.");
        IOX_REPORT_FATAL(iox::PoshError::MEPOO__MEMPOOL_ADDMEMPOOL_AFTER_GENERATECHUNKMANAGEMENTPOOL);
    }
    else if (m_memPoolVector.size() > 0 && adjustedChunkSize <= m_memPoolVector.back().getChunkSize())
    {
        IOX_LOG(
            FATAL,
            "The following mempools were already added to the mempool handler:" << [this](auto& log) -> auto& {
                this->printMemPoolVector(log);
                return log;
            } << "These mempools must be added in an increasing chunk size ordering. The newly added  MemPool [ "
                 "ChunkSize = "
              << adjustedChunkSize << ", ChunkPayloadSize = " << chunkPayloadSize << ", ChunkCount = " << numberOfChunks
              << "] breaks that requirement!");
        IOX_REPORT_FATAL(iox::PoshError::MEPOO__MEMPOOL_CONFIG_MUST_BE_ORDERED_BY_INCREASING_SIZE);
    }

    m_memPoolVector.emplace_back(adjustedChunkSize, numberOfChunks, managementAllocator, chunkMemoryAllocator);
    m_totalNumberOfChunks += numberOfChunks;
}

void MemoryManager::generateChunkManagementPool(BumpAllocator& managementAllocator) noexcept
{
    m_denyAddMemPool = true;
    uint64_t chunkSize = sizeof(ChunkManagement);
    m_chunkManagementPool.emplace_back(chunkSize, m_totalNumberOfChunks, managementAllocator, managementAllocator);
}

uint32_t MemoryManager::getNumberOfMemPools() const noexcept
{
    return static_cast<uint32_t>(m_memPoolVector.size());
}

MemPoolInfo MemoryManager::getMemPoolInfo(const uint32_t index) const noexcept
{
    if (index >= m_memPoolVector.size())
    {
        return {0, 0, 0, 0};
    }
    return m_memPoolVector[index].getInfo();
}

uint64_t MemoryManager::sizeWithChunkHeaderStruct(const MaxChunkPayloadSize_t size) noexcept
{
    return size + sizeof(ChunkHeader);
}

uint64_t MemoryManager::requiredChunkMemorySize(const MePooConfig& mePooConfig) noexcept
{
    uint64_t memorySize{0};
    for (const auto& mempoolConfig : mePooConfig.m_mempoolConfig)
    {
        // for the required chunk memory size only the size of the ChunkHeader
        // and the the chunk-payload size is taken into account;
        // the user has the option to further partition the chunk-payload with
        // a user-header and therefore reduce the user-payload size
        memorySize += align(static_cast<uint64_t>(mempoolConfig.m_chunkCount)
                                * MemoryManager::sizeWithChunkHeaderStruct(mempoolConfig.m_size),
                            MemPool::CHUNK_MEMORY_ALIGNMENT);
    }
    return memorySize;
}

uint64_t MemoryManager::requiredManagementMemorySize(const MePooConfig& mePooConfig) noexcept
{
    uint64_t memorySize{0U};
    uint64_t sumOfAllChunks{0U};
    for (const auto& mempool : mePooConfig.m_mempoolConfig)
    {
        sumOfAllChunks += mempool.m_chunkCount;
        memorySize +=
            align(MemPool::freeList_t::requiredIndexMemorySize(mempool.m_chunkCount), MemPool::CHUNK_MEMORY_ALIGNMENT);
    }

    memorySize += align(sumOfAllChunks * sizeof(ChunkManagement), MemPool::CHUNK_MEMORY_ALIGNMENT);
    memorySize += align(MemPool::freeList_t::requiredIndexMemorySize(sumOfAllChunks), MemPool::CHUNK_MEMORY_ALIGNMENT);

    return memorySize;
}

uint64_t MemoryManager::requiredFullMemorySize(const MePooConfig& mePooConfig) noexcept
{
    return requiredManagementMemorySize(mePooConfig) + requiredChunkMemorySize(mePooConfig);
}

void MemoryManager::configureMemoryManager(const MePooConfig& mePooConfig,
                                           BumpAllocator& managementAllocator,
                                           BumpAllocator& chunkMemoryAllocator) noexcept
{
    for (auto entry : mePooConfig.m_mempoolConfig)
    {
        addMemPool(managementAllocator, chunkMemoryAllocator, entry.m_size, entry.m_chunkCount);
    }

    generateChunkManagementPool(managementAllocator);
}

expected<SharedChunk, MemoryManager::Error> MemoryManager::getChunk(const ChunkSettings& chunkSettings) noexcept
{
    void* chunk{nullptr};
    MemPool* memPoolPointer{nullptr};
    const auto requiredChunkSize = chunkSettings.requiredChunkSize();

    uint64_t aquiredChunkSize = 0U;

    for (auto& memPool : m_memPoolVector)
    {
        uint64_t chunkSizeOfMemPool = memPool.getChunkSize();
        if (chunkSizeOfMemPool >= requiredChunkSize)
        {
            chunk = memPool.getChunk();
            memPoolPointer = &memPool;
            aquiredChunkSize = chunkSizeOfMemPool;
            break;
        }
    }

    if (m_memPoolVector.size() == 0)
    {
        IOX_LOG(ERROR, "There are no mempools available!");

        IOX_REPORT(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_WITHOUT_MEMPOOL, iox::er::RUNTIME_ERROR);
        return err(Error::NO_MEMPOOLS_AVAILABLE);
    }
    else if (memPoolPointer == nullptr)
    {
        IOX_LOG(
            ERROR,
            "The following mempools are available:" << [this](auto& log) -> auto& {
                this->printMemPoolVector(log);
                return log;
            } << "Could not find a fitting mempool for a chunk of size "
              << requiredChunkSize);

        IOX_REPORT(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_CHUNK_IS_TOO_LARGE, iox::er::RUNTIME_ERROR);
        return err(Error::NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE);
    }
    else if (chunk == nullptr)
    {
        IOX_LOG(
            ERROR,
            "MemoryManager: unable to acquire a chunk with a chunk-payload size of "
                << chunkSettings.userPayloadSize()
                << "The following mempools are available:" << [this](auto& log) -> auto& {
                this->printMemPoolVector(log);
                return log;
            });

        IOX_REPORT(iox::PoshError::MEPOO__MEMPOOL_GETCHUNK_POOL_IS_RUNNING_OUT_OF_CHUNKS, iox::er::RUNTIME_ERROR);
        return err(Error::MEMPOOL_OUT_OF_CHUNKS);
    }
    else
    {
        auto chunkHeader = new (chunk) ChunkHeader(aquiredChunkSize, chunkSettings);
        auto chunkManagement = new (m_chunkManagementPool.front().getChunk())
            ChunkManagement(chunkHeader, memPoolPointer, &m_chunkManagementPool.front());
        return ok(SharedChunk(chunkManagement));
    }
}

std::ostream& operator<<(std::ostream& stream, const MemoryManager::Error value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

log::LogStream& operator<<(log::LogStream& stream, const MemoryManager::Error value) noexcept
{
    stream << asStringLiteral(value);
    return stream;
}

} // namespace mepoo
} // namespace iox
