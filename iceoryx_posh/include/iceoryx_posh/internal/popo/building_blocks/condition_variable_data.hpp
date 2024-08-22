// Copyright (c) 2020 by Robert Bosch GmbH. All rights reserved.
// Copyright (c) 2021 - 2022 by Apex.AI Inc. All rights reserved.
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
#ifndef IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_DATA_HPP
#define IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_DATA_HPP

#include "iceoryx_posh/iceoryx_posh_types.hpp"
#include "iceoryx_posh/internal/posh_error_reporting.hpp"
#include "iox/atomic.hpp"
#include "iox/unnamed_semaphore.hpp"

namespace iox
{
namespace popo
{
struct ConditionVariableData
{
    ConditionVariableData() noexcept;
    explicit ConditionVariableData(const RuntimeName_t& runtimeName) noexcept;

    ConditionVariableData(const ConditionVariableData& rhs) = delete;
    ConditionVariableData(ConditionVariableData&& rhs) = delete;
    ConditionVariableData& operator=(const ConditionVariableData& rhs) = delete;
    ConditionVariableData& operator=(ConditionVariableData&& rhs) = delete;
    ~ConditionVariableData() noexcept = default;

    optional<UnnamedSemaphore> m_semaphore;
    RuntimeName_t m_runtimeName;
    concurrent::Atomic<bool> m_toBeDestroyed{false};
    concurrent::Atomic<bool> m_activeNotifications[MAX_NUMBER_OF_NOTIFIERS];
    concurrent::Atomic<bool> m_wasNotified{false};
};

} // namespace popo
} // namespace iox

#endif // IOX_POSH_POPO_BUILDING_BLOCKS_CONDITION_VARIABLE_DATA_HPP
