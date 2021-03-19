// Copyright (c) 2019 - 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace mepoo
{
ChunkHeader::ChunkHeader(const uint32_t chunkSize,
                         const uint32_t payloadSize,
                         const uint32_t payloadAlignment,
                         const uint32_t customHeaderSize,
                         const uint32_t customHeaderAlignment) noexcept
{
    static_assert(alignof(ChunkHeader) >= 8U,
                  "All the calculations expect the ChunkHeader alignment to be at least 8!");
    static_assert(std::is_same<PayloadOffset_t, std::remove_const<decltype(payloadAlignment)>::type>::value,
                  "PayloadOffset_t and payloadAlignment must have same type in order to prevent an overflow for the "
                  "payload offset calculation for extremely large alignments");
    cxx::Expects(customHeaderAlignment <= alignof(ChunkHeader)
                 && "The alignment of the custom header must not exceed the alignment of the ChunkHeader!");

    this->chunkSize = chunkSize;
    this->payloadSize = payloadSize;

    // have a look at »Payload Offset Calculation« in chunk_header.md for more details regarding the calculation
    if (customHeaderSize == 0U)
    {
        if (payloadAlignment <= alignof(mepoo::ChunkHeader))
        {
            // the most simple case with no custom header and the payload adjacent to the ChunkHeader
            payloadOffset = sizeof(ChunkHeader);
        }
        else
        {
            // the second most simple case with no custom header but the payload alignment
            // exceeds the ChunkHeader alignment and is therefore not necessarily adjacent
            uint64_t addressOfChunkHeader = reinterpret_cast<uint64_t>(this);
            uint64_t headerEndAddress = addressOfChunkHeader + sizeof(ChunkHeader);
            uint64_t alignedPayloadAddress = iox::cxx::align(headerEndAddress, static_cast<uint64_t>(payloadAlignment));
            uint64_t offsetToPayload = alignedPayloadAddress - addressOfChunkHeader;
            // the cast is safe since payloadOffset and payloadAlignment have the same type and since the alignment must
            // be a power of 2, the max alignment is about half of the max value the type can hold
            payloadOffset = static_cast<PayloadOffset_t>(offsetToPayload);

            // this is safe since the alignment of the payload is larger than the one from the ChunkHeader
            // -> the payload is either adjacent and `backOffset` is at the same location as `payloadOffset`
            //    or the payload is not adjacent and there is space of at least the alignment of ChunkHeader
            //    between the ChunkHeader and the payload
            auto addressOfBackOffset = alignedPayloadAddress - sizeof(PayloadOffset_t);
            auto backOffset = reinterpret_cast<PayloadOffset_t*>(addressOfBackOffset);
            *backOffset = payloadOffset;
        }
    }
    else
    {
        // the most complex case with a custom header
        auto addressOfChunkHeader = reinterpret_cast<uint64_t>(this);
        uint64_t headerEndAddress = addressOfChunkHeader + sizeof(ChunkHeader) + customHeaderSize;
        uint64_t potentialBackOffsetAddress =
            iox::cxx::align(headerEndAddress, static_cast<uint64_t>(alignof(PayloadOffset_t)));
        uint64_t potentialPayloadAddress = potentialBackOffsetAddress + sizeof(PayloadOffset_t);
        uint64_t alignedPayloadAddress =
            iox::cxx::align(potentialPayloadAddress, static_cast<uint64_t>(payloadAlignment));
        uint64_t offsetToPayload = alignedPayloadAddress - addressOfChunkHeader;
        // the cast is safe since payloadOffset and payloadAlignment have the same type and since the alignment must
        // be a power of 2, the max alignment is about half of the max value the type can hold
        payloadOffset = static_cast<PayloadOffset_t>(offsetToPayload);

        // this always works if the alignment of PayloadOffset_t and payloadAlignment are equal,
        // if not there are two options:
        //   - either the alignment of the PayloadOffset_t is larger than payloadAlignment
        //     -> the payload is adjacent to the back-offset and therefore this also works
        //   - or the alignment of the PayloadOffset_t is smaller than payloadAlignment
        //     -> the back-offset can be put adjacent to to the Topic since the smaller alignment always fits
        auto addressOfBackOffset = alignedPayloadAddress - sizeof(PayloadOffset_t);
        auto backOffset = reinterpret_cast<PayloadOffset_t*>(addressOfBackOffset);
        *backOffset = payloadOffset;
    }

    cxx::Ensures(overflowSafeUsedSizeOfChunk() <= chunkSize
                 && "Used size of chunk would exceed the actual chunk size!");
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
    // or if the payload has a custom alignment
    auto payloadOffset = reinterpret_cast<PayloadOffset_t*>(payloadAddress - sizeof(PayloadOffset_t));
    return reinterpret_cast<ChunkHeader*>(payloadAddress - *payloadOffset);
}

uint32_t ChunkHeader::usedSizeOfChunk() const noexcept
{
    return static_cast<uint32_t>(overflowSafeUsedSizeOfChunk());
}

uint64_t ChunkHeader::overflowSafeUsedSizeOfChunk() const noexcept
{
    return static_cast<uint64_t>(payloadOffset) + static_cast<uint64_t>(payloadSize);
}

} // namespace mepoo
} // namespace iox
