// Copyright (c) 2020 by Robert Bosch GmbH, Apex.AI Inc. All rights reserved.
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

template <typename T, typename Subscriber, typename port_t>
inline BaseSubscriber<T, Subscriber, port_t>::BaseSubscriber() noexcept
{
}

template <typename T, typename Subscriber, typename port_t>
inline BaseSubscriber<T, Subscriber, port_t>::BaseSubscriber(const capro::ServiceDescription& service,
                                                             const SubscriberOptions& subscriberOptions) noexcept
    : m_port(iox::runtime::PoshRuntime::getInstance().getMiddlewareSubscriber(service, subscriberOptions))
{
}

template <typename T, typename Subscriber, typename port_t>
inline BaseSubscriber<T, Subscriber, port_t>::~BaseSubscriber()
{
    m_port.destroy();
}

template <typename T, typename Subscriber, typename port_t>
inline uid_t BaseSubscriber<T, Subscriber, port_t>::getUid() const noexcept
{
    return m_port.getUniqueID();
}

template <typename T, typename Subscriber, typename port_t>
inline capro::ServiceDescription /// todo #25 make this a reference.
BaseSubscriber<T, Subscriber, port_t>::getServiceDescription() const noexcept
{
    return m_port.getCaProServiceDescription();
}

template <typename T, typename Subscriber, typename port_t>
inline void BaseSubscriber<T, Subscriber, port_t>::subscribe() noexcept
{
    m_port.subscribe();
}

template <typename T, typename Subscriber, typename port_t>
inline SubscribeState BaseSubscriber<T, Subscriber, port_t>::getSubscriptionState() const noexcept
{
    return m_port.getSubscriptionState();
}

template <typename T, typename Subscriber, typename port_t>
inline void BaseSubscriber<T, Subscriber, port_t>::unsubscribe() noexcept
{
    m_port.unsubscribe();
}

template <typename T, typename Subscriber, typename port_t>
inline bool BaseSubscriber<T, Subscriber, port_t>::hasSamples() const noexcept
{
    return m_port.hasNewChunks();
}

template <typename T, typename Subscriber, typename port_t>
inline bool BaseSubscriber<T, Subscriber, port_t>::hasMissedSamples() noexcept
{
    return m_port.hasLostChunksSinceLastCall();
}

template <typename T, typename Subscriber, typename port_t>
inline cxx::expected<cxx::optional<Sample<const T>>, ChunkReceiveError>
BaseSubscriber<T, Subscriber, port_t>::take() noexcept
{
    auto result = m_port.tryGetChunk();
    if (result.has_error())
    {
        return cxx::error<ChunkReceiveError>(result.get_error());
    }
    else
    {
        auto optionalHeader = result.value();
        if (optionalHeader.has_value())
        {
            auto header = optionalHeader.value();
            auto samplePtr = cxx::unique_ptr<T>(reinterpret_cast<T*>(header->payload()), m_sampleDeleter);
            return cxx::success<cxx::optional<Sample<const T>>>(
                cxx::make_optional<Sample<const T>>(std::move(samplePtr)));
        }
        else
        {
            return cxx::success<cxx::optional<Sample<const T>>>(cxx::nullopt);
        }
    }
}

template <typename T, typename Subscriber, typename port_t>
inline void BaseSubscriber<T, Subscriber, port_t>::releaseQueuedSamples() noexcept
{
    m_port.releaseQueuedChunks();
}

template <typename T, typename Subscriber, typename port_t>
inline void BaseSubscriber<T, Subscriber, port_t>::invalidateTrigger(const uint64_t uniqueTriggerId) noexcept
{
    if (m_trigger.getUniqueId() == uniqueTriggerId)
    {
        m_port.unsetConditionVariable();
        m_trigger.invalidate();
    }
}

// ============================== Sample Deleter ============================== //

template <typename T, typename Subscriber, typename port_t>
inline BaseSubscriber<T, Subscriber, port_t>::SubscriberSampleDeleter::SubscriberSampleDeleter(port_t& port)
    : m_port(std::ref(port))
{
}

template <typename T, typename Subscriber, typename port_t>
inline void BaseSubscriber<T, Subscriber, port_t>::SubscriberSampleDeleter::operator()(T* const ptr) const
{
    auto header = mepoo::ChunkHeader::fromPayload(ptr);
    m_port.get().releaseChunk(header);
}

template <typename T, typename Subscriber, typename port_t>
template <uint64_t WaitSetCapacity>
inline cxx::expected<WaitSetError>
BaseSubscriber<T, Subscriber, port_t>::enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                                   [[gnu::unused]] const SubscriberEvent subscriberEvent,
                                                   const uint64_t eventId,
                                                   const EventInfo::Callback<Subscriber> callback) noexcept
{
    Subscriber* self = reinterpret_cast<Subscriber*>(this);

    return waitset
        .acquireTriggerHandle(
            self, {*this, &SelfType::hasSamples}, {*this, &SelfType::invalidateTrigger}, eventId, callback)
        .and_then([this](TriggerHandle& trigger) {
            m_trigger = std::move(trigger);
            m_port.setConditionVariable(m_trigger.getConditionVariableData());
        });
}

template <typename T, typename Subscriber, typename port_t>
template <uint64_t WaitSetCapacity>
inline cxx::expected<WaitSetError>
BaseSubscriber<T, Subscriber, port_t>::enableEvent(WaitSet<WaitSetCapacity>& waitset,
                                                   [[gnu::unused]] const SubscriberEvent subscriberEvent,
                                                   const EventInfo::Callback<Subscriber> callback) noexcept
{
    return enableEvent(waitset, subscriberEvent, EventInfo::INVALID_ID, callback);
}

template <typename T, typename Subscriber, typename port_t>
inline void BaseSubscriber<T, Subscriber, port_t>::disableEvent(const SubscriberEvent subscriberEvent) noexcept
{
    static_cast<void>(subscriberEvent);

    m_trigger.reset();
    m_port.unsetConditionVariable();
}


} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BASE_SUBSCRIBER_INL
