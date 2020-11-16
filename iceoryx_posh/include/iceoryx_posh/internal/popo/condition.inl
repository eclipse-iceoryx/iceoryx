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

#ifndef IOX_POSH_POPO_CONDITION_INL
#define IOX_POSH_POPO_CONDITION_INL

namespace iox
{
namespace popo
{
template <typename T>
void internalConditionCleanupCall(void* const origin, void* const entry) noexcept
{
    reinterpret_cast<T*>(origin)->remove(entry);
}

template <typename T>
void Condition::attachConditionVariable(T* const origin, ConditionVariableData* const conditionVariableDataPtr) noexcept
{
    if (isConditionVariableAttached())
    {
        LogWarn()
            << "Attaching an already attached condition leads to a detach from the current WaitSet. Best practice "
               "is to detach Condition first before attaching it.";
        detachConditionVariable();
    }

    m_origin = origin;
    m_cleanupCall = internalConditionCleanupCall<T>;
    setConditionVariable(conditionVariableDataPtr);
}
} // namespace popo
} // namespace iox

#endif
