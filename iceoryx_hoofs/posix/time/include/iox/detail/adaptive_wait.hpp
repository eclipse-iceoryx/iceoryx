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

#ifndef IOX_HOOFS_TIME_ADAPTIVE_WAIT_HPP
#define IOX_HOOFS_TIME_ADAPTIVE_WAIT_HPP

#include "iox/duration.hpp"
#include "iox/function_ref.hpp"

#include <cstdint>

namespace iox
{
namespace detail
{
/// @brief Building block to implement a busy waiting loop efficiently. It
///        pursues a strategy where in the beginning the behavior is like a
///        busy loop but after some iteration some waiting time is introduced
///        and increased over time.
///        So we have a low latency if the event one is waiting for is happening
///        soon but the CPU load is low when one waits for a long time.
/// @code
///   // must be defined outside of the loop to track the calls
///   adaptive_wait adaptive_wait;
///   while(hasMyEventOccurred) {
///       // will wait until a defined max waiting time is reached
///       adaptive_wait.wait();
///
///       // if the wait should be reset so that one starts with yield again
///       // one can recreate the class and assign it via the copy constructor
///       if ( waitResetConditionOccurred ) adaptive_wait = adaptive_wait();
///   }
/// @endcode
class adaptive_wait
{
  public:
    /// @brief Waits in a smart wait. The first times it calls
    ///        std::thread::yield() after a waiting strategy
    ///        with exponential waiting times is pursued.
    void wait() noexcept;

    /// @brief Waits in a loop in a smart wait until continueToWait returns false.
    /// @param[in] continueToWait callable which returns if the wait should continue
    void wait_loop(const function_ref<bool()> continueToWait) noexcept;

  protected:
    /// @note All numbers are not accurate and are just rough estimates
    ///       acquired by the experiments described below.

    /// @brief The value was choosen by educated guess.
    /// std::thread::sleep_for causes a lot of overhead. 100us was
    /// choosen with the experiment below. The overhead of sleep_for is roughly around
    /// 50% of the actual waitingTime (100us). When the waitingTime is lower the
    /// overhead of sleep_for makes up the majority of the time yield is waiting.
    /// @code
    ///   auto start = std::chrono::steady_clock::now();
    ///   for (uint64_t i = 0; i < repetition; ++i) {
    ///       std::this_thread::sleep_for(sleepingTime);
    ///   }
    ///   auto end = std::chrono::steady_clock::now();
    ///
    ///   auto minimalDuration = repetition * sleepingTime;
    ///   auto actualDuration = end - start;
    ///   // actualDuration ~= 1.5 minimalDuration
    /// @endcode
    static constexpr std::chrono::microseconds INITIAL_WAITING_TIME{100};

    /// @brief The value was choosen by educated guess.
    ///        With 10ms a busy loop is around 0.1% in top. when decreasing it
    ///        to 5ms we get around 0.7% and then it starts to raise fast.
    static constexpr std::chrono::milliseconds FINAL_WAITING_TIME{10};

    /// @brief std::thread::yield cause not much overhead. 10000U was chosen
    /// since the code below requires around 1ms to run on a standard pc.
    /// @code
    ///   auto start = std::chrono::steady_clock::now();
    ///   for (uint64_t i = 0; i < repetition; ++i) {
    ///       std::this_thread::yield();
    ///   }
    ///   auto end = std::chrono::steady_clock::now();
    ///   IOX_LOG(INFO, (end - start).count()); // prints around 1ms
    /// @endcode
    static constexpr uint64_t YIELD_REPETITIONS = 10000U;

    /// @brief The initial repetition is choosen in a way that
    ///        INITIAL_WAITING_TIME * 100U equals roughly FINAL_WAITING_TIME
    static constexpr uint64_t INITIAL_REPETITIONS = 100U + YIELD_REPETITIONS;

  private:
    uint64_t m_yieldCount = 0U;
};
} // namespace detail
} // namespace iox

#endif // IOX_HOOFS_TIME_ADAPTIVE_WAIT_HPP
