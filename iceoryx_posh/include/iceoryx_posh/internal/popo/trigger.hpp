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

#ifndef IOX_POSH_POPO_TRIGGER_HPP
#define IOX_POSH_POPO_TRIGGER_HPP

#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"

namespace iox
{
namespace popo
{
class Condition;
struct TriggerId
{
    TriggerId() = default;

    TriggerId(uint64_t classId)
        : m_classId(classId)
    {
        static uint64_t currentInstanceId = 0U;
        m_instanceId = currentInstanceId++;
    }

    uint64_t m_classId = 0U;
    uint64_t m_instanceId = 0U;
};

class Trigger
{
  public:
    Trigger() noexcept = default;
    Trigger(Condition* condition,
            const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
            const cxx::MethodCallback<void>& invalidationCallback,
            ConditionVariableData* conditionVariableDataPtr,
            const uint64_t classId) noexcept;

    // TODO remove
    Trigger(Condition* condition,
            const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
            const cxx::MethodCallback<void>& invalidationCallback,
            ConditionVariableData* conditionVariableDataPtr,
            const TriggerId& trigger) noexcept
        : m_condition(condition)
        , m_conditionVariableDataPtr(conditionVariableDataPtr)
        , m_invalidationCallback(invalidationCallback)
        , m_hasTriggeredCallback(hasTriggeredCallback)
        , m_triggerId(trigger)
    {
    }

    Trigger(const Trigger& other, const cxx::MethodCallback<void, Trigger&>& removalCallback) noexcept;

    Trigger(const Trigger&) = delete;
    Trigger& operator=(const Trigger&) = delete;
    Trigger(Trigger&& rhs) noexcept;
    Trigger& operator=(Trigger&& rhs) noexcept;

    ~Trigger();

    explicit operator bool() const noexcept;
    bool isValid() const noexcept;

    bool hasTriggered() const noexcept;
    void invalidate() noexcept;
    void reset() noexcept;

    TriggerId getTriggerId() const noexcept;

    bool operator==(const Trigger& rhs) const noexcept;
    bool operator==(const void*) const noexcept;

    // private:

    Condition* m_condition;
    ConditionVariableData* m_conditionVariableDataPtr{nullptr};

    cxx::MethodCallback<void, Trigger&> m_removalCallback;
    cxx::MethodCallback<void> m_invalidationCallback;
    cxx::ConstMethodCallback<bool> m_hasTriggeredCallback;

    TriggerId m_triggerId;
};


} // namespace popo
} // namespace iox

#endif
