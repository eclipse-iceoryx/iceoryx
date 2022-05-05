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

#ifndef IOX_HOOFS_CXX_SPINATOR_HPP
#define IOX_HOOFS_CXX_SPINATOR_HPP

#include "iceoryx_hoofs/internal/units/duration.hpp"

#include <cstdint>

namespace iox
{
namespace cxx
{
namespace internal
{
class spinator
{
  public:
    void yield() noexcept;

  private:
    struct wait_strategy
    {
        uint64_t factor = 1U;
        uint64_t repetitionsPerStep = 1U;
    };

    // max waiting time: ~8ms, reached after: ~106ms, steps: 13
    static constexpr wait_strategy LOW_LATENCY_HIGH_CPU_LOAD = {2U, 1U << 13U};
    // max waiting time: ~4ms, reached after: ~25ms, steps: 6
    static constexpr wait_strategy MEDIUM_LATENCY_LOW_CPU_LOAD = {4U, 1U << 12U};
    // max waiting time: ~4ms, reached after: ~16ms, steps: 4
    static constexpr wait_strategy HIGH_LATENCY_LOW_CPU_LOAD = {8U, 1U << 12U};

    // static constexpr wait_strategy WAIT_STRATEGY = HIGH_LATENCY_LOW_CPU_LOAD;
    static constexpr wait_strategy WAIT_STRATEGY = LOW_LATENCY_HIGH_CPU_LOAD;

    struct
    {
        units::Duration waitingTime = units::Duration::fromMicroseconds(1U);
        uint64_t repetitionsPerStep = WAIT_STRATEGY.repetitionsPerStep;
        uint64_t yieldCount = 0U;
    } m_current;

    bool m_performYield = true;
    bool m_timeoutSaturated = false;
};
} // namespace internal
} // namespace cxx
} // namespace iox

#endif
