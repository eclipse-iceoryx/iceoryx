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
// limitations under the License

#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"

namespace iox
{
namespace popo
{
SubscriberPortUser::SubscriberPortUser(cxx::not_null<MemberType_t* const> subscriberPortDataPtr) noexcept
    : m_subscriberPortDataPtr(subscriberPortDataPtr)
    , m_chunkReceiver(&m_subscriberPortDataPtr->m_chunkReceiverData)

{
}

const SubscriberPortUser::MemberType_t* SubscriberPortUser::getMembers() const noexcept
{
    return m_subscriberPortDataPtr;
}

SubscriberPortUser::MemberType_t* SubscriberPortUser::getMembers() noexcept
{
    return m_subscriberPortDataPtr;
}


void SubscriberPortUser::subscribe(const uint32_t queueCapacity) noexcept
{
}

void SubscriberPortUser::unsubscribe() noexcept
{
}

SubscribeState SubscriberPortUser::getSubscriptionState() const noexcept
{
    return SubscribeState::NOT_SUBSCRIBED;
}

cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiveError> SubscriberPortUser::getChunk() noexcept
{
    return cxx::success<cxx::optional<const mepoo::ChunkHeader*>>(cxx::nullopt_t());
}

void SubscriberPortUser::releaseChunk(const mepoo::ChunkHeader* chunkHeader) noexcept
{
}

void SubscriberPortUser::releaseQueuedChunks() noexcept
{
}

bool SubscriberPortUser::hasNewChunks() noexcept
{
    return false;
}

bool SubscriberPortUser::hasLostChunks() noexcept
{
    return true;
}

void SubscriberPortUser::attachConditionVariable() noexcept
{
}

void SubscriberPortUser::detachConditionVaribale() noexcept
{
}

bool SubscriberPortUser::isConditionVariableAttached() noexcept
{
    return false;
}

} // namespace popo
} // namespace iox
