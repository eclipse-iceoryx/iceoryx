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

#pragma once

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_utils/cxx/variant_queue.hpp"
#include "iceoryx_posh/internal/mepoo/shared_pointer.hpp"
#include "iceoryx_utils/posix_wrapper/semaphore.hpp"

namespace iox
{
namespace popo
{

struct ChunkQueueData
{
    struct ChunkTuple
    {
        ChunkTuple() = default;
        ChunkTuple(iox::relative_ptr<mepoo::ChunkManagement> f_chunk) noexcept
            : m_segmentId(f_chunk.getId())
            , m_chunkOffset(f_chunk.getOffset())
        {
        }

        RelativePointer::id_t m_segmentId{iox::RelativePointer::NULL_POINTER_ID};
        RelativePointer::offset_t m_chunkOffset{iox::RelativePointer::NULL_POINTER_OFFSET};
    };

    ChunkQueueData(cxx::VariantQueueTypes queueType) noexcept
    : m_queue(queueType)
    {
    
    }

    cxx::VariantQueue<ChunkTuple, MAX_RECEIVER_QUEUE_CAPACITY> m_queue;
    mepoo::SharedPointer<posix::Semaphore> m_semaphore;
    std::atomic_bool m_semaphoreAttached{false};

};

} // namespace popo
} // namespace iox
