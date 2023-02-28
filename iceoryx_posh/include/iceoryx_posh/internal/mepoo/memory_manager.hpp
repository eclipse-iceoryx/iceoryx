// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_MEMORY_MANAGER_HPP
#define IOX_POSH_MEPOO_MEMORY_MANAGER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/mepoo/mem_pool.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/mepoo/chunk_settings.hpp"
#include "iox/algorithm.hpp"
#include "iox/bump_allocator.hpp"
#include "iox/expected.hpp"
#include "iox/memory.hpp"
#include "iox/vector.hpp"

#include <cstdint>
#include <limits>

// this header must always be the last one, otherwise windows macros
// are kicking in and nothing compiles
#include "iceoryx_platform/platform_correction.hpp"

namespace iox
{
namespace log
{
class LogStream;
}
namespace mepoo
{
struct MePooConfig;

class MemoryManager
{
    using MaxChunkPayloadSize_t = range<uint32_t, 1, std::numeric_limits<uint32_t>::max() - sizeof(ChunkHeader)>;

  public:
    enum class Error
    {
        NO_MEMPOOLS_AVAILABLE,
        NO_MEMPOOL_FOR_REQUESTED_CHUNK_SIZE,
        MEMPOOL_OUT_OF_CHUNKS,
    };

    MemoryManager() noexcept = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    MemoryManager& operator=(MemoryManager&&) = delete;
    ~MemoryManager() noexcept = default;

    void configureMemoryManager(const MePooConfig& mePooConfig,
                                BumpAllocator& managementAllocator,
                                BumpAllocator& chunkMemoryAllocator) noexcept;

    /// @brief Obtains a chunk from the mempools
    /// @param[in] chunkSettings for the requested chunk
    /// @return a SharedChunk if successful, otherwise a MemoryManager::Error
    expected<SharedChunk, Error> getChunk(const ChunkSettings& chunkSettings) noexcept;

    uint32_t getNumberOfMemPools() const noexcept;

    MemPoolInfo getMemPoolInfo(const uint32_t index) const noexcept;

    static uint64_t requiredChunkMemorySize(const MePooConfig& mePooConfig) noexcept;
    static uint64_t requiredManagementMemorySize(const MePooConfig& mePooConfig) noexcept;
    static uint64_t requiredFullMemorySize(const MePooConfig& mePooConfig) noexcept;

  private:
    static uint32_t sizeWithChunkHeaderStruct(const MaxChunkPayloadSize_t size) noexcept;

    void printMemPoolVector(log::LogStream& log) const noexcept;
    void addMemPool(BumpAllocator& managementAllocator,
                    BumpAllocator& chunkMemoryAllocator,
                    const greater_or_equal<uint32_t, MemPool::CHUNK_MEMORY_ALIGNMENT> chunkPayloadSize,
                    const greater_or_equal<uint32_t, 1> numberOfChunks) noexcept;
    void generateChunkManagementPool(BumpAllocator& managementAllocator) noexcept;

  private:
    bool m_denyAddMemPool{false};
    uint32_t m_totalNumberOfChunks{0};

    vector<MemPool, MAX_NUMBER_OF_MEMPOOLS> m_memPoolVector;
    vector<MemPool, 1> m_chunkManagementPool;
};

/// @brief Converts the MemoryManager::Error to a string literal
/// @param[in] value to convert to a string literal
/// @return pointer to a string literal
inline constexpr const char* asStringLiteral(const MemoryManager::Error value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with std::ostream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
std::ostream& operator<<(std::ostream& stream, const MemoryManager::Error value) noexcept;

/// @brief Convenience stream operator to easily use the 'asStringLiteral' function with iox::log::LogStream
/// @param[in] stream sink to write the message to
/// @param[in] value to convert to a string literal
/// @return the reference to 'stream' which was provided as input parameter
log::LogStream& operator<<(log::LogStream& stream, const MemoryManager::Error value) noexcept;

} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/memory_manager.inl"

#endif // IOX_POSH_MEPOO_MEMORY_MANAGER_HPP
