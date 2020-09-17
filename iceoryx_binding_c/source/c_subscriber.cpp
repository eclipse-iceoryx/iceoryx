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
#include "iceoryx_binding_c/internal/cpp2c_subscriber.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"
#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"

using namespace iox;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;
using namespace iox::runtime;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
}

iox_sub_t iox_sub_init(iox_sub_storage_t* self,
                       const char* const service,
                       const char* const instance,
                       const char* const event,
                       uint64_t historyRequest)
{
    new (self) cpp2c_Subscriber();
    iox_sub_t me = reinterpret_cast<iox_sub_t>(self);
    me->m_portData =
        PoshRuntime::getInstance().getMiddlewareSubscriber(ServiceDescription{IdString(TruncateToCapacity, service),
                                                                              IdString(TruncateToCapacity, instance),
                                                                              IdString(TruncateToCapacity, event)},
                                                           historyRequest);

    return me;
}

void iox_sub_deinit(iox_sub_t const self)
{
    self->~cpp2c_Subscriber();
}

void iox_sub_subscribe(iox_sub_t const self, const uint64_t queueCapacity)
{
    SubscriberPortUser(self->m_portData).subscribe(queueCapacity);
}

void iox_sub_unsubscribe(iox_sub_t const self)
{
    SubscriberPortUser(self->m_portData).unsubscribe();
}

iox_SubscribeState iox_sub_get_subscription_state(iox_sub_t const self)
{
    return cpp2c::SubscribeState(SubscriberPortUser(self->m_portData).getSubscriptionState());
}

iox_ChunkReceiveResult iox_sub_get_chunk(iox_sub_t const self, const void** const payload)
{
    auto result = SubscriberPortUser(self->m_portData).tryGetChunk();
    if (result.has_error())
    {
        return cpp2c::ChunkReceiveResult(result.get_error());
    }

    if (!result->has_value())
    {
        return ChunkReceiveResult_NO_CHUNK_RECEIVED;
    }

    *payload = (**result)->payload();
    return ChunkReceiveResult_SUCCESS;
}

void iox_sub_release_chunk(iox_sub_t const self, const void* const chunk)
{
    SubscriberPortUser(self->m_portData).releaseChunk(convertPayloadPointerToChunkHeader(chunk));
}

void iox_sub_release_queued_chunks(iox_sub_t const self)
{
    SubscriberPortUser(self->m_portData).releaseQueuedChunks();
}

bool iox_sub_has_new_chunks(iox_sub_t const self)
{
    return SubscriberPortUser(self->m_portData).hasNewChunks();
}

bool iox_sub_has_lost_chunks(iox_sub_t const self)
{
    return SubscriberPortUser(self->m_portData).hasLostChunksSinceLastCall();
}
