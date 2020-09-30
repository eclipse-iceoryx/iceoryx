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

#include "iceoryx_binding_c/internal/cpp2c_enum_translation.hpp"
#include "iceoryx_binding_c/internal/cpp2c_publisher.hpp"
#include "iceoryx_posh/internal/popo/ports/publisher_port_user.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

using namespace iox;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/publisher.h"
}

iox_pub_t iox_pub_init(iox_pub_storage_t* self,
                       const char* service,
                       const char* instance,
                       const char* event,
                       const uint64_t historyCapacity)
{
    new (self) cpp2c_Publisher();
    iox_pub_t me = reinterpret_cast<iox_pub_t>(self);
    me->m_portData = PoshRuntime::getInstance().getMiddlewarePublisher(
        ServiceDescription{
            IdString(TruncateToCapacity, service),
            IdString(TruncateToCapacity, instance),
            IdString(TruncateToCapacity, event),
        },
        historyCapacity);
    return me;
}

void iox_pub_deinit(iox_pub_t const self)
{
    self->m_portData->m_toBeDestroyed.store(true);
    self->~cpp2c_Publisher();
}

iox_AllocationResult iox_pub_allocate_chunk(iox_pub_t const self, void** const chunk, const uint32_t payloadSize)
{
    auto result = PublisherPortUser(self->m_portData).tryAllocateChunk(payloadSize).and_then([&](ChunkHeader* h) {
        *chunk = h->payload();
    });
    if (result.has_error())
    {
        return cpp2c::AllocationResult(result.get_error());
    }

    return AllocationResult_SUCCESS;
}

void iox_pub_free_chunk(iox_pub_t const self, void* const chunk)
{
    PublisherPortUser(self->m_portData).freeChunk(convertPayloadPointerToChunkHeader(chunk));
}

void iox_pub_send_chunk(iox_pub_t const self, void* const chunk)
{
    PublisherPortUser(self->m_portData).sendChunk(convertPayloadPointerToChunkHeader(chunk));
}

const void* iox_pub_try_get_previous_chunk(iox_pub_t const self)
{
    const void* returnValue = nullptr;
    PublisherPortUser(self->m_portData).tryGetPreviousChunk().and_then([&](const ChunkHeader* h) {
        returnValue = h->payload();
    });
    return returnValue;
}

void iox_pub_offer(iox_pub_t const self)
{
    PublisherPortUser(self->m_portData).offer();
}

void iox_pub_stop_offer(iox_pub_t const self)
{
    PublisherPortUser(self->m_portData).stopOffer();
}

bool iox_pub_is_offered(iox_pub_t const self)
{
    return PublisherPortUser(self->m_portData).isOffered();
}

bool iox_pub_has_subscribers(iox_pub_t const self)
{
    return PublisherPortUser(self->m_portData).hasSubscribers();
}
