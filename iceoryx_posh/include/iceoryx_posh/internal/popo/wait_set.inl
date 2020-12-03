// Copyright (c) 2020 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_WAIT_SET_INL
#define IOX_POSH_POPO_WAIT_SET_INL

namespace iox
{
namespace popo
{
template <typename T>
inline cxx::expected<TriggerHandle, WaitSetError>
WaitSet::acquireTrigger(T* const origin,
                        const cxx::ConstMethodCallback<bool>& triggerCallback,
                        const cxx::MethodCallback<void, uint64_t>& invalidationCallback,
                        const uint64_t triggerId,
                        const Trigger::Callback<T> callback) noexcept
{
    static_assert(!std::is_copy_constructible<T>::value && !std::is_copy_assignable<T>::value
                      && !std::is_move_assignable<T>::value && !std::is_move_constructible<T>::value,
                  "At the moment only non copyable and non movable origin types are supported! To implement this we "
                  "have to notify the WaitSet when origin moves about the new pointer to origin. This could be done in "
                  "a callback inside of Trigger.");

    Trigger logicalEqualTrigger(origin,
                                m_conditionVariableDataPtr,
                                triggerCallback,
                                cxx::MethodCallback<void, uint64_t>(),
                                triggerId,
                                Trigger::Callback<T>());

    // it is not allowed to have to logical equal trigger in the same waitset
    // otherwise when we call removeTrigger(Trigger) we do not know which trigger
    // we should remove if the trigger is attached multiple times.
    for (auto& currentTrigger : m_triggerVector)
    {
        if (currentTrigger.isLogicalEqualTo(logicalEqualTrigger))
        {
            return cxx::error<WaitSetError>(WaitSetError::TRIGGER_ALREADY_ACQUIRED);
        }
    }

    if (!m_triggerVector.emplace_back(
            origin, m_conditionVariableDataPtr, triggerCallback, invalidationCallback, triggerId, callback))
    {
        return cxx::error<WaitSetError>(WaitSetError::TRIGGER_VECTOR_OVERFLOW);
    }

    return iox::cxx::success<TriggerHandle>(TriggerHandle(
        m_conditionVariableDataPtr, {this, &WaitSet::removeTrigger}, m_triggerVector.back().getUniqueId()));
}

template <typename T>
inline void WaitSet::moveOriginOfTrigger(const Trigger& trigger, T* const newOrigin) noexcept
{
    for (auto& currentTrigger : m_triggerVector)
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
