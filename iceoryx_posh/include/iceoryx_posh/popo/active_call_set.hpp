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

#include "iceoryx_posh/internal/popo/building_blocks/event_listener.hpp"
#include "iceoryx_posh/internal/popo/building_blocks/event_variable_data.hpp"
#include "iceoryx_posh/popo/event_attorney.hpp"
#include "iceoryx_posh/popo/trigger_handle.hpp"
#include "iceoryx_utils/cxx/expected.hpp"
#include "iceoryx_utils/cxx/method_callback.hpp"
#include "iceoryx_utils/cxx/types.hpp"
#include "iceoryx_utils/internal/concurrent/loffli.hpp"
#include "iceoryx_utils/internal/concurrent/smart_lock.hpp"

#include <thread>
#include <type_traits>

namespace iox
{
namespace popo
{
enum class ActiveCallSetError
{
    ACTIVE_CALL_SET_FULL,
    EVENT_ALREADY_ATTACHED,
};

class ActiveCallSet
{
  public:
    template <typename T>
    using CallbackRef_t = void (&)(T* const);
    using TranslationCallbackRef_t = void (&)(void* const, void (*const)(void* const));

    template <typename T>
    using CallbackPtr_t = void (*)(T* const);
    using TranslationCallbackPtr_t = void (*)(void* const, void (*const)(void* const));

    ActiveCallSet() noexcept;
    ActiveCallSet(const ActiveCallSet&) = delete;
    ActiveCallSet(ActiveCallSet&&) = delete;
    ~ActiveCallSet();

    ActiveCallSet& operator=(const ActiveCallSet&) = delete;
    ActiveCallSet& operator=(ActiveCallSet&&) = delete;

    template <typename T>
    cxx::expected<ActiveCallSetError> attachEvent(T& eventOrigin, CallbackRef_t<T> eventCallback) noexcept;

    template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
    cxx::expected<ActiveCallSetError>
    attachEvent(T& eventOrigin, const EventType eventType, CallbackRef_t<T> eventCallback) noexcept;

    template <typename T, typename EventType, typename = std::enable_if_t<std::is_enum<EventType>::value>>
    void detachEvent(T& eventOrigin, const EventType eventType) noexcept;

    template <typename T>
    void detachEvent(T& eventOrigin) noexcept;

    static constexpr uint64_t capacity() noexcept;

    uint64_t size() const noexcept;

  protected:
    ActiveCallSet(EventVariableData* eventVariable) noexcept;

  private:
    class Event_t;

    void threadLoop() noexcept;
    cxx::expected<uint64_t, ActiveCallSetError>
    addEvent(void* const origin,
             const uint64_t eventType,
             const uint64_t eventTypeHash,
             CallbackRef_t<void> callback,
             TranslationCallbackRef_t translationCallback,
             const cxx::MethodCallback<void, uint64_t> invalidationCallback) noexcept;
    void removeEvent(void* const origin, const uint64_t eventType, const uint64_t eventTypeHash) noexcept;

    void removeTrigger(const uint64_t index) noexcept;

  private:
    enum class NoEnumUsed
    {
        PLACEHOLDER = 0
    };

    class Event_t
    {
      public:
        ~Event_t();

        bool isEqualTo(const void* const origin, const uint64_t eventType, const uint64_t eventTypeHash) const noexcept;
        bool resetIfEqualTo(const void* const origin, const uint64_t eventType, const uint64_t eventTypeHash) noexcept;
        bool reset() noexcept;
        void init(const uint64_t eventId,
                  void* const origin,
                  const uint64_t eventType,
                  const uint64_t eventTypeHash,
                  CallbackRef_t<void> callback,
                  TranslationCallbackRef_t translationCallback,
                  const cxx::MethodCallback<void, uint64_t> invalidationCallback) noexcept;
        void executeCallback() noexcept;
        bool isInitialized() const noexcept;

      private:
        static constexpr uint64_t INVALID_ID = std::numeric_limits<uint64_t>::max();

        void* m_origin = nullptr;
        uint64_t m_eventType = INVALID_ID;
        uint64_t m_eventTypeHash = INVALID_ID;

        CallbackPtr_t<void> m_callback = nullptr;
        TranslationCallbackPtr_t m_translationCallback = nullptr;

        uint64_t m_eventId = INVALID_ID;
        cxx::MethodCallback<void, uint64_t> m_invalidationCallback;
    };

    // TODO: integrate in LoFFLi?!
    class IndexManager_t
    {
      public:
        IndexManager_t() noexcept;
        bool pop(uint32_t& index) noexcept;
        void push(const uint32_t index) noexcept;
        uint64_t size() const noexcept;

        uint32_t m_loffliStorage[concurrent::LoFFLi::requiredMemorySize(MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET)
                                 / sizeof(uint32_t)];
        concurrent::LoFFLi m_loffli;
        std::atomic<uint64_t> m_size{0U};
    } m_indexManager;


    std::thread m_thread;
    concurrent::smart_lock<Event_t, std::recursive_mutex> m_events[MAX_NUMBER_OF_EVENTS_PER_ACTIVE_CALL_SET];
    std::mutex m_addEventMutex;

    std::atomic_bool m_wasDtorCalled{false};
    EventVariableData* m_eventVariable = nullptr;
    EventListener m_eventListener;
};
} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/active_call_set.inl"

#endif
