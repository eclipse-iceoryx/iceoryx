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

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/mepoo/memory_info.hpp"

namespace iox
{
namespace popo
{
struct ConditionVariableSignaler
{
    ConditionVariableSignaler(const mepoo::MemoryInfo& memoryInfo = mepoo::MemoryInfo()) noexcept
        : m_memoryInfo(memoryInfo)
    {
    }

    mepoo::MemoryInfo m_memoryInfo;
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_WAITSET_CONDITION_HPP
