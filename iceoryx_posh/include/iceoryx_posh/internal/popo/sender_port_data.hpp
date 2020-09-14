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
#ifndef IOX_POSH_POPO_SENDER_PORT_DATA_HPP
#define IOX_POSH_POPO_SENDER_PORT_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/capro/capro_message.hpp"
#include "iceoryx_posh/internal/mepoo/memory_manager.hpp"
#include "iceoryx_posh/internal/mepoo/shared_chunk.hpp"
#include "iceoryx_posh/internal/popo/ports/base_port_data.hpp"
#include "iceoryx_posh/internal/popo/receiver_handler.hpp"
#include "iceoryx_posh/internal/popo/used_chunk_list.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"
#include "iceoryx_posh/runtime/port_config_info.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/optional.hpp"
#include "iceoryx_utils/internal/concurrent/taco.hpp"
#include "iceoryx_utils/internal/relocatable_pointer/relative_ptr.hpp"

#include <atomic>
#include <cstdint>

namespace iox
{
namespace popo
{
struct SenderPortData : public BasePortData
{
    using MemoryInfo = iox::mepoo::MemoryInfo;

    struct Throughput
    {
        mepoo::SequenceNumberType sequenceNumber{0u};
        uint32_t payloadSize{0u};
        uint32_t chunkSize{0u};
        mepoo::TimePointNs lastDeliveryTimestamp{mepoo::DurationNs(0)};
        mepoo::TimePointNs currentDeliveryTimestamp{mepoo::DurationNs(0)};
    };

    SenderPortData(mepoo::MemoryManager* const memoryMgr = nullptr, mepoo::SharedChunk lastChunk = nullptr) noexcept;
    SenderPortData(const capro::ServiceDescription& serviceDescription,
                   mepoo::MemoryManager* const memMgr,
                   const std::string& applicationName,
                   const MemoryInfo& memoryInfo = MemoryInfo()) noexcept;

    using ReceiverHandler_t = ReceiverHandler<MAX_RECEIVERS_PER_SENDERPORT, ThreadSafe>;
    ReceiverHandler_t m_receiverHandler;

    // Written by application, read by RouDi
    std::atomic_bool m_activateRequested{false};
    std::atomic_bool m_active{false};
    bool m_isUnique{false};


    UsedChunkList<MAX_CHUNKS_ALLOCATED_PER_PUBLISHER_SIMULTANEOUSLY> m_allocatedChunksList;

    mepoo::SequenceNumberType m_sequenceNumber{0u};
    // throughput related members
    std::atomic<uint32_t> m_activePayloadSize{0u};
    Throughput m_throughput{};
    mutable Throughput m_throughputReadCache{};
    enum class ThreadContext : uint32_t
    {
        Application,
        RouDi,
        END_OF_LIST
    };
    mutable concurrent::TACO<Throughput, ThreadContext> m_throughputExchange{
        concurrent::TACOMode::DenyDataFromSameContext};

    iox::relative_ptr<mepoo::MemoryManager> m_memoryMgr;
    mepoo::SharedChunk m_lastChunk{nullptr};

    MemoryInfo m_memoryInfo;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_SENDER_PORT_DATA_HPP
