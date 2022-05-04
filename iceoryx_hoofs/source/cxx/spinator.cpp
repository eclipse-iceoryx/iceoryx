// Copyright (c) 2022 by Apex.AI Inc. All rights reserved.
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

#include "iceoryx_hoofs/internal/cxx/spinator.hpp"

#include <thread>

namespace iox
{
namespace cxx
{
namespace internal
{
spinator::spinator(const spinator_properties& properties) noexcept
    : m_properties{properties}
    , m_currentWaitingTime{properties.initialWaitingTime}
    , m_increasePerStep{
          units::Duration::fromNanoseconds((properties.maxWaitingTime - properties.initialWaitingTime).toNanoseconds()
                                           / std::max(properties.stepCount, static_cast<uint64_t>(1U)))}
{
}

void spinator::yield() noexcept
{
    std::this_thread::sleep_for(std::chrono::nanoseconds(m_currentWaitingTime.toNanoseconds()));

    ++m_yieldCount;

    if (m_yieldCount % m_properties.repetitionsPerStep == 0)
    {
        if (m_currentWaitingTime + m_increasePerStep <= m_properties.maxWaitingTime)
        {
            m_currentWaitingTime = m_currentWaitingTime + m_increasePerStep;
        }
    }
}

} // namespace internal
} // namespace cxx
} // namespace iox

