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
/// @brief Building block to implement a busy waiting loop efficiently. It
///        pursues a strategy where in the beginning the behavior is like a
///        busy loop but after some iteration some waiting time is introduced
///        and exponentially increased.
///        So we have a low latency if the event one is waiting for is happening
///        soon but the CPU load is low when one waits for a long time.
/// @code
///   spinator spinator; // must be defined outside of the loop to track the calls
///   while(hasMyEventOccurred) {
///       spinator.yield();
///   }
/// @endcode
class spinator
{
  public:
    /// @brief Waits in a smart wait. The first times yield is called
    ///        std::thread::yield() is called and after then a waiting strategy
    ///        with exponential waiting times is pursued.
    void yield() noexcept;

  private:
    /// @brief defines the waiting strategy of the spinator
    /// 1. First, spinator::yield() will call std::thread::yield for
    ///    repetitionsPerStep times
    /// 2. Then it will wait for 1 microsecond for repetitionsPerStep times
    /// 3. After that the waiting time will be multiplied by factor and the
    ///    repetitionsPerStep divided by factor until repetitionsPerStep are
    ///    less or equal to one.
    /// 4. When repetitionsPerStep is less or equal one the waiting time will be
    ///    no longer increased and yield will wait a constant time.
    ///
    /// Example:
    ///   Assume factor == 2 and repetitionsPerStep == 1024 = 2 ^ 10.
    ///   1. In the first step we have a busy wait repeated 1024 times
    ///   2. Then we wait 1024 times for 1 microsecond, so we wait in total
    ///      ~1 milliseconds = 1 microsecond * 1024
    ///   3. Next the waiting times is multiplied by 2 and the
    ///      repetitionsPerStep are devided by 2.
    ///      We wait again for ~1 milliseconds = 2 microsecond * 512
    ///   4. This is repeated 10 times (2^10 = 1024) until
    ///      repetitionsPerStep == 1 and the waitingTime == 1 milliseconds.
    ///      So we have 10 steps, the max waiting time is 1ms and the time
    ///      required until the final waiting time is reached is
    ///      repetitionsPerStep * step = 1024 * 10 = 10 ms.
    struct wait_strategy
    {
        uint64_t factor = 1U;
        uint64_t repetitionsPerStep = 1U;
    };

    /// @note The difference in CPU load betweed HIGH and LOW_CPU_LOAD seems to
    ///       be around 0.3% on a i7-10875H CPU @ 2.30GHz

    // max waiting time: ~8ms, reached after: ~106ms, steps: 13
    static constexpr wait_strategy LOW_LATENCY_HIGH_CPU_LOAD = {2U, 1U << 13U};
    // max waiting time: ~4ms, reached after: ~25ms, steps: 6
    static constexpr wait_strategy MEDIUM_LATENCY_MEDIUM_CPU_LOAD = {4U, 1U << 12U};
    // max waiting time: ~4ms, reached after: ~16ms, steps: 4
    static constexpr wait_strategy HIGH_LATENCY_LOW_CPU_LOAD = {8U, 1U << 12U};

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
