// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
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
#include "iceoryx_posh/internal/popo/building_blocks/condition_variable_waiter.hpp"
#include "iceoryx_posh/popo/condition.hpp"
#include "iceoryx_posh/runtime/posh_runtime.hpp"
#include "iceoryx_utils/cxx/vector.hpp"

namespace iox
{
namespace popo
{
enum class WaitSetError : uint8_t
{
    CONDITION_VECTOR_OVERFLOW,
    CONDITION_VARIABLE_ALREADY_SET,
    CONDITION_VARIABLE_ATTACH_FAILED
};

/// @brief Logical disjunction of a certain number of Conditions
///
/// The WaitSet stores Conditions and allows the user to wait till those set of Conditions become true. It works over
/// process borders. With the creation of a WaitSet it requests a condition variable from RouDi and destroys it with the
/// destructor. Hence the lifetime of the condition variable is bound to the lifetime of the WaitSet. Before destroying
/// a Condition object the user has to ensure that it was detached from the WaitSet before.
///
///
/// @code
///
/// @todo Add gateway use-case to example code
///
/// // Create Waitset
/// WaitSet myWaitSet();
///
/// Subscriber mySubscriber1({"Radar", "FrontRight", "Counter"});
/// Subscriber mySubscriber2({"Radar", "FrontLeft", "Counter"});
/// GuardCondition myGuardCond;
///
///
/// myWaitSet.attachCondition(mySubscriber1);
/// myWaitSet.attachCondition(mySubscriber2);
/// myWaitSet.attachCondition(myGuardCond);
///
/// std::thread someOtherThread{[&](){
/// 	while(true)
/// 	{
/// 	  if(IWantToWakeUpTheWaitSet)
/// 	  {
/// 		  // Let the WaitSet return
/// 		  myGuardCond.setTrigger();
/// 	  }
/// 	  // Don't forget to reset the trigger
/// 	  myGuardCond.resetTrigger();
/// 	}
/// }};
///
/// while (true)
/// {
/// 	// Wait till new data has arrived (any condition has become true)
/// 	auto vectorOfFulfilledConditions = myWaitSet.wait();
///
/// 	for(auto& element : vectorOfFulfilledConditions)
/// 	{
/// 		if(element == &mySubscriber1)
/// 		{
/// 			// Subscriber1 has received new data
/// 			ChunkHeader myData;
/// 			mySubscriber1.tryGetChunk(myData);
/// 			doSomeThingWithTheNewData(myData);
/// 		}
/// 	}
/// }
/// someOtherThread.join();
///
/// myWaitSet.detachCondition(guardCond);
/// myWaitSet.detachCondition(mySubscriber2);
/// myWaitSet.detachCondition(mySubscriber1);
///
/// @endcode
class WaitSet
{
  public:
    using ConditionVector = cxx::vector<Condition*, MAX_NUMBER_OF_CONDITIONS_PER_WAITSET>;

    enum class WaitPolicy : uint16_t
    {
        BLOCKING_WAIT,
        TIMED_WAIT
    };

    explicit WaitSet(cxx::not_null<ConditionVariableData* const> =
                         runtime::PoshRuntime::getInstance().getMiddlewareConditionVariable()) noexcept;
    virtual ~WaitSet() noexcept;
    WaitSet(const WaitSet& rhs) = delete;
    WaitSet(WaitSet&& rhs) = delete;
    WaitSet& operator=(const WaitSet& rhs) = delete;
    WaitSet& operator=(WaitSet&& rhs) = delete;

    /// @brief Adds a condition to the internal vector
    /// @param[in] condition, condition to be attached
    /// @return Returns an expected, that can contain on error from WaitSetError
    cxx::expected<WaitSetError> attachCondition(Condition& condition) noexcept;

    /// @brief Removes a condition from the internal vector
    /// @param[in] condition, condition to be detached
    /// @return True if successful, false if unsuccessful
    bool detachCondition(Condition& condition) noexcept;

    /// @brief Clears all conditions from the waitset
    void detachAllConditions() noexcept;

    /// @brief Blocking wait with time limit till one or more of the condition become true
    /// @param[in] timeout How long shall be waited for a signalling condition
    /// @return ConditionVector vector of condition pointers that have become
    /// fulfilled
    ConditionVector timedWait(const units::Duration timeout) noexcept;

    /// @brief Blocking wait till one or more of the condition become true
    /// @return ConditionVector vector of condition pointers that have become
    /// fulfilled
    ConditionVector wait() noexcept;

  private:
    template <WaitPolicy policy>
    ConditionVector waitAndReturnFulfilledConditions(cxx::optional<units::Duration> timeout = cxx::nullopt) noexcept;
    ConditionVector m_conditionVector;
    ConditionVariableData* m_conditionVariableDataPtr{nullptr};
    ConditionVariableWaiter m_conditionVariableWaiter;
};

} // namespace popo
} // namespace iox

#include "iceoryx_posh/internal/popo/wait_set.inl"

#endif // IOX_POSH_POPO_WAIT_SET_HPP
