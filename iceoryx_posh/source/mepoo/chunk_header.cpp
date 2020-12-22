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

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace mepoo
{
ChunkHeader::ChunkHeader() noexcept
{
}

void* ChunkHeader::payload() const noexcept
{
    // payload is always located relative to "this" in this way
    return reinterpret_cast<void*>(reinterpret_cast<uint64_t>(this) + payloadOffset);
}

ChunkHeader* ChunkHeader::fromPayload(const void* const payload) noexcept
{
    if (payload == nullptr)
    {
        return nullptr;
    }
    uint64_t payloadAddress = reinterpret_cast<uint64_t>(payload);
    // the payload offset is always stored in front of the payload, no matter if a custom header is used or not
    // or if the payload has a custom allignment
    using PayloadOffsetType = decltype(ChunkHeader::payloadOffset);
    auto payloadOffset = reinterpret_cast<PayloadOffsetType*>(payloadAddress - sizeof(PayloadOffsetType));
    return reinterpret_cast<ChunkHeader*>(payloadAddress - *payloadOffset);
}

uint32_t ChunkHeader::usedSizeOfChunk()
{
    auto usedSizeOfChunk = static_cast<uint64_t>(payloadOffset) + static_cast<uint64_t>(payloadSize);

    cxx::Expects(usedSizeOfChunk <= chunkSize);

    return static_cast<uint32_t>(usedSizeOfChunk);
}

} // namespace mepoo
} // namespace iox
