// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/popo/rpc_header.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

namespace iox
{
namespace popo
{
RpcBaseHeader::RpcBaseHeader(const UniqueId& uniqueClientQueueId,
                             const uint32_t lastKnownClientQueueIndex,
                             const int64_t sequenceId,
                             const uint8_t rpcHeaderVersion) noexcept
    : m_rpcHeaderVersion(rpcHeaderVersion)
    , m_lastKnownClientQueueIndex(lastKnownClientQueueIndex)
    , m_uniqueClientQueueId(uniqueClientQueueId)
    , m_sequenceId(sequenceId)
{
}

uint8_t RpcBaseHeader::getRpcHeaderVersion() const noexcept
{
    return m_rpcHeaderVersion;
}

int64_t RpcBaseHeader::getSequenceId() const noexcept
{
    return m_sequenceId;
}

mepoo::ChunkHeader* RpcBaseHeader::getChunkHeader() noexcept
{
    return mepoo::ChunkHeader::fromUserHeader(this);
}

const mepoo::ChunkHeader* RpcBaseHeader::getChunkHeader() const noexcept
{
    return mepoo::ChunkHeader::fromUserHeader(this);
}

void* RpcBaseHeader::getUserPayload() noexcept
{
    return mepoo::ChunkHeader::fromUserHeader(this)->userPayload();
}

const void* RpcBaseHeader::getUserPayload() const noexcept
{
    return mepoo::ChunkHeader::fromUserHeader(this)->userPayload();
}

RequestHeader::RequestHeader(const UniqueId& uniqueClientQueueId, const uint32_t lastKnownClientQueueIndex) noexcept
    : RpcBaseHeader(uniqueClientQueueId, lastKnownClientQueueIndex, START_SEQUENCE_ID, RPC_HEADER_VERSION)
{
}

void RequestHeader::setSequenceId(const int64_t sequenceId) noexcept
{
    this->m_sequenceId = sequenceId;
}

RequestHeader* RequestHeader::fromPayload(void* const payload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(payload);
    return chunkHeader ? static_cast<RequestHeader*>(chunkHeader->userHeader()) : nullptr;
}

const RequestHeader* RequestHeader::fromPayload(const void* const payload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(payload);
    return chunkHeader ? static_cast<const RequestHeader*>(chunkHeader->userHeader()) : nullptr;
}

ResponseHeader::ResponseHeader(const UniqueId& uniqueClientQueueId,
                               const uint32_t lastKnownClientQueueIndex,
                               const int64_t sequenceId) noexcept
    : RpcBaseHeader(uniqueClientQueueId, lastKnownClientQueueIndex, sequenceId, RPC_HEADER_VERSION)
{
}

void ResponseHeader::setServerError() noexcept
{
    m_hasServerError = true;
}

bool ResponseHeader::hasServerError() const noexcept
{
    return m_hasServerError;
}

ResponseHeader* ResponseHeader::fromPayload(void* const payload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(payload);
    return chunkHeader ? static_cast<ResponseHeader*>(chunkHeader->userHeader()) : nullptr;
}

const ResponseHeader* ResponseHeader::fromPayload(const void* const payload) noexcept
{
    auto chunkHeader = mepoo::ChunkHeader::fromUserPayload(payload);
    return chunkHeader ? static_cast<const ResponseHeader*>(chunkHeader->userHeader()) : nullptr;
}

} // namespace popo
} // namespace iox
