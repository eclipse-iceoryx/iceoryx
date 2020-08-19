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

#include "iceoryx_binding_c/internal/subscriber.hpp"
#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"

using namespace iox;
using namespace iox::cxx;
using namespace iox::popo;
using namespace iox::capro;

#include <iostream>

SubscriberPortData* subscriber_new()
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    return new SubscriberPortData(
        ServiceDescription{1, 2, 3}, "bla", VariantQueueTypes::FiFo_SingleProducerSingleConsumer);
}

void subscriber_delete(SubscriberPortData* const self)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    delete self;
}

void subscriber_subscribe(SubscriberPortData* const self, const uint64_t queueCapacity)
{
    std::cout << __PRETTY_FUNCTION__ << std::endl;
    SubscriberPortUser(self).subscribe(queueCapacity);
}

void subscriber_unsubscribe(SubscriberPortData* const self)
{
    SubscriberPortUser(self).unsubscribe();
}

subscriber_SubscriptionState subscriber_getSubscriptionState(SubscriberPortData* const self)
{
    return static_cast<subscriber_SubscriptionState>(static_cast<int>(SubscriberPortUser(self).getSubscriptionState()));
}

subscriber_AllocateError subscriber_getChunk(SubscriberPortData* const self, const ChunkHeader** const header)
{
    auto result = SubscriberPortUser(self).getChunk();
    if (result.has_error())
    {
        return (result.get_error() == ChunkReceiveError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL)
                   ? subscriber_AllocateError::TOO_MANY_CHUNKS_HELD_IN_PARALLEL
                   : subscriber_AllocateError::INTERNAL_ERROR;
    }

    if (!result->has_value())
    {
        return subscriber_AllocateError::NO_CHUNK_RECEIVED;
    }

    *header = **result;
    return subscriber_AllocateError::SUCCESS;
}

void subscriber_releaseChunk(SubscriberPortData* const self, const ChunkHeader* const chunkHeader)
{
    SubscriberPortUser(self).releaseChunk(chunkHeader);
}

void subscriber_releaseQueuedChunks(SubscriberPortData* const self)
{
    SubscriberPortUser(self).releaseQueuedChunks();
}

bool subscriber_hasNewChunks(SubscriberPortData* const self)
{
    return SubscriberPortUser(self).hasNewChunks();
}

bool subscriber_hasLostChunks(SubscriberPortData* const self)
{
    return SubscriberPortUser(self).hasLostChunks();
}

void subscriber_attachConditionVariable(SubscriberPortData* const self)
{
    SubscriberPortUser(self).attachConditionVariable();
}

void subscriber_detachConditionVariable(SubscriberPortData* const self)
{
    SubscriberPortUser(self).detachConditionVariable();
}

bool subscriber_isConditionVariableAttached(SubscriberPortData* const self)
{
    return SubscriberPortUser(self).isConditionVariableAttached();
}

