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

#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"

using namespace iox;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;

extern "C" {
#include "iceoryx_binding_c/publisher.h"
}

PublisherPortData*
iox_pub_create(const char* service, const char* instance, const char* event, const uint64_t historyCapacity)
{
    return new PublisherPortData(
        ServiceDescription{
            IdString(TruncateToCapacity, service),
            IdString(TruncateToCapacity, instance),
            IdString(TruncateToCapacity, event),
        },
        "JoinTheChurchOfHypnotoad!",
        nullptr,
        historyCapacity);
}

void iox_pub_destroy(PublisherPortData* const self)
{
    delete self;
}

iox_popo_AllocationResult
iox_pub_allocate_chunk(PublisherPortData* const self, void** const chunk, const uint32_t payloadSize)
{
    auto result =
        PublisherPortUser(self).tryAllocateChunk(payloadSize).and_then([&](ChunkHeader* h) { *chunk = h->payload(); });
    if (result.has_error())
    {
        switch (result.get_error())
        {
        case AllocationError::RUNNING_OUT_OF_CHUNKS:
            return AllocationResult_RUNNING_OUT_OF_CHUNKS;
        case AllocationError::TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL:
            return AllocationResult_TOO_MANY_CHUNKS_ALLOCATED_IN_PARALLEL;
        default:
            return AllocationResult_UNDEFINED_ERROR;
        }
    }

    return AllocationResult_SUCCESS;
}

void iox_pub_free_chunk(PublisherPortData* const self, void* const chunk)
{
    PublisherPortUser(self).freeChunk(convertPayloadPointerToChunkHeader(chunk));
}

void iox_pub_send_chunk(PublisherPortData* const self, void* const chunk)
{
    PublisherPortUser(self).sendChunk(convertPayloadPointerToChunkHeader(chunk));
}

const void* iox_pub_try_get_previous_chunk(PublisherPortData* const self)
{
    const void* returnValue = nullptr;
    PublisherPortUser(self).tryGetPreviousChunk().and_then([&](const ChunkHeader* h) { returnValue = h->payload(); });
    return returnValue;
}

void iox_pub_offer(PublisherPortData* const self)
{
    PublisherPortUser(self).offer();
}

void iox_pub_stop_offer(PublisherPortData* const self)
{
    PublisherPortUser(self).stopOffer();
}

bool iox_pub_is_offered(PublisherPortData* const self)
{
    return PublisherPortUser(self).isOffered();
}

bool iox_pub_has_subscribers(PublisherPortData* const self)
{
    return PublisherPortUser(self).hasSubscribers();
}

