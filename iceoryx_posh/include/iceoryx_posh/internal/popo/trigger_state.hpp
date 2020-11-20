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

#include <cstdint>
#include <limits>

namespace iox
{
namespace popo
{
class TriggerState
{
  public:
    static constexpr uint64_t INVALID_TRIGGER_ID = std::numeric_limits<uint64_t>::max();
    template <typename T>
    using Callback = void (*)(T* const);

    TriggerState() = default;

    template <typename T>
    TriggerState(T* const origin, const uint64_t triggerId, const Callback<T> callback) noexcept;

    const uint64_t& getTriggerId() const noexcept;

    template <typename T>
    bool doesOriginateFrom(T* const origin) const noexcept;

    template <typename T>
    T* getOrigin() noexcept;

    template <typename T>
    const T* getOrigin() const noexcept;

    void operator()() const noexcept;

  protected:
    void* m_origin = nullptr;
    uint64_t m_originTypeHash = 0U;
    uint64_t m_triggerId = INVALID_TRIGGER_ID;

    Callback<void> m_callbackPtr;
    cxx::function_ref<void(void* const, Callback<void>)> m_callback;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/trigger_state.inl"

#endif
