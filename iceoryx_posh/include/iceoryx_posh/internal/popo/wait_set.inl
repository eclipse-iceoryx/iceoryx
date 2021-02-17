// Copyright (c) 2020, 2021 by Apex.AI Inc. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0

#ifndef IOX_POSH_POPO_WAIT_SET_INL
#define IOX_POSH_POPO_WAIT_SET_INL

namespace iox
{
namespace popo
{
template <uint64_t Capacity>
inline WaitSet<Capacity>::WaitSet() noexcept
    : WaitSet(runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable())
{
}

template <uint64_t Capacity>
inline WaitSet<Capacity>::WaitSet(cxx::not_null<ConditionVariableData* const> condVarDataPtr) noexcept
    : m_conditionVariableDataPtr(condVarDataPtr)
    , m_conditionVariableWaiter(m_conditionVariableDataPtr)
{
}

template <uint64_t Capacity>
inline WaitSet<Capacity>::~WaitSet() noexcept
{
    removeAllTriggers();
    m_conditionVariableDataPtr->m_toBeDestroyed.store(true, std::memory_order_relaxed);
}

template <uint64_t Capacity>
template <typename T>
inline cxx::expected<uint64_t, WaitSetError>
WaitSet<Capacity>::attachEventImpl(T& eventOrigin,
                                   const WaitSetHasTriggeredCallback& hasTriggeredCallback,
                                   const uint64_t eventId,
                                   const EventInfo::Callback<T>& eventCallback) noexcept
{
    if (!hasTriggeredCallback)
    {
        return cxx::error<WaitSetError>(WaitSetError::PROVIDED_HAS_TRIGGERED_CALLBACK_IS_UNSET);
    }

    Trigger possibleLogicallyEqualTrigger(
        &eventOrigin, hasTriggeredCallback, cxx::MethodCallback<void, uint64_t>(), eventId, Trigger::Callback<T>());

    for (auto& currentTrigger : m_triggerList)
    {
        if (currentTrigger.isLogicalEqualTo(possibleLogicallyEqualTrigger))
        {
            return cxx::error<WaitSetError>(WaitSetError::EVENT_ALREADY_ATTACHED);
        }
    }

    cxx::MethodCallback<void, uint64_t> invalidationCallback = EventAttorney::getInvalidateTriggerMethod(eventOrigin);

    if (!m_triggerList.push_back(
            Trigger{&eventOrigin, hasTriggeredCallback, invalidationCallback, eventId, eventCallback}))
    {
        return cxx::error<WaitSetError>(WaitSetError::WAIT_SET_FULL);
    }

    return cxx::success<uint64_t>(m_triggerList.back().getUniqueId());
}

template <uint64_t Capacity>
template <typename T, typename EventType, typename>
inline cxx::expected<WaitSetError> WaitSet<Capacity>::attachEvent(T& eventOrigin,
                                                                  const EventType eventType,
                                                                  const uint64_t eventId,
                                                                  const EventInfo::Callback<T>& eventCallback) noexcept
{
    auto hasTriggeredCallback = EventAttorney::getHasTriggeredCallbackForEvent(eventOrigin, eventType);

    return attachEventImpl(eventOrigin, hasTriggeredCallback, eventId, eventCallback).and_then([&](auto& uniqueId) {
        EventAttorney::enableEvent(
            eventOrigin,
            TriggerHandle(*m_conditionVariableDataPtr, {*this, &WaitSet::removeTrigger}, uniqueId),
            eventType);
    });
}

template <uint64_t Capacity>
template <typename T, typename EventType, typename>
inline cxx::expected<WaitSetError> WaitSet<Capacity>::attachEvent(T& eventOrigin,
                                                                  const EventType eventType,
                                                                  const EventInfo::Callback<T>& eventCallback) noexcept
{
    return attachEvent(eventOrigin, eventType, EventInfo::INVALID_ID, eventCallback);
}

template <uint64_t Capacity>
template <typename T>
inline cxx::expected<WaitSetError> WaitSet<Capacity>::attachEvent(T& eventOrigin,
                                                                  const uint64_t eventId,
                                                                  const EventInfo::Callback<T>& eventCallback) noexcept
{
    auto hasTriggeredCallback = EventAttorney::getHasTriggeredCallbackForEvent(eventOrigin);

    return attachEventImpl(eventOrigin, hasTriggeredCallback, eventId, eventCallback).and_then([&](auto& uniqueId) {
        EventAttorney::enableEvent(
            eventOrigin, TriggerHandle(*m_conditionVariableDataPtr, {*this, &WaitSet::removeTrigger}, uniqueId));
    });
}

template <uint64_t Capacity>
template <typename T>
cxx::expected<WaitSetError> WaitSet<Capacity>::attachEvent(T& eventOrigin,
                                                           const EventInfo::Callback<T>& eventCallback) noexcept
{
    return attachEvent(eventOrigin, EventInfo::INVALID_ID, eventCallback);
}

template <uint64_t Capacity>
template <typename T, typename... Targs>
inline void WaitSet<Capacity>::detachEvent(T& eventOrigin, const Targs&... args) noexcept
{
    EventAttorney::disableEvent(eventOrigin, args...);
}

template <uint64_t Capacity>
inline void WaitSet<Capacity>::removeTrigger(const uint64_t uniqueTriggerId) noexcept
{
    for (auto currentTrigger = m_triggerList.begin(); currentTrigger != m_triggerList.end(); ++currentTrigger)
    {
        if (currentTrigger->getUniqueId() == uniqueTriggerId)
        {
            currentTrigger->invalidate();
            m_triggerList.erase(currentTrigger);
            return;
        }
    }
}

template <uint64_t Capacity>
inline void WaitSet<Capacity>::removeAllTriggers() noexcept
{
    for (auto& trigger : m_triggerList)
    {
        trigger.reset();
    }

    m_triggerList.clear();
}

template <uint64_t Capacity>
inline typename WaitSet<Capacity>::EventInfoVector WaitSet<Capacity>::timedWait(const units::Duration timeout) noexcept
{
    return waitAndReturnTriggeredTriggers([this, timeout] { return !m_conditionVariableWaiter.timedWait(timeout); });
}

template <uint64_t Capacity>
inline typename WaitSet<Capacity>::EventInfoVector WaitSet<Capacity>::wait() noexcept
{
    return waitAndReturnTriggeredTriggers([this] {
        m_conditionVariableWaiter.wait();
        return false;
    });
}

template <uint64_t Capacity>
inline typename WaitSet<Capacity>::EventInfoVector WaitSet<Capacity>::createVectorWithTriggeredTriggers() noexcept
{
    EventInfoVector triggers;
    for (auto& currentTrigger : m_triggerList)
    {
        if (currentTrigger.hasTriggered())
        {
            // We do not need to verify if push_back was successful since
            // m_conditionVector and triggers are having the same type, a
            // vector with the same guaranteed capacity.
            // Therefore it is guaranteed that push_back works!
            triggers.push_back(&currentTrigger.getEventInfo());
        }
    }

    return triggers;
}

template <uint64_t Capacity>
template <typename WaitFunction>
inline typename WaitSet<Capacity>::EventInfoVector
WaitSet<Capacity>::waitAndReturnTriggeredTriggers(const WaitFunction& wait) noexcept
{
    WaitSet::EventInfoVector triggers;

    /// Inbetween here and last wait someone could have set the trigger to true, hence reset it.
    m_conditionVariableWaiter.reset();
    triggers = createVectorWithTriggeredTriggers();

    // It is possible that after the reset call and before the createVectorWithTriggeredTriggers call
    // another trigger came in. Then createVectorWithTriggeredTriggers would have already handled it.
    // But this would lead to an empty triggers vector in the next run if no other trigger
    // came in.
    if (!triggers.empty())
    {
        return triggers;
    }

    return (wait()) ? triggers : createVectorWithTriggeredTriggers();
}

template <uint64_t Capacity>
inline uint64_t WaitSet<Capacity>::size() const noexcept
{
    return m_triggerList.size();
}

template <uint64_t Capacity>
inline uint64_t WaitSet<Capacity>::capacity() const noexcept
{
    return m_triggerList.capacity();
}

template <uint64_t Capacity>
template <typename T>
inline void WaitSet<Capacity>::moveOriginOfTrigger(const Trigger& trigger, T* const newOrigin) noexcept
{
    for (auto& currentTrigger : m_triggerList)
    {
        if (currentTrigger.isLogicalEqualTo(trigger))
        {
            currentTrigger.updateOrigin(newOrigin);
        }
    }
}

} // namespace popo
} // namespace iox

#endif
