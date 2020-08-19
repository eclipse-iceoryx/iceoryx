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


#include "iceoryx_posh/internal/popo/ports/subscriber_port_data.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/mepoo/chunk_header.hpp"

using namespace iox;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::capro;
using namespace iox::mepoo;

extern "C" {
#include "iceoryx_binding_c/subscriber.h"
}

SubscriberPortData* Subscriber_new()
{
    return new SubscriberPortData(
        ServiceDescription{1, 2, 3}, "bla", VariantQueueTypes::FiFo_SingleProducerSingleConsumer);
}

void Subscriber_delete(SubscriberPortData* const self)
{
    delete self;
}

void Subscriber_subscribe(SubscriberPortData* const self, const uint64_t queueCapacity)
{
    SubscriberPortUser(self).subscribe(queueCapacity);
}

void Subscriber_unsubscribe(SubscriberPortData* const self)
{
    SubscriberPortUser(self).unsubscribe();
}

Subscriber_SubscriptionState Subscriber_getSubscriptionState(SubscriberPortData* const self)
{
    return static_cast<Subscriber_SubscriptionState>(static_cast<int>(SubscriberPortUser(self).getSubscriptionState()));
}

Subscriber_AllocateError Subscriber_getChunk(SubscriberPortData* const self, const ChunkHeader** const header)
{
    auto result = SubscriberPortUser(self).getChunk();
    if (result.has_error())
    {
        return (result.get_error() == ChunkReceiveError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL)
                   ? Subscriber_AllocateError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL
                   : Subscriber_AllocateError::INTERNAL_ERROR;
    }

    if (!result->has_value())
    {
        return Subscriber_AllocateError::NO_CHUNK_RECEIVED;
    }

    *header = **result;
    return Subscriber_AllocateError::SUCCESS;
}

void Subscriber_releaseChunk(SubscriberPortData* const self, const ChunkHeader* const chunkHeader)
{
    SubscriberPortUser(self).releaseChunk(chunkHeader);
}

void Subscriber_releaseQueuedChunks(SubscriberPortData* const self)
{
    SubscriberPortUser(self).releaseQueuedChunks();
}

bool Subscriber_hasNewChunks(SubscriberPortData* const self)
{
    return SubscriberPortUser(self).hasNewChunks();
}

bool Subscriber_hasLostChunks(SubscriberPortData* const self)
{
    return SubscriberPortUser(self).hasLostChunks();
}

void Subscriber_attachConditionVariable(SubscriberPortData* const self)
{
    SubscriberPortUser(self).attachConditionVariable();
}

void Subscriber_detachConditionVariable(SubscriberPortData* const self)
{
    SubscriberPortUser(self).detachConditionVariable();
}

bool Subscriber_isConditionVariableAttached(SubscriberPortData* const self)
{
    return SubscriberPortUser(self).isConditionVariableAttached();
}

