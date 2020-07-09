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
#ifndef IOX_POSH_POPO_WAITSET_CONDITION_HPP
#define IOX_POSH_POPO_WAITSET_CONDITION_HPP

#include "iceoryx_posh/internal/popo/waitset/condition_variable_data.hpp"

namespace iox
{
namespace popo
{
/// @brief Base class representing a generic condition that can be stored in a WaitSet
/// @note No functionality in this class
class Condition
{
  public:
    virtual ~Condition() = default;

    virtual bool hasTrigger() const noexcept = 0;
    virtual void resetTrigger() noexcept = 0;
    virtual bool isConditionVariableAttached() noexcept = 0;
    virtual bool attachConditionVariable(ConditionVariableData* ConditionVariableDataPtr) noexcept = 0;
    virtual bool detachConditionVariable() noexcept = 0;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_WAITSET_CONDITION_HPP
