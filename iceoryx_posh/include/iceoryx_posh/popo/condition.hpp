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
#ifndef IOX_POSH_POPO_CONDITION_HPP
#define IOX_POSH_POPO_CONDITION_HPP

#include "iceoryx_posh/internal/log/posh_logging.hpp"
#include "iceoryx_utils/cxx/function_ref.hpp"

namespace iox
{
namespace popo
{
struct ConditionVariableData;
class WaitSet;
/// @brief Base class representing a generic condition that can be stored in a WaitSet
class Condition
{
  public:
    Condition() noexcept = default;

    /// @brief Removes the condition from the WaitSet safely but does not detach it.
    ///        This means unsetConditionVariable is not called since this falls
    ///        in the responsibility of the class dtor of the derived class.
    virtual ~Condition() noexcept;

    /// @brief copy and move operations are deleted since the WaitSet stores
    ///        pointers to conditions. If we would allow copy and move it would
    ///        be possible that the waitset contains dangling pointers after a
    ///        condition was copied or moved.
    Condition(const Condition& rhs) noexcept = delete;
    Condition(Condition&& rhs) = delete;
    Condition& operator=(const Condition& rhs) = delete;
    Condition& operator=(Condition&& rhs) = delete;

    /// @brief Was the condition fulfilled since last call?
    virtual bool hasTriggered() const noexcept = 0;
    /// @brief Called by a WaitSet before attaching a Condition to see whether it was already added
    bool isConditionVariableAttached() const noexcept;

  protected:
    virtual void setConditionVariable(ConditionVariableData* const conditionVariableDataPtr) noexcept = 0;
    virtual void unsetConditionVariable() noexcept = 0;

    friend class WaitSet;

  private:
    template <typename T>
    void attachConditionVariable(T* const origin, ConditionVariableData* const conditionVariableDataPtr) noexcept;
    void detachConditionVariable() noexcept;

  private:
    void* m_origin{nullptr};
    cxx::function_ref<void(void*, void*)> m_cleanupCall;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/condition.inl"

#endif // IOX_POSH_POPO_CONDITION_HPP
