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

#include "iceoryx_posh/popo/publisher.hpp"

#include "iceoryx_posh/runtime/posh_runtime.hpp"

namespace iox
{
namespace popo
{
Publisher::Publisher() noexcept
{
}

Publisher::Publisher(const capro::ServiceDescription& service, const cxx::CString100& runnableName) noexcept
    : m_sender(runtime::PoshRuntime::getInstance().getMiddlewareSender(service, Interfaces::INTERNAL, runnableName))
{
}

const void* Publisher::getLastChunk() const noexcept
{
    assert(false && "Not yet supported");
    return nullptr;
}

mepoo::ChunkHeader* Publisher::allocateChunkWithHeader(uint32_t payloadSize, bool useDynamicPayloadSizes) noexcept
{
    return m_sender.reserveChunk(payloadSize, useDynamicPayloadSizes);
}

void* Publisher::allocateChunk(uint32_t payloadSize, bool useDynamicPayloadSizes) noexcept
{
    auto chunkHeader = m_sender.reserveChunk(payloadSize, useDynamicPayloadSizes);
    return chunkHeader->payload();
}

void Publisher::sendChunk(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    m_sender.deliverChunk(chunkHeader);
}

void Publisher::sendChunk(const void* const payload) noexcept
{
    auto chunkHeader = iox::mepoo::convertPayloadPointerToChunkHeader(const_cast<void* const>(payload));
    m_sender.deliverChunk(chunkHeader);
}

void Publisher::freeChunk(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    m_sender.freeChunk(chunkHeader);
}

void Publisher::freeChunk(void* const payload) noexcept
{
    auto chunkHeader = iox::mepoo::convertPayloadPointerToChunkHeader(payload);
    m_sender.freeChunk(chunkHeader);
}

void Publisher::offer() noexcept
{
    m_sender.activate();
}

void Publisher::stopOffer() noexcept
{
    m_sender.deactivate();
}

bool Publisher::hasSubscribers() noexcept
{
    return m_sender.hasSubscribers();
}

void Publisher::enableDoDeliverOnSubscription() noexcept
{
    m_sender.enableDoDeliverOnSubscription();
}

} // namespace popo
} // namespace iox
