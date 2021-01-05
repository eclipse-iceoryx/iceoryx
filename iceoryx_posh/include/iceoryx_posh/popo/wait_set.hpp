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
#ifndef IOX_POSH_POPO_WAIT_SET_HPP
#define IOX_POSH_POPO_WAIT_SET_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_data.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"
#include "iceoryx_posh/popo/trigger.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/cxx/list.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

namespace iox
{
namespace popo
{
class Condition;

enum class WaitSetError : uint8_t
{
    WAIT_SET_FULL,
    EVENT_ALREADY_ATTACHED,
};

/// @brief Logical disjunction of a certain number of Triggers
///
/// The WaitSet stores Triggers and allows the user to wait till one or more of those Triggers are triggered. It works
/// over process borders. With the creation of a WaitSet it requests a condition variable from RouDi and destroys it
/// with the destructor. Hence the lifetime of the condition variable is bound to the lifetime of the WaitSet.
/// @param[in] Capacity the amount of events which can be attached to the waitset
template <uint64_t Capacity = MAX_NUMBER_OF_EVENTS_PER_WAITSET>
class WaitSet
{
  public:
    static constexpr uint64_t CAPACITY = Capacity;
    using TriggerList = cxx::list<Trigger, CAPACITY>;
    using EventInfoVector = cxx::vector<const EventInfo*, CAPACITY>;

    WaitSet() noexcept;
    ~WaitSet() noexcept;

    /// @brief all the Trigger have a pointer pointing to this waitset for cleanup
    ///        calls, therefore the WaitSet cannot be moved
    WaitSet(const WaitSet& rhs) = delete;
    WaitSet(WaitSet&& rhs) = delete;
    WaitSet& operator=(const WaitSet& rhs) = delete;
    WaitSet& operator=(WaitSet&& rhs) = delete;

    /// @brief attaches an event to the WaitSet
    /// @param[in] eventOrigin the origin of the event
    /// @param[in] args... additional event identifying arguments
    /// @return when an error occurs an enum which is describing the error is returned
    template <typename T, typename... Targs>
    cxx::expected<WaitSetError> attachEvent(T& eventOrigin, const Targs&... args) noexcept;

    /// @brief detaches an event from the WaitSet
    /// @param[in] eventOrigin the origin of the event that should be detached
    /// @param[in] args... additional event identifying arguments
    template <typename T, typename... Targs>
    void detachEvent(T& eventOrigin, const Targs&... args) noexcept;

    /// @brief Acquires a trigger from the waitset  The trigger is then attached to an object, the origin,
    ///        which triggers the Trigger if a specific event happens. The object must then signal the
    ///        trigger that it was triggered via the triggerCallback. If the WaitSet goes out of scope
    ///        before the object does it calls the invalidationCallback to invalidate the Trigger inside
    ///        of the object
    ///        You cannot acquire an already logically equal acquired trigger. This means if you acquire a trigger
    ///        twice with the same: origin, triggerCallback and eventId this method will return
    ///        EVENT_ALREADY_ATTACHED
    /// @param[in] origin the pointer to the object which will attach the trigger
    /// @param[in] triggerCallback a method from the object which will signal the Trigger that it was triggered
    /// @param[in] invalidationCallback callback which will be called in the destructor of the waitset, important when
    /// the waitset goes out of scope before the origin does.
    /// @param[in] eventId an arbitrary id to identify the trigger later when a list of triggers is returned via wait
    /// or timedWait
    /// @param[in] callback a callback which is attached to the trigger and can be later called when the trigger will be
    ///            returned via wait or timedWait
    /// @return returns the newly created trigger if the WaitSet has space left otherwise it returns
    /// WaitSetError::WAIT_SET_FULL
    template <typename T>
    cxx::expected<TriggerHandle, WaitSetError>
    acquireTriggerHandle(T* const origin,
                         const cxx::ConstMethodCallback<bool>& triggerCallback,
                         const cxx::MethodCallback<void, uint64_t>& invalidationCallback,
                         const uint64_t eventId = EventInfo::INVALID_ID,
                         const EventInfo::Callback<T> callback = nullptr) noexcept;

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
    uint64_t capacity() const noexcept;

  protected:
    explicit WaitSet(cxx::not_null<ConditionVariableData* const>) noexcept;

  private:
    EventInfoVector waitAndReturnTriggeredTriggers(const units::Duration& timeout) noexcept;
    template <typename WaitFunction>
    EventInfoVector waitAndReturnTriggeredTriggers(const WaitFunction& wait) noexcept;
    EventInfoVector createVectorWithTriggeredTriggers() noexcept;

    template <typename T>
    void moveOriginOfTrigger(const Trigger& trigger, T* const newOrigin) noexcept;

    void removeTrigger(const uint64_t uniqueTriggerId) noexcept;
    void removeAllTriggers() noexcept;

  private:
    /// needs to be a list since we return pointer to the underlying EventInfo class with wait
    TriggerList m_triggerList;
    ConditionVariableData* m_conditionVariableDataPtr{nullptr};
    ConditionVariableWaiter m_conditionVariableWaiter;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/wait_set.inl"

#endif // IOX_POSH_POPO_WAIT_SET_HPP
