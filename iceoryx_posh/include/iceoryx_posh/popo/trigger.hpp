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
#include "iceoryx_posh/popo/trigger_state.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"

#include <typeinfo>

namespace iox
{
namespace popo
{
/// @brief The Trigger class is usually managed by a factory class like a
///      WaitSet and acquired by classes which would like to signal an
///      event. Multiple Trigger can share a common ConditionVariableData pointer
///      so that multiple Trigger can signal a single instance.
///
class Trigger : public TriggerState
{
  public:
    using TriggerState::INVALID_TRIGGER_ID;
    template <typename T>
    using Callback = TriggerState::Callback<T>;

    /// @brief Creates an empty Trigger
    Trigger() noexcept = default;
    template <typename T>

    /// @brief Creates a Trigger
    /// @param[in] origin pointer to the class where the signal originates from, if its set to nullptr the Trigger is in
    /// a defined but invalid state
    /// @param[in] conditionVariableDataPtr pointer to the condition variable data, if its set to nullptr the Trigger is
    /// in a defined but invalid state
    /// @param[in] hasTriggeredCallback callback to a method which informs the trigger if it was triggered or not. If an
    /// empty callback is set the trigger is in a defined but invalid state.
    /// @param[in] resetCallback callback which is called when the trigger goes out of scope.
    /// @param[in] triggerId id of the trigger
    /// @param[in] callback function pointer of type void(*)(T * const) to a callback which can be called by the
    /// trigger.
    Trigger(T* const origin,
            ConditionVariableData* conditionVariableDataPtr,
            const cxx::ConstMethodCallback<bool>& hasTriggeredCallback,
            const cxx::MethodCallback<void, const Trigger&>& resetCallback,
            const uint64_t triggerId,
            const Callback<T> callback) noexcept;

    Trigger(const Trigger&) = delete;
    Trigger& operator=(const Trigger&) = delete;
    Trigger(Trigger&& rhs) noexcept;
    Trigger& operator=(Trigger&& rhs) noexcept;

    /// @brief calls reset on destruction
    ~Trigger();

    /// @brief returns true if the Trigger is valid otherwise false
    ///        A trigger is valid when:
    ///         - origin != nullptr
    ///         - conditionVariableDataPtr != nullptr
    ///         - hasTriggeredCallback is set
    explicit operator bool() const noexcept;

    /// @brief returns true if the trigger is valid otherwise false
    bool isValid() const noexcept;

    /// @brief if the Trigger is valid it triggers the trigger otherwise it does nothing
    void trigger() noexcept;

    /// @brief returns the result of the provided hasTriggeredCallback
    bool hasTriggered() const noexcept;

    /// @brief resets and by that invalidates the Trigger
    void reset() noexcept;

    /// @brief returns the pointer to the underlying condition variable data
    ConditionVariableData* getConditionVariableData() noexcept;

    /// @brief returns true if the Triggers are equal otherwise false. Two Triggers are equal when
    ///       - origin == rhs.origin
    ///       - conditionVariableDataPtr == rhs.conditionVariableDataPtr
    ///       - hasTriggeredCallback == rhs.hasTriggeredCallback
    bool operator==(const Trigger& rhs) const noexcept;

    /// @brief return true if the Triggers are not equal, otherwise false.
    bool operator!=(const Trigger& rhs) const noexcept;

    /// @brief sets a new origin for the trigger
    /// @param[in] newOrigin origin of the trigger
    template <typename T>
    void setNewOrigin(T* const newOrigin) noexcept;

  private:
    ConditionVariableData* m_conditionVariableDataPtr = nullptr;

    cxx::ConstMethodCallback<bool> m_hasTriggeredCallback;
    cxx::MethodCallback<void, const Trigger&> m_resetCallback;
    cxx::MethodCallback<void, const Trigger&, void*> m_moveCallback;
    bool m_resetCallbackWasCalled{false};
};


} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/trigger.inl"

#endif
