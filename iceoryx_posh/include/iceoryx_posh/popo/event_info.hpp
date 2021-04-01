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

#ifndef IOX_POSH_POPO_EVENT_INFO_HPP
#define IOX_POSH_POPO_EVENT_INFO_HPP

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"
#include "iceoryx_utils/error_handling/error_handling.hpp"

#include <cstdint>
#include <limits>

namespace iox
{
namespace popo
{
/// @brief EventInfo holds the state of a trigger like the pointer to the triggerOrigin,
///        the event id and the callback.
class EventInfo
{
  public:
    static constexpr uint64_t INVALID_ID = std::numeric_limits<uint64_t>::max();
    template <typename T>
    using Callback = void (*)(T* const);

    /// @brief constructs an empty EventInfo
    EventInfo() = default;
    virtual ~EventInfo() = default;

    /// @brief constructs a EventInfo object
    /// @param[in] eventOrigin the origin of the event
    /// @param[in] eventId id of the event
    /// @param[in] callback the callback of the event
    template <typename T>
    EventInfo(T* const eventOrigin, const uint64_t eventId, const Callback<T> callback) noexcept;

    /// @brief returns the event id
    /// @return the empty EventInfo always returns INVALID_ID, otherwise the actual eventId is returned
    /// which can also be INVALID_ID
    uint64_t getEventId() const noexcept;

    /// @brief confirms the eventOrigin
    /// @param[in] eventOrigin the possible eventOrigin
    /// @return true if the address is equal to the eventOrigin, otherwise false. The empty EventInfo returns
    /// always false.
    template <typename T>
    bool doesOriginateFrom(T* const eventOrigin) const noexcept;

    /// @brief returns the pointer to the eventOrigin.
    /// @return If T equals the Triggerable type it returns the eventOrigin.
    /// Otherwise it calls the errorHandler with a moderate error of
    /// kPOPO__EVENT_INFO_TYPE_INCONSISTENCY_IN_GET_ORIGIN and returns nullptr.
    template <typename T>
    T* getOrigin() const noexcept;

    /// @brief If a callback is set it executes the callback.
    /// @return true if the callback was called, otherwise false
    bool operator()() const noexcept;

    friend class Trigger;

  protected:
    void* m_eventOrigin = nullptr;
    uint64_t m_eventOriginTypeHash = 0U;
    uint64_t m_eventId = INVALID_ID;

    Callback<void> m_callbackPtr = nullptr;
    cxx::function_ref<void(void* const, Callback<void>)> m_callback;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/event_info.inl"

#endif
