// Copyright (c) 2019, 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_MEPOO_CHUNK_HEADER_HPP
#define IOX_POSH_MEPOO_CHUNK_HEADER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"

#include <cstdint>

namespace iox
{
namespace mepoo
{
/// @brief IMPORTANT the alignment MUST be 32 or less since all mempools are
///         32 byte aligned otherwise we get alignment problems!
struct alignas(32) ChunkHeader
{
    ChunkHeader() noexcept;

    /// @brief From the 1.0 release onward, this must be incremented for each incompatible change, e.g.
    ///            - data width of members changes
    ///            - members are rearranged
    ///            - semantic meaning of a member changes
    static constexpr uint8_t CHUNK_HEADER_VERSION{1U};

    /// @brief The size of the whole chunk, including the header
    uint32_t chunkSize{0U};

    /// @brief Used to detect incompatibilities for record&replay functionality
    uint8_t chunkHeaderVersion{CHUNK_HEADER_VERSION};

    /// @brief Currently not used and set to `0`
    uint8_t reserved1{0U};
    uint8_t reserved2{0U};
    uint8_t reserved3{0U};

    /// @brief The unique identifier of the publisher the chunk was sent from
    UniquePortId originId{popo::InvalidId};

    /// @brief a serial number for the sent chunks
    uint64_t sequenceNumber{0U};

    /// @brief The size of the chunk occupied by the payload
    uint32_t payloadSize{0U};

    /// @brief The offset of the payload relative to the begin of the chunk
    uint32_t payloadOffset{sizeof(ChunkHeader)};

    /// @brief Get a pointer to the payload carried by the chunk
    /// @return the pointer to the payload
    void* payload() const noexcept;

    /// @brief Get the pointer to the custom header
    /// @return the pointer to the custom header
    template <typename T>
    T* customHeader() const noexcept;

    /// @brief Get a pointer to the `ChunkHeader` associated to the payload of the chunk
    /// @param[in] payload is the pointer to the payload of the chunk
    /// @return the pointer to the `ChunkHeader` or a `nullptr` if `payload` is a `nullptr`
    static ChunkHeader* fromPayload(const void* const payload) noexcept;

    /// @brief Calculates the used size of the chunk with the ChunkHeader, custom heander and payload
    /// @return the used size of the chunk
    uint32_t usedSizeOfChunk();
};

} // namespace mepoo
} // namespace iox

#include "iceoryx_posh/internal/mepoo/chunk_header.inl"

#endif // IOX_POSH_MEPOO_CHUNK_HEADER_HPP
