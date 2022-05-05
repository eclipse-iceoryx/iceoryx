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
constexpr spinator::wait_strategy spinator::LOW_LATENCY_HIGH_CPU_LOAD;
constexpr spinator::wait_strategy spinator::MEDIUM_LATENCY_MEDIUM_CPU_LOAD;
constexpr spinator::wait_strategy spinator::HIGH_LATENCY_LOW_CPU_LOAD;
constexpr spinator::wait_strategy spinator::WAIT_STRATEGY;

void spinator::yield() noexcept
{
    if (m_performYield)
    {
        std::this_thread::yield();
    }
    else
    {
        std::this_thread::sleep_for(std::chrono::nanoseconds(m_current.waitingTime.toNanoseconds()));
    }

    if (!m_timeoutSaturated)
    {
        ++m_current.yieldCount;

        if (m_current.repetitionsPerStep <= m_current.yieldCount)
        {
            if (m_performYield)
            {
                m_performYield = false;
            }
            else
            {
                m_current.waitingTime = m_current.waitingTime * WAIT_STRATEGY.factor;
                m_current.repetitionsPerStep = m_current.repetitionsPerStep / WAIT_STRATEGY.factor;
                m_timeoutSaturated = (m_current.repetitionsPerStep <= 1U);
            }
        }
    }
}
} // namespace internal
} // namespace cxx
} // namespace iox
