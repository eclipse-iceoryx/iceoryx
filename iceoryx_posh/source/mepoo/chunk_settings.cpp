// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_posh/mepoo/chunk_settings.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iox/algorithm.hpp"
#include "iox/memory.hpp"

namespace iox
{
namespace mepoo
{
ChunkSettings::ChunkSettings(const uint64_t userPayloadSize,
                             const uint32_t userPayloadAlignment,
                             const uint32_t userHeaderSize,
                             const uint32_t userHeaderAlignment,
                             const uint64_t requiredChunkSize) noexcept
    : m_userPayloadSize(userPayloadSize)
    , m_userPayloadAlignment(userPayloadAlignment)
    , m_userHeaderSize(userHeaderSize)
    , m_userHeaderAlignment(userHeaderAlignment)
    , m_requiredChunkSize(requiredChunkSize)
{
}

expected<ChunkSettings, ChunkSettings::Error> ChunkSettings::create(const uint64_t userPayloadSize,
                                                                    const uint32_t userPayloadAlignment,
                                                                    const uint32_t userHeaderSize,
                                                                    const uint32_t userHeaderAlignment) noexcept
{
    // since alignas accepts 0, we also do but we adjust it to 1 in case there are some division or modulo operations
    // with the alignment later on
    uint32_t adjustedUserPayloadAlignment = userPayloadAlignment == 0U ? 1U : userPayloadAlignment;
    uint32_t adjustedUserHeaderAlignment = userHeaderAlignment == 0U ? 1U : userHeaderAlignment;

    if (!isPowerOfTwo(adjustedUserPayloadAlignment) || !isPowerOfTwo(adjustedUserHeaderAlignment))
    {
        return err(ChunkSettings::Error::ALIGNMENT_NOT_POWER_OF_TWO);
    }

    if (adjustedUserHeaderAlignment > alignof(ChunkHeader))
    {
        // for ease of calculation, the alignment of the user-header is restricted to not exceed the alignment of the
        // ChunkHeader
        return err(ChunkSettings::Error::USER_HEADER_ALIGNMENT_EXCEEDS_CHUNK_HEADER_ALIGNMENT);
    }

    if (userHeaderSize % adjustedUserHeaderAlignment != 0U)
    {
        return err(ChunkSettings::Error::USER_HEADER_SIZE_NOT_MULTIPLE_OF_ITS_ALIGNMENT);
    }

    auto expectChunkSize = calculateRequiredChunkSize(userPayloadSize, adjustedUserPayloadAlignment, userHeaderSize);
    if (expectChunkSize.has_error())
    {
        return err(expectChunkSize.error());
    }
    uint64_t requiredChunkSize = expectChunkSize.value();


    return ok(ChunkSettings{
        userPayloadSize, adjustedUserPayloadAlignment, userHeaderSize, adjustedUserHeaderAlignment, requiredChunkSize});
}

expected<uint64_t, ChunkSettings::Error> ChunkSettings::calculateRequiredChunkSize(
    const uint64_t userPayloadSize, const uint32_t userPayloadAlignment, const uint32_t userHeaderSize) noexcept
{
    // have a look at »Required Chunk Size Calculation« in chunk_header.md for more details regarding the calculation
    if (userHeaderSize == 0)
    {
        // the most simple case with no user-header and the user-payload adjacent to the ChunkHeader
        if (userPayloadAlignment <= alignof(mepoo::ChunkHeader))
        {
            if (userPayloadSize > std::numeric_limits<uint64_t>::max() - sizeof(ChunkHeader))
            {
                return err(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE);
            }

            uint64_t requiredChunkSize = sizeof(ChunkHeader) + userPayloadSize;

            return ok(requiredChunkSize);
        }

        // the second most simple case with no user-header but the user-payload alignment
        // exceeds the ChunkHeader alignment and is therefore not necessarily adjacent
        uint64_t preUserPayloadAlignmentOverhang = sizeof(ChunkHeader) - alignof(ChunkHeader);

        if (userPayloadSize
            > std::numeric_limits<uint64_t>::max() - preUserPayloadAlignmentOverhang - userPayloadAlignment)
        {
            return err(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE);
        }

        uint64_t requiredChunkSize = preUserPayloadAlignmentOverhang + userPayloadAlignment + userPayloadSize;

        return ok(requiredChunkSize);
    }

    // the most complex case with a user-header
    constexpr uint64_t SIZE_OF_USER_PAYLOAD_OFFSET_T{sizeof(ChunkHeader::UserPayloadOffset_t)};
    constexpr uint64_t ALIGNMENT_OF_USER_PAYLOAD_OFFSET_T{alignof(ChunkHeader::UserPayloadOffset_t)};
    uint64_t headerSize = sizeof(ChunkHeader) + userHeaderSize;
    uint64_t preUserPayloadAlignmentOverhang = align(headerSize, ALIGNMENT_OF_USER_PAYLOAD_OFFSET_T);
    uint64_t maxPadding = algorithm::maxVal(SIZE_OF_USER_PAYLOAD_OFFSET_T, static_cast<uint64_t>(userPayloadAlignment));

    if (userPayloadSize > std::numeric_limits<uint64_t>::max() - preUserPayloadAlignmentOverhang - maxPadding)
    {
        return err(ChunkSettings::Error::REQUIRED_CHUNK_SIZE_EXCEEDS_MAX_CHUNK_SIZE);
    }

    uint64_t requiredChunkSize = preUserPayloadAlignmentOverhang + maxPadding + userPayloadSize;

    return ok(requiredChunkSize);
}

uint64_t ChunkSettings::requiredChunkSize() const noexcept
{
    return m_requiredChunkSize;
}

uint64_t ChunkSettings::userPayloadSize() const noexcept
{
    return m_userPayloadSize;
}

uint32_t ChunkSettings::userPayloadAlignment() const noexcept
{
    return m_userPayloadAlignment;
}

uint32_t ChunkSettings::userHeaderSize() const noexcept
{
    return m_userHeaderSize;
}

uint32_t ChunkSettings::userHeaderAlignment() const noexcept
{
    return m_userHeaderAlignment;
}

} // namespace mepoo
} // namespace iox
