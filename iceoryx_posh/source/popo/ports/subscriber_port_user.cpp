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
// limitations under the License

#include "iceoryx_posh/internal/popo/ports/subscriber_port_user.hpp"
#include "iceoryx_posh/internal/log/posh_logging.hpp"

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


void SubscriberPortUser::subscribe(const uint64_t queueCapacity) noexcept
{
    if (!getMembers()->m_subscribeRequested.load(std::memory_order_relaxed))
    {
        // start with new chunks, drop old ones that could be in the queue
        m_chunkReceiver.clear();

        /// @todo is it safe to change the capacity when it is no more the initial subscribe?
        /// What is the contract for changing the capacity?

        uint64_t capacity = queueCapacity;
        if (capacity > m_chunkReceiver.getMaximumCapacity())
        {
            LogWarn() << "Requested queue capacity " << queueCapacity
                      << " exceeds the maximum possible one for this subscriber"
                      << ", limiting to " << m_chunkReceiver.getMaximumCapacity();
            capacity = m_chunkReceiver.getMaximumCapacity();
        }
        m_chunkReceiver.setCapacity(capacity);

        getMembers()->m_subscribeRequested.store(true, std::memory_order_relaxed);
    }
}

void SubscriberPortUser::unsubscribe() noexcept
{
    if (getMembers()->m_subscribeRequested.load(std::memory_order_relaxed))
    {
        getMembers()->m_subscribeRequested.store(false, std::memory_order_relaxed);
    }
}

SubscribeState SubscriberPortUser::getSubscriptionState() const noexcept
{
    return getMembers()->m_subscriptionState;
}

cxx::expected<cxx::optional<const mepoo::ChunkHeader*>, ChunkReceiveError> SubscriberPortUser::getChunk() noexcept
{
    return m_chunkReceiver.get();
}

void SubscriberPortUser::releaseChunk(const mepoo::ChunkHeader* chunkHeader) noexcept
{
    m_chunkReceiver.release(chunkHeader);
}

void SubscriberPortUser::releaseQueuedChunks() noexcept
{
    m_chunkReceiver.clear();
}

bool SubscriberPortUser::hasNewChunks() noexcept
{
    return !m_chunkReceiver.empty();
}

bool SubscriberPortUser::hasLostChunks() noexcept
{
    return m_chunkReceiver.hasOverflown();
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
