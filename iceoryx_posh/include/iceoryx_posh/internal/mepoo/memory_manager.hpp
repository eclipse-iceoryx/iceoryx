// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <cstdint>
#include <limits>

// this header must always be the last one, otherwise windows macros
// are kicking in and nothing compiles
#include "iceoryx_utils/platform/platform_correction.hpp"

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
    using MaxChunkPayloadSize_t = cxx::range<uint32_t, 1, std::numeric_limits<uint32_t>::max() - sizeof(ChunkHeader)>;

  public:
    MemoryManager() noexcept = default;
    MemoryManager(const MemoryManager&) = delete;
    MemoryManager(MemoryManager&&) = delete;
    MemoryManager& operator=(const MemoryManager&) = delete;
    MemoryManager& operator=(MemoryManager&&) = delete;
    ~MemoryManager() noexcept = default;

    void configureMemoryManager(const MePooConfig& f_mePooConfig,
                                posix::Allocator* f_managementAllocator,
                                posix::Allocator* f_payloadAllocator) noexcept;

    SharedChunk getChunk(const ChunkSettings& chunkSettings) noexcept;

    uint32_t getNumberOfMemPools() const noexcept;

    MemPoolInfo getMemPoolInfo(uint32_t f_index) const noexcept;

    /// @brief calculates the required chunk size regarding the constraints on payload and custom header
    /// @param[in] payloadSize is the size of the user payload without additional headers
    /// @param[in] payloadAlignment is the alignment of the user payload
    /// @param[in] customHeaderSize is the size of the custom user header; use iox::CHUNK_NO_CUSTOM_HEADER_SIZE to omit
    /// a custom header
    /// @param[in] customHeaderAlignment is the alignment of the custom user header; use
    /// iox::CHUNK_NO_CUSTOM_HEADER_ALIGNMENT to omit a custom header
    /// @return the minimum chunk size required to hold the user payload with the custom header
    static uint32_t requiredChunkSize(const uint32_t payloadSize,
                                      const uint32_t payloadAlignment,
                                      const uint32_t customHeaderSize,
                                      const uint32_t customHeaderAlignment) noexcept;

    static uint64_t requiredChunkMemorySize(const MePooConfig& f_mePooConfig) noexcept;
    static uint64_t requiredManagementMemorySize(const MePooConfig& f_mePooConfig) noexcept;
    static uint64_t requiredFullMemorySize(const MePooConfig& f_mePooConfig) noexcept;

  private:
    static uint32_t sizeWithChunkHeaderStruct(const MaxChunkPayloadSize_t size) noexcept;

    void printMemPoolVector(log::LogStream& log) const noexcept;
    void addMemPool(posix::Allocator* f_managementAllocator,
                    posix::Allocator* f_payloadAllocator,
                    const cxx::greater_or_equal<uint32_t, MemPool::MEMORY_ALIGNMENT> f_payloadSize,
                    const cxx::greater_or_equal<uint32_t, 1> f_numberOfChunks) noexcept;
    void generateChunkManagementPool(posix::Allocator* f_managementAllocator) noexcept;

  private:
    bool m_denyAddMemPool{false};
    uint32_t m_totalNumberOfChunks{0};

    cxx::vector<MemPool, MAX_NUMBER_OF_MEMPOOLS> m_memPoolVector;
    cxx::vector<MemPool, 1> m_chunkManagementPool;
};

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_MEMORY_MANAGER_HPP
