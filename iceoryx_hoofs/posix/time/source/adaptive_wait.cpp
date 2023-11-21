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

#include "iox/detail/adaptive_wait.hpp"

#include <thread>

namespace iox
{
namespace detail
{
constexpr std::chrono::microseconds adaptive_wait::INITIAL_WAITING_TIME;
constexpr std::chrono::milliseconds adaptive_wait::FINAL_WAITING_TIME;
constexpr uint64_t adaptive_wait::YIELD_REPETITIONS;
constexpr uint64_t adaptive_wait::INITIAL_REPETITIONS;

void adaptive_wait::wait() noexcept
{
    ++m_yieldCount;

    if (m_yieldCount <= YIELD_REPETITIONS)
    {
        std::this_thread::yield();
    }
    else if (m_yieldCount <= INITIAL_REPETITIONS)
    {
        std::this_thread::sleep_for(INITIAL_WAITING_TIME);
    }
    else
    {
        std::this_thread::sleep_for(FINAL_WAITING_TIME);
    }
}

void adaptive_wait::wait_loop(const function_ref<bool()> continueToWait) noexcept
{
    while (continueToWait())
    {
        wait();
    }
}
} // namespace detail
} // namespace iox
