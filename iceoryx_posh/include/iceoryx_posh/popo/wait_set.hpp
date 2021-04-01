// Copyright (c) 2020 - 2021 by Robert Bosch GmbH. All rights reserved.
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
//
// SPDX-License-Identifier: Apache-2.0
#ifndef IOX_POSH_POPO_WAIT_SET_HPP
#define IOX_POSH_POPO_WAIT_SET_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/popo/enum_trigger_type.hpp"
#include "iceoryx_posh/popo/event_attorney.hpp"
#include "iceoryx_posh/popo/trigger.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/algorithm.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/cxx/helplets.hpp"
#include "iceoryx_utils/cxx/list.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"
#include "iceoryx_utils/cxx/stack.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

#include <typeinfo>

namespace iox
{
namespace popo
{
class Condition;

enum class WaitSetError : uint8_t
{
    INVALID_STATE,
    WAIT_SET_FULL,
    ALREADY_ATTACHED,
};


/// @brief Logical disjunction of a certain number of Triggers
///
/// The WaitSet stores Triggers and allows the user to wait till one or more of those Triggers are triggered. It works
/// over process borders. With the creation of a WaitSet it requests a condition variable from RouDi and destroys it
/// with the destructor. Hence the lifetime of the condition variable is bound to the lifetime of the WaitSet.
/// @param[in] Capacity the amount of events which can be attached to the waitset
template <uint64_t Capacity = MAX_NUMBER_OF_ATTACHMENTS_PER_WAITSET>
class WaitSet
{
  public:
    static constexpr uint64_t CAPACITY = Capacity;
    using TriggerArray = cxx::optional<Trigger>[Capacity];
    using EventInfoVector = cxx::vector<const EventInfo*, CAPACITY>;

    WaitSet() noexcept;
    ~WaitSet() noexcept;

    /// @brief all the Trigger have a pointer pointing to this waitset for cleanup
    ///        calls, therefore the WaitSet cannot be moved
    WaitSet(const WaitSet& rhs) = delete;
    WaitSet(WaitSet&& rhs) = delete;
    WaitSet& operator=(const WaitSet& rhs) = delete;
    WaitSet& operator=(WaitSet&& rhs) = delete;

    /// @brief attaches an event of a given class to the WaitSet.
    /// @param[in] eventOrigin the class from which the event originates.
    /// @param[in] eventType the event specified by the class
    /// @param[in] eventId an arbitrary user defined id for the event
    /// @param[in] eventCallback a callback which should be assigned to the event
    template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
    cxx::expected<WaitSetError> attachEvent(T& eventOrigin,
                                            const EventType eventType,
                                            const uint64_t eventId = 0U,
                                            const EventInfo::Callback<T>& eventCallback = {}) noexcept;

    /// @brief attaches an event of a given class to the WaitSet.
    /// @param[in] eventOrigin the class from which the event originates.
    /// @param[in] eventType the event specified by the class
    /// @param[in] eventCallback a callback which should be assigned to the event
    template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value, void>>
    cxx::expected<WaitSetError>
    attachEvent(T& eventOrigin, const EventType eventType, const EventInfo::Callback<T>& eventCallback) noexcept;

    /// @brief attaches an event of a given class to the WaitSet.
    /// @param[in] eventOrigin the class from which the event originates.
    /// @param[in] eventId an arbitrary user defined id for the event
    /// @param[in] eventCallback a callback which should be assigned to the event
    template <typename T>
    cxx::expected<WaitSetError>
    attachEvent(T& eventOrigin, const uint64_t eventId = 0U, const EventInfo::Callback<T>& eventCallback = {}) noexcept;

    /// @brief attaches an event of a given class to the WaitSet.
    /// @param[in] eventOrigin the class from which the event originates.
    /// @param[in] eventCallback a callback which should be assigned to the event
    template <typename T>
    cxx::expected<WaitSetError> attachEvent(T& eventOrigin, const EventInfo::Callback<T>& eventCallback) noexcept;

    /// @brief attaches a state of a given class to the WaitSet.
    /// @param[in] stateOrigin the class from which the state originates.
    /// @param[in] stateType the state specified by the class
    /// @param[in] id an arbitrary user defined id for the state
    /// @param[in] stateCallback a callback which should be assigned to the state
    template <typename T, typename StateType, typename = std::enable_if_t<std::is_enum<StateType>::value>>
    cxx::expected<WaitSetError> attachState(T& stateOrigin,
                                            const StateType stateType,
                                            const uint64_t id = 0U,
                                            const EventInfo::Callback<T>& stateCallback = {}) noexcept;

    /// @brief attaches a state of a given class to the WaitSet.
    /// @param[in] stateOrigin the class from which the state originates.
    /// @param[in] stateType the state specified by the class
    /// @param[in] stateCallback a callback which should be assigned to the state
    template <typename T, typename StateType, typename = std::enable_if_t<std::is_enum<StateType>::value, void>>
    cxx::expected<WaitSetError>
    attachState(T& stateOrigin, const StateType stateType, const EventInfo::Callback<T>& stateCallback) noexcept;

    /// @brief attaches a state of a given class to the WaitSet.
    /// @param[in] stateOrigin the class from which the state originates.
    /// @param[in] id an arbitrary user defined id for the state
    /// @param[in] stateCallback a callback which should be assigned to the state
    template <typename T>
    cxx::expected<WaitSetError>
    attachState(T& stateOrigin, const uint64_t id = 0U, const EventInfo::Callback<T>& stateCallback = {}) noexcept;

    /// @brief attaches a state of a given class to the WaitSet.
    /// @param[in] stateOrigin the class from which the state originates.
    /// @param[in] stateCallback a callback which should be assigned to the state
    template <typename T>
    cxx::expected<WaitSetError> attachState(T& stateOrigin, const EventInfo::Callback<T>& stateCallback) noexcept;

    /// @brief detaches an event from the WaitSet
    /// @param[in] eventOrigin the origin of the event that should be detached
    /// @param[in] args... additional event identifying arguments
    template <typename T, typename... Targs>
    void detachEvent(T& eventOrigin, const Targs&... args) noexcept;

    /// @brief detaches a state based trigger from the WaitSet
    /// @param[in] stateOrigin the origin of the state that should be detached
    /// @param[in] args... additional state identifying arguments
    template <typename T, typename... Targs>
    void detachState(T& stateOrigin, const Targs&... args) noexcept;

    /// @brief Blocking wait with time limit till one or more of the triggers are triggered
    /// @param[in] timeout How long shall we waite for a trigger
    /// @return EventInfoVector of EventInfos that have been triggered
    EventInfoVector timedWait(const units::Duration timeout) noexcept;

    /// @brief Blocking wait till one or more of the triggers are triggered
    /// @return EventInfoVector of EventInfos that have been triggered
    EventInfoVector wait() noexcept;

    /// @brief Returns the amount of stored Trigger inside of the WaitSet
    uint64_t size() const noexcept;

    /// @brief returns the maximum amount of triggers which can be acquired from a waitset
    static constexpr uint64_t capacity() noexcept;

  protected:
    explicit WaitSet(ConditionVariableData& condVarData) noexcept;

  private:
    enum class NoStateEnumUsed : StateEnumIdentifier
    {
        PLACEHOLDER
    };

    enum class NoEventEnumUsed : EventEnumIdentifier
    {
        PLACEHOLDER
    };

    using WaitFunction = cxx::function_ref<ConditionListener::NotificationVector_t()>;
    template <typename T>
    cxx::expected<uint64_t, WaitSetError> attachImpl(T& eventOrigin,
                                                     const WaitSetIsConditionSatisfiedCallback& hasTriggeredCallback,
                                                     const uint64_t eventId,
                                                     const EventInfo::Callback<T>& eventCallback,
                                                     const uint64_t originType,
                                                     const uint64_t originTypeHash) noexcept;

    EventInfoVector waitAndReturnTriggeredTriggers(const WaitFunction& wait) noexcept;
    EventInfoVector createVectorWithTriggeredTriggers() noexcept;

    void removeTrigger(const uint64_t uniqueTriggerId) noexcept;
    void removeAllTriggers() noexcept;
    void acquireNotifications(const WaitFunction& wait) noexcept;

  private:
    /// needs to be a list since we return pointer to the underlying EventInfo class with wait
    TriggerArray m_triggerArray;
    ConditionVariableData* m_conditionVariableDataPtr{nullptr};
    ConditionListener m_conditionListener;

    cxx::stack<uint64_t, Capacity> m_indexRepository;
    ConditionListener::NotificationVector_t m_activeNotifications;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/wait_set.inl"

#endif // IOX_POSH_POPO_WAIT_SET_HPP
