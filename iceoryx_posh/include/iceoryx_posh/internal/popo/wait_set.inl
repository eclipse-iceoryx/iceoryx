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
#ifndef IOX_POSH_POPO_WAIT_SET_INL
#define IOX_POSH_POPO_WAIT_SET_INL

namespace iox
{
namespace popo
{
template <WaitSet::WaitPolicy policy>
inline WaitSet::ConditionVector
WaitSet::waitAndReturnFulfilledConditions(cxx::optional<units::Duration> timeout) noexcept
{
    ConditionVector conditionsWithFulfilledPredicate;

    auto checkIfOneOfConditionsIsFulfilled = [&]() {
        for (auto& currentCondition : m_conditionVector)
        {
            if (currentCondition->hasTriggered())
            {
                if (!conditionsWithFulfilledPredicate.push_back(currentCondition))
                {
                    errorHandler(Error::kPOPO__WAITSET_CONDITION_VECTOR_OVERFLOW, nullptr, ErrorLevel::FATAL);
                }
            }
        }
    };

    /// @note Inbetween here and last wait someone could have set the trigger to true, hence reset it
    m_conditionVariableWaiter.reset();

    // Is one of the conditons true?
    checkIfOneOfConditionsIsFulfilled();

    if (conditionsWithFulfilledPredicate.empty())
    {
        if (policy == WaitPolicy::TIMED_WAIT)
        {
            auto hasTimeOut = !m_conditionVariableWaiter.timedWait(timeout.value());

            if (hasTimeOut == true)
            {
                // Return empty list
                return conditionsWithFulfilledPredicate;
            }
        }
        else
        {
            assert(!timeout.has_value() && "Timeout provided but WaitPolicy::BLOCKING_WAIT chosen");
            m_conditionVariableWaiter.wait();
        }

        // Check again if one of the conditions is true after we received the signal
        checkIfOneOfConditionsIsFulfilled();
    }
    // Return of a copy of all conditions that were fulfilled
    return conditionsWithFulfilledPredicate;
}


} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_WAIT_SET_INL
