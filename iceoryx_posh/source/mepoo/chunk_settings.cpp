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

#include "iceoryx_posh/mepoo/chunk_settings.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"

namespace iox
{
namespace mepoo
{
cxx::expected<ChunkSettings, ChunkSettings::Error> ChunkSettings::create(const uint32_t payloadSize,
                                                                         const uint32_t payloadAlignment,
                                                                         const uint32_t customHeaderSize,
                                                                         const uint32_t customHeaderAlignment) noexcept
{
    // since alignas accepts 0, we also do but we adjust it to 1 in case there are some division or modulo operations
    // with the alignment later on
    uint32_t adjustedPayloadAlignment = payloadAlignment == 0U ? 1U : payloadAlignment;
    uint32_t adjustedCustomHeaderAlignment = customHeaderAlignment == 0U ? 1U : customHeaderAlignment;

    if (!cxx::isPowerOfTwo(payloadAlignment) || !cxx::isPowerOfTwo(customHeaderAlignment))
    {
        return cxx::error<ChunkSettings::Error>(ChunkSettings::Error::ALIGNMENT_NOT_POWER_OF_TWO);
    }

    if (customHeaderSize % adjustedCustomHeaderAlignment != 0U)
    {
        return cxx::error<ChunkSettings::Error>(ChunkSettings::Error::CUSTOM_HEADER_SIZE_NOT_MULTIPLE_OF_ITS_ALIGNMENT);
    }

    if (adjustedCustomHeaderAlignment > alignof(ChunkHeader))
    {
        // for ease of calculation, the alignment of the custom header is restricted to not exceed the alignment of the
        // ChunkHeader
        return cxx::error<ChunkSettings::Error>(
            ChunkSettings::Error::CUSTOM_HEADER_ALIGNMENT_EXCEEDS_CHUNK_HEADER_ALIGNMENT);
    }

    uint64_t requiredChunkSize = calculateRequiredChunkSize(payloadSize, adjustedPayloadAlignment, customHeaderSize);

    if (requiredChunkSize > std::numeric_limits<uint32_t>::max())
    {
        return cxx::error<ChunkSettings::Error>(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE);
    }

    return cxx::success<ChunkSettings>(ChunkSettings{payloadSize,
                                                     adjustedPayloadAlignment,
                                                     customHeaderSize,
                                                     adjustedCustomHeaderAlignment,
                                                     static_cast<uint32_t>(requiredChunkSize)});
}
uint64_t ChunkSettings::calculateRequiredChunkSize(const uint32_t payloadSize,
                                                   const uint32_t payloadAlignment,
                                                   const uint32_t customHeaderSize) noexcept
{
    // have a look at »Required Chunk Size Calculation« in chunk_header.md for more details regarding the calculation
    if (customHeaderSize == 0)
    {
        // the most simple case with no custom header and the payload adjacent to the ChunkHeader
        if (payloadAlignment <= alignof(mepoo::ChunkHeader))
        {
            uint64_t requiredChunkSize = static_cast<uint64_t>(sizeof(ChunkHeader)) + payloadSize;

            return requiredChunkSize;
        }

        // the second most simple case with no custom header but the payload alignment
        // exceeds the ChunkHeader alignment and is therefore not necessarily adjacent
        uint64_t prePayloadAlignmentOverhang = static_cast<uint64_t>(sizeof(ChunkHeader) - alignof(ChunkHeader));
        uint64_t requiredChunkSize = prePayloadAlignmentOverhang + payloadAlignment + payloadSize;

        return requiredChunkSize;
    }

    // the most complex case with a custom header
    constexpr uint64_t SIZE_OF_PAYLOAD_OFFSET_T{sizeof(ChunkHeader::PayloadOffset_t)};
    constexpr uint64_t ALIGNMENT_OF_PAYLOAD_OFFSET_T{alignof(ChunkHeader::PayloadOffset_t)};
    uint64_t headerSize = static_cast<uint64_t>(sizeof(ChunkHeader) + customHeaderSize);
    uint64_t prePayloadAlignmentOverhang = cxx::align(headerSize, ALIGNMENT_OF_PAYLOAD_OFFSET_T);
    uint64_t maxPadding = algorithm::max(SIZE_OF_PAYLOAD_OFFSET_T, static_cast<uint64_t>(payloadAlignment));
    uint64_t requiredChunkSize = prePayloadAlignmentOverhang + maxPadding + payloadSize;

    return requiredChunkSize;
}

uint32_t ChunkSettings::requiredChunkSize() const noexcept
{
    return m_requiredChunkSize;
}

uint32_t ChunkSettings::payloadSize() const noexcept
{
    return m_payloadSize;
}

uint32_t ChunkSettings::payloadAlignment() const noexcept
{
    return m_payloadAlignment;
}

uint32_t ChunkSettings::customHeaderSize() const noexcept
{
    return m_customHeaderSize;
}

uint32_t ChunkSettings::customHeaderAlignment() const noexcept
{
    return m_customHeaderAlignment;
}

} // namespace mepoo
} // namespace iox
