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

#ifndef IOX_POSH_POPO_TRIGGER_STATE_HPP
#define IOX_POSH_POPO_TRIGGER_STATE_HPP

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <cstdint>
#include <limits>

namespace iox
{
namespace popo
{
/// @brief TriggerState holds the state of a trigger like the pointer to the triggerOrigin,
///        the trigger id and the callback.
class TriggerState
{
  public:
    static constexpr uint64_t INVALID_TRIGGER_ID = std::numeric_limits<uint64_t>::max();
    template <typename T>
    using Callback = void (*)(T* const);

    /// @brief constructs an empty TriggerState
    TriggerState() = default;
    virtual ~TriggerState() = default;

    /// @brief constructs a TriggerState object
    /// @param[in] triggerOrigin the triggerOrigin of the TriggerState
    /// @param[in] triggerId id of the trigger
    /// @param[in] callback the callback of the trigger
    template <typename T>
    TriggerState(T* const triggerOrigin, const uint64_t triggerId, const Callback<T> callback) noexcept;

    /// @brief returns the trigger id
    /// @return the empty TriggerState always returns INVALID_TRIGGER_ID, otherwise the actual triggerId is returned
    /// which can also be INVALID_TRIGGER_ID
    uint64_t getTriggerId() const noexcept;

    /// @brief confirms the triggerOrigin
    /// @param[in] triggerOrigin the possible triggerOrigin
    /// @return true if the address is equal to the triggerOrigin, otherwise false. The empty TriggerState returns
    /// always false.
    template <typename T>
    bool doesOriginateFrom(T* const triggerOrigin) const noexcept;

    /// @brief returns the pointer to the triggerOrigin.
    /// @return If T equals the Triggerable type it returns the triggerOrigin.
    /// Otherwise it calls the errorHandler with a moderate error of
    /// kPOPO__TRIGGER_STATE_TYPE_INCONSISTENCY_IN_GET_ORIGIN and returns nullptr.
    template <typename T>
    T* getOrigin() noexcept;

    /// @brief returns the pointer to the triggerOrigin.
    /// @return If T equals the Triggerable type it returns the triggerOrigin.
    /// Otherwise it calls the errorHandler with a moderate error of
    /// kPOPO__TRIGGER_STATE_TYPE_INCONSISTENCY_IN_GET_ORIGIN and returns nullptr.
    template <typename T>
    const T* getOrigin() const noexcept;

    /// @brief If a callback is set it executes the callback.
    /// @return true if the callback was called, otherwise false
    bool operator()() const noexcept;

    friend class Trigger;

  protected:
    void* m_triggerOrigin = nullptr;
    uint64_t m_triggerOriginTypeHash = 0U;
    uint64_t m_triggerId = INVALID_TRIGGER_ID;

    Callback<void> m_callbackPtr = nullptr;
    cxx::function_ref<void(void* const, Callback<void>)> m_callback;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/trigger_state.inl"

#endif
