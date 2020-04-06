// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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

#include "iceoryx_posh/internal/popo/building_blocks/chunk_receiver.hpp"

#include "iceoryx_posh/internal/log/posh_logging.hpp"

namespace iox
{
namespace popo
{
ChunkReceiver::ChunkReceiver(MemberType_t* const chunkReceiverDataPtr) noexcept
    : ChunkQueue(chunkReceiverDataPtr)
{
}

const ChunkReceiver::MemberType_t* ChunkReceiver::getMembers() const noexcept
{
    return reinterpret_cast<const MemberType_t*>(ChunkQueue::getMembers());
}

ChunkReceiver::MemberType_t* ChunkReceiver::getMembers() noexcept
{
    return reinterpret_cast<MemberType_t*>(ChunkQueue::getMembers());
}

cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiverError> get() noexcept
{
    return cxx::error<ChunkReceiverError>(ChunkReceiverError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL);
}

void release(const mepoo::ChunkHeader* chunkHeader) noexcept
{
}

void releaseAll() noexcept
{
}

} // namespace popo
} // namespace iox
