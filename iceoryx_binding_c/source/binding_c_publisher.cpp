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
Publisher_new(const char* service, const char* instance, const char* event, const uint64_t historyCapacity)
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

void Publisher_delete(PublisherPortData* const self)
{
    delete self;
}

iox_popo_AllocationError
Publisher_allocateChunk(PublisherPortData* const self, void** const chunk, const uint32_t payloadSize)
{
    auto result =
        PublisherPortUser(self).allocateChunk(payloadSize).and_then([&](ChunkHeader* h) { *chunk = h->payload(); });
    if (result.has_error())
    {
        return static_cast<iox_popo_AllocationError>(static_cast<int>(result.get_error()));
    }

    return AllocationError_SUCCESS;
}

void Publisher_freeChunk(PublisherPortData* const self, void* const chunk)
{
    PublisherPortUser(self).freeChunk(convertPayloadPointerToChunkHeader(chunk));
}

void Publisher_sendChunk(PublisherPortData* const self, void* const chunk)
{
    PublisherPortUser(self).sendChunk(convertPayloadPointerToChunkHeader(chunk));
}

const void* Publisher_getLastChunk(PublisherPortData* const self)
{
    const void* returnValue = nullptr;
    PublisherPortUser(self).getLastChunk().and_then([&](const ChunkHeader* h) { returnValue = h->payload(); });
    return returnValue;
}

void Publisher_offer(PublisherPortData* const self)
{
    PublisherPortUser(self).offer();
}

void Publisher_stopOffer(PublisherPortData* const self)
{
    PublisherPortUser(self).stopOffer();
}

bool Publisher_isOffered(PublisherPortData* const self)
{
    return PublisherPortUser(self).isOffered();
}

bool Publisher_hasSubscribers(PublisherPortData* const self)
{
    return PublisherPortUser(self).hasSubscribers();
}

