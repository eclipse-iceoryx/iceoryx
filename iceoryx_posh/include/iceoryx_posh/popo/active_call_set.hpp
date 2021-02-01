// Copyright (c) 2021 by Apex.AI Inc. All rights reserved.
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

#ifndef IOX_POSH_POPO_ACTIVE_CALL_SET_HPP
#define IOX_POSH_POPO_ACTIVE_CALL_SET_HPP

#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/types.hpp"
#include "iceoryx_utils/internal/concurrent/loffli.hpp"

#include <thread>
#include <type_traits>

namespace iox
{
namespace popo
{
enum class ActiveCallSetError
{

};

class ActiveCallSet
{
  public:
    template <typename T>
    using Callback_t = void (*)(T* const);

    ActiveCallSet() noexcept;
    ~ActiveCallSet();


    template <typename T>
    cxx::expected<ActiveCallSetError> attachEvent(T& eventOrigin, const Callback_t<T>& eventCallback) noexcept
    {
        return cxx::success<>();
    }

    template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
    cxx::expected<ActiveCallSetError>
    attachEvent(T& eventOrigin, const EventType eventType, const Callback_t<T>& eventCallback) noexcept
    {
        return cxx::success<>();
    }

    template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
    cxx::expected<ActiveCallSetError> detachEvent(T& eventOrigin, const EventType eventType) noexcept
    {
        return cxx::success<>();
    }

    template <typename T>
    cxx::expected<ActiveCallSetError> detachEvent(T& eventOrigin) noexcept
    {
        return cxx::success<>();
    }

  private:
    struct Event_t;

    void threadLoop() noexcept;
    void addEvent(void* const origin, const uint64_t eventType, const Callback_t<void> callback) noexcept;
    void removeEvent(void* const origin, const uint64_t eventType) noexcept;

  private:
    enum class CallbackState
    {
        INACTIVE,
        ACTIVE,
        TO_BE_DELETED,
        EMPTY
    };

    struct Event_t
    {
        bool isEqualTo(const void* const origin, const uint64_t eventType) const noexcept;
        void toBeDeleted() noexcept;
        void reset() noexcept;
        void init(concurrent::LoFFLi& indexManager,
                  const uint32_t index,
                  void* const origin,
                  const uint64_t eventType,
                  const Callback_t<void> callback) noexcept;
        void set(void* const origin, const uint64_t eventType, const Callback_t<void> callback) noexcept;
        void operator()() noexcept;

        void* m_origin = nullptr;
        uint64_t m_eventType = 0U;
        std::atomic<uint64_t> m_setCounter{0U};

        Callback_t<void> m_callback = nullptr;
        std::atomic<CallbackState> m_callbackState{CallbackState::EMPTY};

        uint32_t m_index = 0U;
        concurrent::LoFFLi* m_indexManager = nullptr;
    };

    std::thread m_thread;
    Event_t m_events[MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET];

    std::atomic_bool m_wasDtorCalled{false};
    EventVariableData* m_eventVariable = nullptr;

    uint32_t m_loffliStorage[concurrent::LoFFLi::requiredMemorySize(MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
                             / sizeof(uint32_t)];
    concurrent::LoFFLi m_indexManager;
};
} // namespace popo
} // namespace iox

#endif
