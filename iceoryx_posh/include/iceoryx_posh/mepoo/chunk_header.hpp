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
#ifndef IOX_POSH_MEPOO_CHUNK_HEADER_HPP
#define IOX_POSH_MEPOO_CHUNK_HEADER_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/typed_unique_id.hpp"
#include "iceoryx_posh/mepoo/chunk_info.hpp"

#include <atomic>
#include <chrono>
#include <cstdint>

namespace iox
{
namespace mepoo
{
/// @brief IMPORTANT the alignment MUST be 32 or less since all mempools are
///         32 byte aligned otherwise we get alignment problems!
struct alignas(32) ChunkHeader
{
    /// @brief ALlocates memory to store the information about the chunks.
    ChunkHeader() noexcept;

    UniquePortId m_originId{popo::CreateInvalidId};
    ChunkInfo m_info;

    void* payload() const
    {
        // payload is always located relative to "this" in this way
        return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(this) + sizeof(ChunkHeader));
    }

    /// @todo this is a temporary dummy variable to keep the size of the ChunkHeader at 64 byte for compatibility
    /// reasons
    void* m_payloadDummy{nullptr};
};

ChunkHeader* convertPayloadPointerToChunkHeader(const void* const payload) noexcept;

} // namespace mepoo
} // namespace iox

#endif // IOX_POSH_MEPOO_CHUNK_HEADER_HPP
