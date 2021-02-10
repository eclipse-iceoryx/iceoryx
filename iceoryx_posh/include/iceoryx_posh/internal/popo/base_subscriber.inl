// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2020 - 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_BASE_SUBSCRIBER_INL
#define IOX_POSH_POPO_BASE_SUBSCRIBER_INL

namespace iox
{
namespace popo
{
// ============================== BaseSubscriber ============================== //

template <typename T, typename port_t>
inline BaseSubscriber<T, port_t>::BaseSubscriber() noexcept
{
}

template <typename T, typename port_t>
inline BaseSubscriber<T, port_t>::BaseSubscriber(const capro::ServiceDescription& service,
                                                 const SubscriberOptions& subscriberOptions) noexcept
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareSubscriber(service, subscriberOptions))
{
}

template <typename T, typename port_t>
inline BaseSubscriber<T, port_t>::~BaseSubscriber()
{
    m_port.destroy();
}

template <typename T, typename port_t>
inline uid_t BaseSubscriber<T, port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename T, typename port_t>
inline capro::ServiceDescription /// todo #25 make this a reference.
BaseSubscriber<T, port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::subscribe() noexcept
{
    m_port.subscribe();
}

template <typename T, typename port_t>
inline SubscribeState BaseSubscriber<T, port_t>::getSubscriptionState() const noexcept
{
    return m_port.getSubscriptionState();
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::unsubscribe() noexcept
{
    m_port.unsubscribe();
}

template <typename T, typename port_t>
inline bool BaseSubscriber<T, port_t>::hasData() const noexcept
{
    return m_port.hasNewChunks();
}

template <typename T, typename port_t>
inline bool BaseSubscriber<T, port_t>::hasMissedData() noexcept
{
    return m_port.hasLostChunksSinceLastCall();
}

template <typename T, typename port_t>
inline cxx::expected<const mepoo::ChunkHeader*, ChunkReceiveResult> BaseSubscriber<T, port_t>::takeChunk() noexcept
{
    auto result = m_port.tryGetChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveResult>(result.get_error());
    }
    else
    {
        auto maybeHeader = result.value();
        if (maybeHeader.has_value())
        {
            return cxx::success<const mepoo::ChunkHeader*>(maybeHeader.value());
        }
    }
    ///@todo: optimization - we could move this to a tryGetChunk but then we should remove expected<optional<>> there in
    /// the call chain
    return cxx::error<ChunkReceiveResult>(ChunkReceiveResult::NO_CHUNK_AVAILABLE);
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::releaseQueuedData() noexcept
{
    m_port.releaseQueuedChunks();
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

template <typename T, typename port_t>
template <uint64_t WaitSetCapacity, typename Subscriber>
inline cxx::expected<WaitSetError>
BaseSubscriber<T, port_t>::enableEventInternal(WaitSet<WaitSetCapacity>& waitset,
                                               [[gnu::unused]] const SubscriberEvent subscriberEvent,
                                               const uint64_t eventId,
                                               const EventInfo::Callback<Subscriber> callback) noexcept
{
    static_assert(std::is_base_of<SelfType, Subscriber>::value, "Subscriber must inherit from BaseSubscriber");
    Subscriber* self = reinterpret_cast<Subscriber*>(this);

    return waitset
        .acquireTriggerHandle(
            self, {*this, &SelfType::hasData}, {*this, &SelfType::invalidateTrigger}, eventId, callback)
        .and_then([this](TriggerHandle& trigger) {
            m_trigger = std::move(trigger);
            m_port.setConditionVariable(m_trigger.getConditionVariableData());
        });
}

template <typename T, typename port_t>
inline void BaseSubscriber<T, port_t>::disableEvent(const SubscriberEvent subscriberEvent) noexcept
{
    static_cast<void>(subscriberEvent);

    m_trigger.reset();
    m_port.unsetConditionVariable();
}

template <typename T, typename port_t>
inline const port_t& BaseSubscriber<T, port_t>::port() const noexcept
{
    return m_port;
}

template <typename T, typename port_t>
inline port_t& BaseSubscriber<T, port_t>::port() noexcept
{
    return m_port;
}


} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_SUBSCRIBER_INL
