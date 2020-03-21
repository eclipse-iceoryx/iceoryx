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

namespace iox
{
namespace popo
{
template <typename SenderPortType>
inline Publisher_t<SenderPortType>::Publisher_t() noexcept
{
}

template <typename SenderPortType>
inline Publisher_t<SenderPortType>::Publisher_t(const capro::ServiceDescription& service,
                                                const cxx::CString100& runnableName) noexcept
    : m_sender(runtime::PoshRuntime::getInstance().getMiddlewareSender(service, runnableName))
{
}

template <typename SenderPortType>
inline Publisher_t<SenderPortType>::~Publisher_t() noexcept
{
    if (m_sender)
    {
        m_sender.destroy();
    }
}

template <typename SenderPortType>
inline const void* Publisher_t<SenderPortType>::getLastChunk() const noexcept
{
    assert(false && "Not yet supported");
    return nullptr;
}

template <typename SenderPortType>
inline mepoo::ChunkHeader* Publisher_t<SenderPortType>::allocateChunkWithHeader(uint32_t payloadSize,
                                                                                bool useDynamicPayloadSizes) noexcept
{
    return m_sender.reserveChunk(payloadSize, useDynamicPayloadSizes);
}

template <typename SenderPortType>
inline void* Publisher_t<SenderPortType>::allocateChunk(uint32_t payloadSize, bool useDynamicPayloadSizes) noexcept
{
    auto chunkHeader = m_sender.reserveChunk(payloadSize, useDynamicPayloadSizes);
    return chunkHeader->payload();
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::sendChunk(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    m_sender.deliverChunk(chunkHeader);
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::sendChunk(const void* const payload) noexcept
{
    auto chunkHeader = iox::mepoo::convertPayloadPointerToChunkHeader(const_cast<void* const>(payload));
    m_sender.deliverChunk(chunkHeader);
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::freeChunk(mepoo::ChunkHeader* const chunkHeader) noexcept
{
    m_sender.freeChunk(chunkHeader);
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::freeChunk(void* const payload) noexcept
{
    auto chunkHeader = iox::mepoo::convertPayloadPointerToChunkHeader(payload);
    m_sender.freeChunk(chunkHeader);
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::offer() noexcept
{
    m_sender.activate();
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::stopOffer() noexcept
{
    m_sender.deactivate();
}

template <typename SenderPortType>
inline bool Publisher_t<SenderPortType>::hasSubscribers() noexcept
{
    return m_sender.hasSubscribers();
}

template <typename SenderPortType>
inline void Publisher_t<SenderPortType>::enableDoDeliverOnSubscription() noexcept
{
    m_sender.enableDoDeliverOnSubscription();
}

} // namespace popo
} // namespace iox
