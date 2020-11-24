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
inline cxx::expected<Trigger, WaitSetError>
WaitSet::acquireTrigger(T* const origin,
                        const cxx::ConstMethodCallback<bool>& triggerCallback,
                        const cxx::MethodCallback<void, const Trigger&>& invalidationCallback,
                        const uint64_t triggerId,
                        const Trigger::Callback<T> callback) noexcept
{
    if (!m_triggerVector.emplace_back(
            origin, m_conditionVariableDataPtr, triggerCallback, invalidationCallback, triggerId, callback))
    {
        return cxx::error<WaitSetError>(WaitSetError::TRIGGER_VECTOR_OVERFLOW);
    }

    return iox::cxx::success<Trigger>(Trigger(
        origin, m_conditionVariableDataPtr, triggerCallback, {this, &WaitSet::removeTrigger}, triggerId, callback));
}

template <typename T>
inline void WaitSet::moveTriggerOrigin(const Trigger& trigger, T* const newOrigin) noexcept
{
    for (auto& currentTrigger : m_triggerVector)
    {
        if (currentTrigger == trigger)
        {
            currentTrigger.setNewOrigin(newOrigin);
            return;
        }
    }
}


} // namespace popo
} // namespace iox

#endif

